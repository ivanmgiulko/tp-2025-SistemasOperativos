#include "cpu-k-dispatch.h"

int manejar_conexion_kernel_dispatch(){
	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_kernel_dispatch);

		switch (paquete->codigo_operacion) {

			case MENSAJE:
				recibir_mensaje(fd_conexion_kernel_dispatch, logger_cpu);
				break;
				
			case INFO_PROC_EXEC:
				//sem_wait(&sem_cpu_kernel);
				log_debug(logger_cpu, "Recibiendo INFO_PROC_EXEC desde Kernel");
				recibir_paquete(fd_conexion_kernel_dispatch, paquete);
				
				t_peticion_instruccion* infoPCB = deserializar_info_pcb(paquete->buffer);
						
				if(pcb_actual->pid != infoPCB->pid && !flag_exit){
					log_error(logger_cpu, "PID DISTINTO AL ANTERIOR");

					if(cache_esta_activada()){
						log_error(logger_cpu, "SE ENTRA A cache_esta_activada");
						actualizar_memoria_principal_completa();
					}
					if(tlb_esta_activada())
						limpiar_tlb();
					
				}
				pthread_mutex_lock(&mutex_cpu);
				pcb_actual->pid = infoPCB->pid;
				pcb_actual->pc = infoPCB->pc;
				pthread_mutex_unlock(&mutex_cpu);

				pedir_instruccion_a_memoria(infoPCB);
				flag_exit = false;	
				free(infoPCB);
				
				eliminar_paquete(paquete);
				
				break;

			case INSTRUCCION:
				break;
			
			case -1:
				log_error(logger_cpu, "el cliente se desconecto.");
				return EXIT_FAILURE;
			default:
				log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
	close(fd_conexion_kernel_dispatch);
	return EXIT_SUCCESS;
}

t_peticion_instruccion* deserializar_info_pcb(t_buffer* buffer) {
	t_peticion_instruccion* infoPCB = malloc(sizeof(t_peticion_instruccion));
    void* stream = buffer->stream;

	memcpy(&(infoPCB->pc), stream, sizeof(uint16_t)); stream += sizeof(uint16_t);

    memcpy(&(infoPCB->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
   
    return infoPCB;

}



