#include "cpu-k-dispatch.h"

int manejar_conexion_kernel_dispatch(){

	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(fd_conexion_kernel_dispatch);

		switch (paquete->codigo_operacion) {

			case MENSAJE:
				recibir_mensaje(fd_conexion_kernel_interrupt, logger_cpu);
				break;
				
			case INFO_PROC_EXEC:
				recv(fd_conexion_kernel_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				recv(fd_conexion_kernel_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
				log_trace(logger_cpu, "llegue desde Kernel");
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


