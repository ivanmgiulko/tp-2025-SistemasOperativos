#include "cpu-k-interrupt.h"
// logger

int manejar_conexion_kernel_interrupt(){
   while (1) {
        t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);
		paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_kernel_interrupt);

		switch (paquete->codigo_operacion) {
			case MENSAJE:
				recibir_mensaje(fd_conexion_kernel_interrupt, logger_cpu);
				
				free(paquete);
				break;
			
			case PROCESO_DESALOJAR:
			

			//	log_warning(logger_cpu, "NIGGA HAY QUE DESALOJAR EL PROCESO");
				flag_interrupt = true;
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
