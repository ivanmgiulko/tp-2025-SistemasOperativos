#include "cpu-memoria.h"


int manejar_conexion_memoria(){
    while (1) {
		int cod_op = recibir_operacion(fd_conexion_memoria);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_conexion_memoria, logger_cpu);
			break;
		case INSTRUCCION:
				log_info(logger_cpu, "Recibi la instrucción solicitada a MEMORIA");
				t_paquete* paquete = recibir_paquete_instruccion(fd_conexion_memoria);
				if (paquete == NULL) {
        			log_error(logger_cpu, "Error al recibir el paquete de instrucción");
        			break;
    			}
				log_info(logger_cpu, "Tamaño del buffer recibido: %d", paquete->buffer->size);				
				manejar_respuesta_de_instruccion(paquete);
				break;
		case -1:
			log_error(logger_cpu, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(fd_conexion_memoria);
	return EXIT_SUCCESS;
}