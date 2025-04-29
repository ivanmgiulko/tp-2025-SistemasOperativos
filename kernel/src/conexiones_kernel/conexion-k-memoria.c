#include "conexion-k-memoria.h"

void enviar_tamanioProceso(char* tam_proceso, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = TAMANIO_PROCESO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(tam_proceso) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, tam_proceso, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

int manejar_conexion_kernel_memoria(){
	while (1) {
		int cod_op = recibir_operacion(fd_conexion_memoria);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_conexion_memoria, logger_kernel);
			break;
		case INSTRUCCION:
			break;
		case -1:
			log_error(logger_kernel, "el cliente [MEMEORIA] se desconecto .");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(fd_conexion_memoria);
	return EXIT_SUCCESS;
}