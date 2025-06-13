#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-memoria/modulo-memoria.h>
#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
// #include <utils_kernel/utils-complementarios/conexion-con-memoria

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