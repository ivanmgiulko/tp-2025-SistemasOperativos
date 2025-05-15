#include "cpu-memoria.h"


int manejar_conexion_memoria(){
    while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(fd_conexion_memoria);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
		
			recibir_mensaje(fd_conexion_memoria, logger_cpu);
			break;
		case INSTRUCCION:
				log_info(logger_cpu, "Recibi la instrucción solicitada a MEMORIA");
				t_paquete* paquete_instruccion = recibir_paquete_instruccion(fd_conexion_memoria);
				if (paquete_instruccion == NULL) {
        			log_error(logger_cpu, "Error al recibir el paquete de instrucción");
        			break;
    			}
				log_info(logger_cpu, "Tamaño del buffer recibido: %d", paquete_instruccion->buffer->size);				
				manejar_respuesta_de_instruccion(paquete_instruccion);
				break;
		case FIN_PID:
			log_info(logger_cpu, "Recibi el fin de PID");
			free(paquete->buffer);
			free(paquete);
			break;
		case -1:
			log_error(logger_cpu, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_cpu, "Operacion desconocida #%d#. No quieras meter la pata", paquete->codigo_operacion);
			break;
		}
	}

	close(fd_conexion_memoria);
	return EXIT_SUCCESS;
}