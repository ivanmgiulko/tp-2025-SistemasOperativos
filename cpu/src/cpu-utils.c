#include "./cpu-utils.h"

// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_instruccion_a_memoria(t_peticion_instruccion* infoPCB){
	log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");
	sem_wait(&sem_cpu);
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");

	//crea la peticion
    

	//Serializa la petición
	int size_peticion = 0;
	void* peticion_serializada = serializar_peticion_instruccion(infoPCB, &size_peticion);
    if(peticion_serializada == NULL) {
        log_warning(logger_cpu, "Error al serializar la peticion de instruccion");
        return;
	}
	
	//Envía la peticion serializada a MEMORIA
	log_info(logger_cpu, "Size_peticion= %d", size_peticion);
	log_debug(logger_cpu, "Petición envíada, aguardo respuesta");
	send(fd_conexion_memoria, peticion_serializada, size_peticion, 0);
	free(peticion_serializada);

	/* //Recibe la instruccion serializada desde MEMORIA
	int size_respuesta;
	log_info(logger_cpu, "Esperando respuesta de memoria");
	void* buffer_respuesta = recibir_buffer(&size_respuesta, fd_conexion_memoria);
	
	t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(buffer_respuesta);
 	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);
 
 	free(respuesta->instruccion);
 	free(respuesta);
 	free(buffer_respuesta); */
}	
	

void manejar_respuesta_de_instruccion(t_paquete* paquete){
    log_info(logger_cpu, "Iniciando deserializacion del paquete de instruccion");

	//Deserializa la instrucción recibida
	t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(paquete->buffer->stream);
	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);

	t_instruccion* instruccion = decode(respuesta->instruccion);
	if (!instruccion) {
        log_error(logger_cpu, "Error al decodificar la instrucción");
        // No olvides liberar respuesta->instruccion antes de salir
        free(respuesta->instruccion);
        free(respuesta);
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return;  // Finaliza la función si no se pudo decodificar la instrucción
    }
	log_debug(logger_cpu, "Instrucción decodificada: %d", instruccion->tipo);

	switch(instruccion->tipo) {
		case INSTR_NOOP:
			log_info(logger_cpu, "Ejecutando instrucción NOOP con parametros");
			break;
		case INSTR_WRITE:
			log_info(logger_cpu, "Ejecutando instrucción WRITE con parametros %s %s",
				instruccion->parametros.write.datos, instruccion->parametros.write.direccion);
			break;
		case INSTR_READ:
			log_info(logger_cpu, "Ejecutando instrucción READ con parametros %s %d",
				instruccion->parametros.read.direccion, instruccion->parametros.read.tamanio);
			break;
		case INSTR_GOTO:
			log_info(logger_cpu, "Ejecutando instrucción GOTO con parametros %d",
				instruccion->parametros.go_to.valor);
			break;

			////////////////////////////
			/////////SYSCALLS///////////
			////////////////////////////
		
		case INSTR_IO:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.io.dispositivo, instruccion->parametros.io.tiempo);
			break;
		case INSTR_INIT_PROC:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.init_proc.archivo, instruccion->parametros.init_proc.tamanio);
			break;
		case INSTR_DUMP_MEMORY:
			log_info(logger_cpu, "syscall detectada... parametros ");
			break;
		case INSTR_EXIT:
			log_info(logger_cpu, "syscall detectada... parametros ");
			break;
		default:
			log_error(logger_cpu, "Instrucción desconocida: %d", instruccion->tipo);
			break;
	}
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(respuesta->instruccion);
	free(respuesta);
}