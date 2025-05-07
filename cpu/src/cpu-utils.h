#ifndef CPU_UTILS_H
#define CPU_UTILS_H
#include "./cpu-gestor.h"
#include "./instrucciones.h"

void pedir_instruccion_a_memoria(t_peticion_instruccion*);
void manejar_respuesta_de_instruccion(t_paquete* paquete);

#endif // CPU_UTILS_H