#include "cpu-memoria.h"


int manejar_conexion_memoria(){
    while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(fd_conexion_memoria);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
		
			recibir_mensaje(fd_conexion_memoria, logger_cpu);
			
			free(paquete);
			break;
		case INSTRUCCION:
				log_info(logger_cpu, "Recibi la instrucción solicitada a MEMORIA");
				t_paquete* paquete_instruccion = recibir_paquete_instruccion(fd_conexion_memoria);
				if (paquete_instruccion == NULL) {
        			log_error(logger_cpu, "Error al recibir el paquete de instrucción");
        			break;
    			}
				log_info(logger_cpu, "Tamaño del buffer recibido: %d", paquete_instruccion->buffer->size);			
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);	
				manejar_respuesta_de_instruccion(paquete_instruccion);
				break;
		case WRITE_MEMORIA:
			uint32_t size_write;
				if (recv(fd_conexion_memoria, &size_write, sizeof(uint32_t), MSG_WAITALL) <= 0) {
					log_error(logger_cpu, "Error al recibir el tamaño del buffer de confirmación WRITE");
					break;
				}

			// Recibir contenido del buffer
			char* mensaje_confirmacion_write = malloc(size_write);
				if (recv(fd_conexion_memoria, mensaje_confirmacion_write, size_write, MSG_WAITALL) <= 0) {
					log_error(logger_cpu, "Error al recibir el mensaje de confirmación WRITE");
					free(mensaje_confirmacion_write);
					break;
				}

			log_info(logger_cpu, "Mensaje de confirmación: %s", mensaje_confirmacion_write);

			pcb_actual->pc++;
			sem_wait(&sem_write);
			sem_post(&sem_cpu);

			free(mensaje_confirmacion_write);
			free(paquete->buffer);
			free(paquete);
			break;
		case READ_MEMORIA:
			uint32_t size_read;
				if (recv(fd_conexion_memoria, &size_read, sizeof(uint32_t), MSG_WAITALL) <= 0) {
					log_error(logger_cpu, "Error al recibir el tamaño del buffer de confirmación READ");
					break;
				}
			
			char* mensaje_confirmacion_read = malloc(size_read);
				if (recv(fd_conexion_memoria, mensaje_confirmacion_read, size_read, MSG_WAITALL) <= 0) {
					log_error(logger_cpu, "Error al recibir el mensaje de confirmación READ");
					free(mensaje_confirmacion_read);
					break;
				}

			log_info(logger_cpu, "Mensaje de confirmación: %s", mensaje_confirmacion_read);

			pcb_actual->pc++;
			sem_wait(&sem_read);
			sem_post(&sem_cpu);
    		

			free(mensaje_confirmacion_read);
			free(paquete->buffer);
			free(paquete);
			break;
		case FIN_PID:
			log_info(logger_cpu, "Recibi el fin de PID");
			sem_post(&sem_cpu_kernel);
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