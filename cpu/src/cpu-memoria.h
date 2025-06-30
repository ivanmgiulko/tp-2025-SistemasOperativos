#ifndef CPU_MEMORIA_H_
#define CPU_MEMORIA_H_
#include "cpu-gestor.h"
#include "cpu-utils.h"

int manejar_conexion_memoria();

void* procesar_instruccion(void* arg);
void* procesar_read_memoria(void* arg);
void* procesar_write_memoria(void* arg);
void* procesar_obtener_marco(void*arg);

#endif // CPU_MEMORIA_H_