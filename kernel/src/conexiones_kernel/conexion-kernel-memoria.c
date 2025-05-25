#include "conexion-kernel-memoria.h"

void enviar_tamanio_proceso(char* tam_proceso, int socket_cliente)
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

char* recibir_respuestaMemoria(int socket_cliente) { 
    int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}

int manejar_conexion_kernel_memoria(int socket_cliente){
	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_cliente);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_kernel);
			break;

		case PROCESO_MEMORIA:
			char* validacionMemo = recibir_respuestaMemoria(socket_cliente);
			if(strcmp(validacionMemo, "No hay espacio en memoria") == 0) {
				// El proceso sigue en la cola NEW
				return 0;
			} else {
				// El proceso pasa a la cola de Ready
				return 1;
			}
			break;

		case PROCESO_FINALIZADO:
			recibir_paquete(socket_cliente, paquete);
			
			t_pcbMemoria* proceso_finalizado = deserializarProceso(paquete->buffer);

			log_info(logger_kernel, "## %d - Finaliza el proceso", proceso_finalizado->pid);
			sem_post(&sem_hay_espacio_en_memoria);
			sem_post(&bin_proceso_eliminar);
			
			eliminar_paquete(paquete);
			
			free(proceso_finalizado);

			return EXIT_SUCCESS;
			break;
		
		case -1:
			log_error(logger_kernel, "el cliente [MEMORIA] se desconecto .");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(fd_conexion_memoria);
	log_trace(logger_kernel, "Se cierra la conexion con Memoria");
	return EXIT_SUCCESS;
}