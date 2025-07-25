#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-memoria/modulo-memoria.h>
#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
// #include <utils_kernel/utils-complementarios/conexion-con-memoria

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

int manejar_conexion_kernel_memoria(int socket_cliente){
	while (1) {
		
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(socket_cliente);

		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_kernel);
			free(paquete);
			break;

		case PROCESO_MEMORIA:
			char* validacion_espacio = recibir_respuesta_memoria(socket_cliente);
			printf("Validación de espacio en memoria: %s\n", validacion_espacio);
			if(strcmp(validacion_espacio, "No hay espacio en memoria") == 0) {
				eliminar_paquete(paquete);
				free(validacion_espacio);
			
				return 0;
			} else {
				// El proceso pasa a la cola de Ready
				eliminar_paquete(paquete);
				free(validacion_espacio);
			
				return 1;
			}
			break;

		case RESPUESTA_DUMPEO:
			
			recibir_paquete(socket_cliente, paquete);
			
			t_respuesta_dump* resp_dump = recibir_respuesta_dump(paquete->buffer);

			t_pcb* proceso_desbloqueado = _sacar_pcb_de_cola(resp_dump->pid, estado_blocked);
			
			proceso_desbloqueado = list_get(estado_blocked->cola, 0);

			if(resp_dump->respuesta == false){

				free(resp_dump);
				pasar_pcb_blocked_a_exit(proceso_desbloqueado);
				log_debug(logger_kernel, "Fallo en el DUMP");	
			
			} else {

				free(resp_dump);
				pasar_pcb_blocked_a_ready(proceso_desbloqueado);
				log_debug(logger_kernel, "Acierto en el DUMP");
				
			}
			eliminar_paquete(paquete);
			return EXIT_SUCCESS;

			break;
		
		case SUSPENSION_HECHA:

			log_debug(logger_kernel, "EL PROCESO FUE SUSPENDIDO Y TENGO MAS MEMORIA AHORA");

			sem_post(&sem_cantidad_pcbs_en_new);
			sem_post(&sem_hay_espacio_en_memoria);

			free(paquete);

			return EXIT_SUCCESS;

			break;

		case PROCESO_FINALIZADO:
			recibir_paquete(socket_cliente, paquete);
			
			int* offset = malloc(sizeof(int));
			*offset = 0;

			uint8_t pid = _deserializar_pid(offset, paquete);
			
			log_info(logger_kernel, "%d - Finaliza el proceso", pid);

			t_pcb* proceso_finalizado = _sacar_pcb_de_cola(pid, estado_exit);
			temporal_stop(proceso_finalizado->metricas_tiempo->tiempoEnExit);

			log_info(logger_kernel, "%d - Metricas de estado: NEW [%d] [%ld], READY [%d] [%ld], BLOCKED [%d] [%ld], EXEC [%d] [%ld], EXIT [%d] [%ld], SUSP-READY [%d] [%ld], SUSP-BLOCKED [%d] [%ld]", 
			proceso_finalizado->pid, 
			proceso_finalizado->metricas_estado->cantVecesNew, 		   proceso_finalizado->metricas_tiempo->tiempoEnNew->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesReady,       proceso_finalizado->metricas_tiempo->tiempoEnReady->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesBlocked,     proceso_finalizado->metricas_tiempo->tiempoEnBlocked->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesExec, 	   proceso_finalizado->metricas_tiempo->tiempoEnExec->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesExit,        proceso_finalizado->metricas_tiempo->tiempoEnExit->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesSuspReady,   proceso_finalizado->metricas_tiempo->tiempoEnSuspReady->elapsed_ms,
			proceso_finalizado->metricas_estado->cantVecesSuspBlocked, proceso_finalizado->metricas_tiempo->tiempoEnSuspBlocked->elapsed_ms);

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
			
			sem_post(&sem_hay_espacio_en_memoria);
			sem_post(&sem_cantidad_pcbs_en_new);

			sem_post(&bin_proceso_eliminar);
			
			free(offset);

			eliminar_paquete(paquete);
			
			return EXIT_SUCCESS;
			break;
		
		case -1:
			log_error(logger_kernel, "el cliente [MEMORIA] se desconecto .");
			eliminar_paquete(paquete);
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			eliminar_paquete(paquete);
			break;
		}
	}

	close(fd_conexion_memoria);
	log_trace(logger_kernel, "Se cierra la conexion con Memoria");
	return EXIT_SUCCESS;
}