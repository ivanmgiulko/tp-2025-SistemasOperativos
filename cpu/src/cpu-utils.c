#include "./cpu-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_instruccion_a_memoria(){
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");
    t_peticion_instruccion peticion = {
        .pid = 15,
        .pc = 10
    };

	int size_peticion = 0;
	void* peticion_serializada = serializar_peticion_instruccion(&peticion, &size_peticion);
    if(peticion_serializada == NULL) {
        log_warning(logger_cpu, "Error al serializar la peticion de instruccion");
        return;
	}
	log_info(logger_cpu, "Peticion serializada de instruccion: %d", size_peticion);
	send(fd_conexion_memoria, peticion_serializada, size_peticion, 0);
	free(peticion_serializada);

	int size_respuesta;
	log_info(logger_cpu, "Esperando respuesta de memoria");
	void* buffer_respuesta = recibir_buffer(&size_respuesta, fd_conexion_memoria);
	log_info(logger_cpu, "pedilo lean");

	t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(buffer_respuesta);
	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);

	free(respuesta->instruccion);
	free(respuesta);
	free(buffer_respuesta);
}