#include "contrato.h"

void* serializar_peticion_instruccion(t_peticion_instruccion* peticion, int* bytes) {
    *bytes = sizeof(int) * 2;
    void* buffer = malloc(*bytes);

    memcpy(buffer, &(peticion->pid), sizeof(int));
    memcpy(buffer + sizeof(int), &(peticion->pc), sizeof(int));

    return buffer;
}

t_peticion_instruccion* deserializar_peticion_instruccion(void* stream) {
    t_peticion_instruccion* peticion = malloc(sizeof(t_peticion_instruccion));
    memcpy(&(peticion->pid), stream, sizeof(int));
    memcpy(&(peticion->pc), stream + sizeof(int), sizeof(int));
    return peticion;
}

void* serializar_respuesta_instruccion(t_respuesta_instruccion* respuesta, int* bytes) {
    int longitud = strlen(respuesta->instruccion) + 1;
    *bytes = sizeof(int) + longitud;

    void* buffer = malloc(*bytes);
    memcpy(buffer, &longitud, sizeof(int));
    memcpy(buffer + sizeof(int), respuesta->instruccion, longitud);

    return buffer;
}

t_respuesta_instruccion* deserializar_respuesta_instruccion(void* stream) {
    t_respuesta_instruccion* respuesta = malloc(sizeof(t_respuesta_instruccion));
    int longitud;
    memcpy(&longitud, stream, sizeof(int));

    respuesta->instruccion = malloc(longitud);
    memcpy(respuesta->instruccion, stream + sizeof(int), longitud);

    return respuesta;
}

char* obtener_instruccion(int pid, int pc, char* path_pseudocodigos, t_log* logger_memoria) {
    char path_archivo[256];
    sprintf(path_archivo, "%s/%d.txt", path_pseudocodigos, pid);

    FILE* archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_error(logger_memoria, "No se pudo abrir el archivo de pseudocodigo: %s", path_archivo);
        return strdup("INSTRUCCION_NO_ENCONTRADA");
    }

    char* linea = NULL;
    size_t len = 0;
    int linea_actual = 0;
    char* instruccion = NULL;

    //Recorre el archivo linea x linea hasta llegar a la linea de pc o acabar las lineas
    while (getline(&linea, &len, archivo) != -1) {
        if (linea_actual == pc) {
            instruccion = strdup(linea);
            break;
        }
        linea_actual++;
    }

    fclose(archivo);
    if (linea) free(linea);// Verifica que no haga free(NULL)

    //Si no encontro la linea ( pc > a cant de lineas)
    if (!instruccion) {
        instruccion = strdup("INSTRUCCION_NO_ENCONTRADA");
    }


    return instruccion;
}