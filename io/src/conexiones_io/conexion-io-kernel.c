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

	recv(socket_cliente, &result, sizeof(int32_t), 0);
	if (result == 1) {
		log_info(logger_io, "Handshake con [KERNEL] exitoso!");
	} else {
		log_error(logger_io, "Handshake con [KERNEL] fallido!");
		free(mensaje_handshake);
		return EXIT_FAILURE;
	}
	free(mensaje_handshake);
}

int manejar_conexion_io(int socket_cliente){
	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_io);
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