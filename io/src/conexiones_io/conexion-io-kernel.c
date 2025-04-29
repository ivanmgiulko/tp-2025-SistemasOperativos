#include "conexion-io-kernel.h"

void enviar_nombreInterfaz(char* mensaje, int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = INTERFAZ;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	// Prueba de HandShake
	size_t bytesHS;
	int32_t handshake = strlen(mensaje) + 1;
	int32_t result;
	bytesHS = send(socket_cliente, &handshake, sizeof(int32_t), 0);
	bytesHS = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
	if (result == 0) {
		log_info(logger_io, "El Handshake esta bien desde el lado de IO!");
    	// Handshake OK
		send(socket_cliente, a_enviar, bytes, 0); 
	} else {
    	// Handshake ERROR
	}

	free(a_enviar);
	eliminar_paquete(paquete);
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