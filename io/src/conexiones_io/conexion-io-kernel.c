#include "conexion-io-kernel.h"

void enviar_nombre_interfaz(char* mensaje, int socket_cliente) {

	// HANDSHAKE
	// DEFINIR TAMANIO DEL NOMBRE DE LA INTERFAZ
	size_t tamanio_interfaz = strlen(mensaje) + 1;

	// DEFINIR TAMANIO TOTAL DEL MENSAJE (TAMANIO DE SIZE_T + TAMANIO NOMBRE DE LA INTERFAZ)
	size_t tamanio_total = sizeof(size_t) + tamanio_interfaz;

	// DEFINIR UNA VARIABLE PARA EL RESULTADO DEL HANDSHAKE
	int32_t result = -1;

	// RESERVAR ESPACIO PARA EL MENSAJE DEL HANDSHAKE
	void* mensaje_handshake = malloc(tamanio_total);

	// COPIAR EL MENSAJE DEL HANDSHAKE EN LA VARIABLE
	memcpy(mensaje_handshake, &tamanio_interfaz, sizeof(size_t));
	memcpy(mensaje_handshake + sizeof(size_t), mensaje, tamanio_interfaz);
	
	// ENVIAR EL MENSAJE DEL HANDSHAKE
	send(socket_cliente, mensaje_handshake, tamanio_total, 0);

	// Recibo respuesta del kernel del HandShake
	recv(socket_cliente, &result, sizeof(int32_t), 0);
	if (result == 1) {
		log_info(logger_io, "Handshake con [KERNEL] exitoso!");
	} else {
		log_error(logger_io, "Handshake con [KERNEL] fallido!");
		free(mensaje_handshake);
		abort();
	}
	
	free(mensaje_handshake);
}

t_info_proceso* recibir_proceso_bloqueado(t_buffer* buffer) { 
	t_info_proceso* pruebaProceso = malloc(sizeof(t_info_proceso));
    void* stream = buffer->stream;

    memcpy(&(pruebaProceso->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
    memcpy(&(pruebaProceso->tiempo), stream, sizeof(int64_t)); stream += sizeof(int64_t);
    
    return pruebaProceso;
}
 
void enviar_pid_desbloqueado(int socket_cliente, uint8_t pid, int codigo_operacion) { 

    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t); 
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &pid, sizeof(uint8_t));

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion; 
    paquete->buffer = buffer;

    size_t tamanio_total = sizeof(int) + sizeof(uint8_t) + buffer->size;
    void* mensaje_serializado = malloc(tamanio_total);

    uint32_t offset = 0;

    memcpy(mensaje_serializado + offset, &(paquete->codigo_operacion), sizeof(int));
    offset += sizeof(int);

    memcpy(mensaje_serializado + offset, &(paquete->buffer->size), sizeof(uint8_t)); 
    offset += sizeof(uint8_t);

    memcpy(mensaje_serializado + offset, paquete->buffer->stream, buffer->size); 
    offset += buffer->size;


    send(socket_cliente, mensaje_serializado, tamanio_total, 0);

    free(mensaje_serializado);
    eliminar_paquete(paquete);
}

int manejar_conexion_io(int socket_cliente){
	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(socket_cliente);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_io);
			break;

		case PROCESO_BLOQUEADO:
			recibir_paquete(socket_cliente, paquete);

			t_info_proceso* proceso_bloqueado = recibir_proceso_bloqueado(paquete->buffer);
			log_debug(logger_io, "Llego el PID: %d | El tiempo: %ld", proceso_bloqueado->pid, proceso_bloqueado->tiempo);

			log_info(logger_io, "## PID: %d - Inicio de IO - Tiempo: %ld", proceso_bloqueado->pid, proceso_bloqueado->tiempo);
			usleep(proceso_bloqueado->tiempo * 1000);
			log_info(logger_io, "## PID: %d - Fin de IO", proceso_bloqueado->pid);
			
			enviar_pid_desbloqueado(socket_cliente, proceso_bloqueado->pid, PROCESO_DESBLOQUEADO);
			break;

		case -1:
			log_error(logger_io, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_io, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	close(socket_cliente);
	return EXIT_SUCCESS;
}