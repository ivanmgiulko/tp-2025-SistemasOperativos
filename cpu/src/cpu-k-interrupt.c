#include "cpu-k-interrupt.h"
// logger

int manejar_conexion_kernel_interrupt(){
    
	while (1) {
		int cod_op = recibir_operacion(fd_conexion_kernel_interrupt);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_conexion_kernel_interrupt, logger_cpu);
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

	close(fd_conexion_kernel_interrupt);
	return EXIT_SUCCESS;
}
// config