#include "contrato.h"

void* serializar_peticion_instruccion(t_peticion_instruccion* peticion, int* bytes) {

    t_paquete* paquete = crear_paquete_instruccion();
    agregar_a_paquete(paquete, &(peticion->pid), sizeof(int));
    agregar_a_paquete(paquete, &(peticion->pc), sizeof(int));
    if(paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        return NULL;
    }


    *bytes = sizeof(int) + sizeof(uint32_t) + paquete->buffer->size;
    void* peticion_serializada = serializar_paquete(paquete, *bytes);
    if(peticion_serializada == NULL) {
        return NULL;
    }
    
    return peticion_serializada;

}

void* serializar_respuesta_instruccion(t_respuesta_instruccion* respuesta, int* bytes) {

    t_paquete* paquete = crear_paquete_instruccion();

    int longitud = strlen(respuesta->instruccion) + 1;
    agregar_a_paquete(paquete, respuesta->instruccion, longitud);
    
    if(paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        return NULL;
    }

    *bytes = sizeof(op_code) + sizeof(uint32_t) + paquete->buffer->size;

    void* respuesta_serializada = serializar_paquete(paquete, *bytes);
    if(respuesta_serializada == NULL) {
        return NULL;
    }
    
    // Liberar memoria del paquete
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return respuesta_serializada;

}

// t_peticion_instruccion* deserializar_peticion_instruccion(void* stream) {
//     t_peticion_instruccion* peticion = malloc(sizeof(t_peticion_instruccion));
//     memcpy(&(peticion->pid), stream, sizeof(int));
//     memcpy(&(peticion->pc), stream + sizeof(int), sizeof(int));
//     return peticion;
// }

t_peticion_instruccion* deserializar_peticion_instruccion(void* buffer) {
    t_peticion_instruccion* peticion = malloc(sizeof(t_peticion_instruccion));
    int desplazamiento = 0;
    int tamanio;

    // Leer tamaño del PID (y descartarlo)
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Leer PID
    memcpy(&(peticion->pid), buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;

    // Leer tamaño del PC
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Leer PC
    memcpy(&(peticion->pc), buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;

    return peticion;
}

t_respuesta_instruccion* deserializar_respuesta_instruccion(void* stream) {
    t_respuesta_instruccion* respuesta = malloc(sizeof(t_respuesta_instruccion));

    // Leer la longitud de la instrucción (el primer "int" del stream)
    int longitud;
    memcpy(&longitud, stream, sizeof(int));
    //log_info(logger_cpu, "Longitud de instrucción recibida: %d", longitud);

    // Validación de la longitud, por si es inválida
    if (longitud <= 0 || longitud > 1024) {
        //log_error(logger_cpu, "Longitud de instrucción inválida: %d", longitud);
        free(respuesta);
        return NULL;
    }

    // Asignar memoria para la instrucción recibida
    respuesta->instruccion = malloc(longitud);

    // Copiar la instrucción desde el stream (saltando los primeros 4 bytes que son la longitud)
    memcpy(respuesta->instruccion, stream + sizeof(int), longitud);

    // Retornar el struct de respuesta con la instrucción deserializada
    return respuesta;
}

// char* obtener_instruccion(int pid, int pc, char* path_pseudocodigos, t_log* logger_memoria) {
//     char path_archivo[256];
//     //sprintf(path_archivo, "%s/%d.txt", path_pseudocodigos, pid);

//     FILE* archivo = fopen(path_pseudocodigos, "r");
//     if (archivo == NULL) {
//         log_error(logger_memoria, "No se pudo abrir el archivo de pseudocodigo: %s", path_archivo);
//         return strdup("INSTRUCCION_NO_ENCONTRADA");
//     }

//     char* linea = NULL;
//     size_t len = 0;
//     int linea_actual = 0;
//     char* instruccion = NULL;

//     //Recorre el archivo linea x linea hasta llegar a la linea de pc o acabar las lineas
//     while (getline(&linea, &len, archivo) != -1) {
//         if (linea_actual == pc) {
//             instruccion = strdup(linea);
//             break;
//         }
//         linea_actual++;
//     }

//     fclose(archivo);
//     if (linea) free(linea);// Verifica que no haga free(NULL)

//     //Si no encontro la linea ( pc > a cant de lineas)
//     if (!instruccion) {
//         instruccion = strdup("INSTRUCCION_NO_ENCONTRADA");
//     }


//     return instruccion;
// }
