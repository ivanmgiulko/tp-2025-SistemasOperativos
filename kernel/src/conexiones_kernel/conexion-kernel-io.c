#include "conexion-kernel-io.h"
int manejar_conexion_kernel_io(){
    socket_io = esperar_cliente(fd_server_io, logger_kernel);
	
	size_t tamanio_interfaz;
	int32_t resultado_handshake_exitoso = 1;
	if(recv(socket_io, &tamanio_interfaz, sizeof(size_t), 0) != sizeof(size_t)){
		log_error(logger_kernel, "Error al recibir el tamanio de la interfaz");
		return EXIT_FAILURE;
	}

	void* stream = malloc(tamanio_interfaz);
	if(recv(socket_io, stream, tamanio_interfaz, 0) != tamanio_interfaz){
		log_error(logger_kernel, "Error al recibir el nombre de la interfaz");
		return EXIT_FAILURE;
	}

	// Enviar respuesta al cliente
	send(socket_io, &resultado_handshake_exitoso, sizeof(int32_t), 0);

	if(lista_de_io == NULL){
		inicializar_lista_io();
	}

	char* nombre_io = malloc(tamanio_interfaz + 1);
	memcpy(nombre_io, stream, tamanio_interfaz);
	nombre_io[tamanio_interfaz] = '\0';  // Asegurarse de que termine en \0

	inicializar_io(nombre_io, socket_io);

	free(stream);

	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_io);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_io, logger_kernel);
			break;
		case PROCESO_DESBLOQUEADO:
			recv(socket_io, &(paquete->buffer->size), sizeof(uint32_t), 0);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(socket_io, paquete->buffer->stream, paquete->buffer->size, 0);

			uint8_t pid_desbloqueado = recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "## %d finalizó IO y pasa a READY", pid_desbloqueado);
			// Sacar el proceso de bloqueado y mandar a Ready

			break;
		case -1:
			log_error(logger_kernel, "el cliente [IO] se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_io);
	return EXIT_SUCCESS;
}

void enviar_proceso_a_io(uint8_t pid, int64_t tiempo, int socket_cliente) { 
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(int64_t);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &tiempo, sizeof(int64_t)); offset += sizeof(int64_t);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_BLOQUEADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

uint8_t recibir_proceso_bloqueado(t_buffer* buffer) { 
	uint8_t* pid = malloc(sizeof(uint8_t));
    void* stream = buffer->stream;

    memcpy(&(pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
    
    return pid;
}