#include "contrato.h"

void* serializar_peticion_instruccion(t_peticion_instruccion* peticion, int* bytes) {

    t_paquete* paquete = crear_paquete_instruccion();
    agregar_a_paquete(paquete, &(peticion->pid), sizeof(uint8_t));
    agregar_a_paquete(paquete, &(peticion->pc), sizeof(uint16_t));
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
t_peticion_instruccion* deserializar_peticion_instruccion(void* buffer) {
    t_peticion_instruccion* peticion = malloc(sizeof(t_peticion_instruccion));
    uint32_t desplazamiento = 0;
    uint32_t tamanio;

    
    // Leer tamaño del PID (y descartarlo)
    memcpy(&tamanio, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer PID
    memcpy(&(peticion->pid), buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;
    // Leer tamaño del PC
    memcpy(&tamanio, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    
    // Leer PC
    memcpy(&(peticion->pc), buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;
    
    
    return peticion;
}
