#include "./cpu-utils.h"

// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_instruccion_a_memoria(t_peticion_instruccion* infoPCB){
	log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");
	sem_wait(&sem_cpu);
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");

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

	ejecutar_instruccion(instruccion);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(respuesta->instruccion);
	free(respuesta);
}