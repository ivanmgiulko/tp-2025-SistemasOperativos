#include "manejar-conexion-memoria.h"
#include "utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h"

void loguear_y_finalizar_proceso(t_pcb* proceso_finalizado) {
    log_info(logger_kernel, "%d - Finaliza el proceso", proceso_finalizado->pid);
    log_info(logger_kernel, "%d - Metricas de estado: NEW [%d] [%ld], READY [%d] [%ld], BLOCKED [%d] [%ld], EXEC [%d] [%ld], EXIT [%d] [%ld], SUSP-READY [%d] [%ld], SUSP-BLOCKED [%d] [%ld]", 
	proceso_finalizado->pid, 
	proceso_finalizado->metricas_estado->cantVecesNew, 		   temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnNew),
	proceso_finalizado->metricas_estado->cantVecesReady,       temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnReady),
	proceso_finalizado->metricas_estado->cantVecesBlocked,     temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnBlocked),
	proceso_finalizado->metricas_estado->cantVecesExec, 	   temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnExec),
	proceso_finalizado->metricas_estado->cantVecesExit,        temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnExit),
	proceso_finalizado->metricas_estado->cantVecesSuspReady,   temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnSuspReady),
	proceso_finalizado->metricas_estado->cantVecesSuspBlocked, temporal_gettime(proceso_finalizado->metricas_tiempo->tiempoEnSuspBlocked));

    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnBlocked);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnExec);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnExit);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnNew);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnReady);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnSuspBlocked);
    temporal_destroy(proceso_finalizado->metricas_tiempo->tiempoEnSuspReady);

	free(proceso_finalizado->metricas_estado);
	free(proceso_finalizado->metricas_tiempo);
	free(proceso_finalizado);
}

void recibir_respuesta_dumpeo(t_respuesta_dump* resp_dump, t_pcb* proceso_desbloqueado) {
    if(resp_dump->respuesta == 0){
	    log_debug(logger_kernel, "Fallo en el DUMP");	

		free(resp_dump);
		pasar_pcb_blocked_a_exit(proceso_desbloqueado);
			
	} else {
		log_debug(logger_kernel, "Acierto en el DUMP");

		free(resp_dump);
		pasar_pcb_blocked_a_ready(proceso_desbloqueado);
				
    }
}

char* recibir_respuesta_memoria(int socket_cliente) { 
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);

    uint32_t tamanio_mensaje;
    memcpy(&tamanio_mensaje, buffer, sizeof(uint32_t));

    char* mensaje = malloc(tamanio_mensaje);
    memcpy(mensaje, buffer + sizeof(uint32_t), tamanio_mensaje);

    free(buffer); 
    return mensaje; 
}