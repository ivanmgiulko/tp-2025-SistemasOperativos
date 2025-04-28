#include"memoria-header.h"
#include <utils/serializacion/serializacion.h>
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

void manejar_conexion_memoria(int socket_cliente) {
    int size;
    void* buffer = recibir_buffer(&size, socket_cliente);

    t_peticion_instruccion* peticion = deserializar_peticion_instruccion(buffer);
    free(buffer);

    char* path_pseudocodigos = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    char* instruccion = obtener_instruccion(peticion->pid, peticion->pc, path_pseudocodigos);

    t_respuesta_instruccion respuesta = { .instruccion = instruccion };

    int size_respuesta;
    void* buffer_respuesta = serializar_respuesta_instruccion(&respuesta, &size_respuesta);
    send(socket_cliente, buffer_respuesta, size_respuesta, 0);

    free(instruccion);
    free(buffer_respuesta);
    free(peticion);
}
