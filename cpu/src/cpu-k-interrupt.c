#include "cpu-k-interrupt.h"
// logger

int manejar_conexion_kernel_interrupt(){
   while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_kernel_interrupt);

		switch (paquete->codigo_operacion) {
			case MENSAJE:
				recibir_mensaje(fd_conexion_kernel_interrupt, logger_cpu);
				
				free(paquete);
				break;
			
			case PROCESO_DESALOJAR:


				log_warning(logger_cpu, "NIGGA HAY QUE DESALOJAR EL PROCESO");
				flag_interrupt = true;

				actualizar_memoria_principal_completa();
				for(uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
					log_trace(logger_cpu, "Bits de uso previo a limpiar: %d",tlb->entradas[i].bit_en_uso);

				}
				limpiar_tlb();
				for(uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
					log_trace(logger_cpu, "Bits de uso luego de limpiar: %d",tlb->entradas[i].bit_en_uso);

				}
				free(paquete);
			
				break;
				
			case -1:
				log_error(logger_cpu, "el cliente se desconecto.");
				return EXIT_FAILURE;
			default:
				log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
				break;
		}
	}

	close(fd_conexion_kernel_interrupt);
	return EXIT_SUCCESS;
}
