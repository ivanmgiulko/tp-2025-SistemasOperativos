#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-memoria/modulo-memoria.h>
#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
// #include <utils_kernel/utils-complementarios/conexion-con-memoria

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

			t_pcb* proceso_desbloqueado = buscar_proceso_en_cola(estado_blocked, resp_dump->pid);

			if(proceso_desbloqueado == NULL) proceso_desbloqueado = buscar_proceso_en_cola(estado_susp_blocked, resp_dump->pid);

			recibir_respuesta_dumpeo(resp_dump, proceso_desbloqueado);

			eliminar_paquete(paquete);
			return EXIT_SUCCESS;

			break;
		
		case SUSPENSION_HECHA:

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

			free(offset);
			
			t_pcb* proceso_finalizado = _sacar_pcb_de_cola(pid, estado_exit);
			temporal_stop(proceso_finalizado->metricas_tiempo->tiempoEnExit);

			loguear_y_finalizar_proceso(proceso_finalizado);

			sem_post(&sem_hay_espacio_en_memoria);
			sem_post(&sem_cantidad_pcbs_en_new);
			sem_post(&bin_proceso_eliminar);
			
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