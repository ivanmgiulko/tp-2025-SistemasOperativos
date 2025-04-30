#include "./cpu-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_instruccion_a_memoria(int fd_conexion_memoria){
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");
    t_peticion_instruccion peticion = {
        .pid = 1,
        .pc = 0
    };

	int size_peticion = 0;
	void* peticion_serializada = serializar_peticion_instruccion(&peticion, &size_peticion);
    if(peticion_serializada == NULL) {
        log_info(logger_cpu, "Error al serializar la peticion de instruccion");
        return;
    }
	send(fd_conexion_memoria, peticion_serializada, size_peticion, 0);
	free(peticion_serializada);

	int size_respuesta;
	void* buffer_respuesta = recibir_buffer(&size_respuesta, fd_conexion_memoria);

	t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(buffer_respuesta);
	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);

	free(respuesta->instruccion);
	free(respuesta);
	free(buffer_respuesta);
}