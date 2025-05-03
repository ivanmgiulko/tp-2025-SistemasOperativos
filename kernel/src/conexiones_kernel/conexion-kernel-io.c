#include "conexion-kernel-io.h"
int manejar_conexion_kernel_io(){
    int socket_io = esperar_cliente(fd_server_io, logger_kernel);
	
	// Prueba de HS para recibir nombre de interfaz
	size_t bytes;

	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	bytes = recv(socket_io, &handshake, sizeof(int32_t), MSG_WAITALL);
		if (handshake == 9) { // Habria que mandarle antes 
    		bytes = send(socket_io, &resultOk, sizeof(int32_t), 0);
		} else {
    		bytes = send(socket_io, &resultError, sizeof(int32_t), 0);
		}
		
	while (1) {
		int cod_op = recibir_operacion(socket_io);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_io, logger_kernel);
			break;
		case INSTRUCCION:
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