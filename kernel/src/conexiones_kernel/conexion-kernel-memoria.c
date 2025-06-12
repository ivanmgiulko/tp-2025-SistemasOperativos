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

		case RESPUESTA_DUMPEO:
			recibir_paquete(socket_cliente, paquete);
			t_respuesta_dump* resp_dump = recibir_respuesta_dump(paquete->buffer);

			t_pcb* proceso_desbloqueado = _sacar_pcb_de_blocked(resp_dump->pid);

			if(resp_dump->respuesta == false){
				
				pasar_pcb_blocked_a_exit(proceso_desbloqueado);
				log_debug(logger_kernel, "NO se pudo hacer el DUMP. Finaliza el proceso...");

			} else {
				
				pasar_pcb_blocked_a_ready(proceso_desbloqueado);
				log_debug(logger_kernel, "SI se pudo hacer el DUMP. El proceso se desbloquea y pasa a Ready");
			}
			return EXIT_SUCCESS;

			break;

		case PROCESO_FINALIZADO:
			recibir_paquete(socket_cliente, paquete);
			
			int* offset = malloc(sizeof(int));
			*offset = 0;

			uint8_t pid = _deserializar_pid(offset, paquete);
			
			log_info(logger_kernel, "%d - Finaliza el proceso", pid);

			pthread_mutex_lock(&estado_exit->mutex);
			t_pcb* proceso_finalizado = buscar_proceso_en_cola_exit(estado_exit->cola, pid);
			temporal_stop(proceso_finalizado->metricas_tiempo->tiempoEnExit);
			pthread_mutex_unlock(&estado_exit->mutex);

			log_info(logger_kernel, "%d - Metricas de estado: NEW [%d] [%ld], READY [%d] [%ld], BLOCKED [%d] [%ld], EXEC [%d] [%ld], EXIT [%d] [%ld], SUSP-READY [%d] [%ld], SUSP-BLOCKED [%d] [%ld]", 
			proceso_finalizado->pid, 
			proceso_finalizado->metricas_estado->cantVecesNew, 		   proceso_finalizado->metricas_tiempo->tiempoEnNew->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesReady,       proceso_finalizado->metricas_tiempo->tiempoEnReady->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesBlocked,     proceso_finalizado->metricas_tiempo->tiempoEnBlocked->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesExec, 	   proceso_finalizado->metricas_tiempo->tiempoEnExec->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesExit,        proceso_finalizado->metricas_tiempo->tiempoEnExit->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesSuspReady,   proceso_finalizado->metricas_tiempo->tiempoEnSuspReady->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesSuspBlocked, proceso_finalizado->metricas_tiempo->tiempoEnSuspBlocked->elapsed_ms);

			free(proceso_finalizado->metricas_estado);
			free(proceso_finalizado->metricas_tiempo);
			free(proceso_finalizado);
			
			sem_post(&sem_hay_espacio_en_memoria);
			sem_post(&bin_proceso_eliminar);
			
			eliminar_paquete(paquete);
			
		
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

t_pcb* buscar_proceso_en_cola_exit(t_list* cola_exit, uint8_t pid) 
{
	bool _tiene_el_pid(void* ptr) {
		t_pcb* proceso = (t_pcb*) ptr;
		return proceso->pid == pid;
	}
	return list_find(cola_exit, _tiene_el_pid);
}

t_respuesta_dump* recibir_respuesta_dump(t_buffer* buffer)
{
	t_respuesta_dump* respuesta_dump = malloc(sizeof(t_respuesta_dump));

	void* stream = buffer->stream;

    memcpy(&(respuesta_dump->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
	memcpy(&(respuesta_dump->respuesta), stream, sizeof(bool)); stream += sizeof(bool);
	
    return respuesta_dump;
}

t_pcb* _sacar_pcb_de_blocked(int pid) 
{ 
    pthread_mutex_lock(&(estado_blocked->mutex));
    t_pcb* _proceso_a_desbloquear = NULL;
    for (int i = 0; i < list_size(estado_blocked->cola); i++) {
        t_pcb* pcb = list_get(estado_blocked->cola, i);
        if (pcb->pid == pid) {
            _proceso_a_desbloquear = list_remove(estado_blocked->cola, i); // Eliminar el elemento
            break; // Salir del bucle una vez encontrado
        }
    }

    pthread_mutex_unlock(&(estado_blocked->mutex));
    return _proceso_a_desbloquear;
}

