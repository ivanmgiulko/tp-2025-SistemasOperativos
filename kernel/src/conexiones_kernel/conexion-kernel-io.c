#include "conexion-kernel-io.h"
int manejar_conexion_kernel_io(){
    int socket_io = esperar_cliente(fd_server_io, logger_kernel);
	
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
		int cod_op = recibir_operacion(socket_io);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_io, logger_kernel);
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