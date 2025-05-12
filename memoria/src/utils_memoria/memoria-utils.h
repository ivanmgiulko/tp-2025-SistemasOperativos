#ifndef MEMORIA_UTILS_H_
#define MEMORIA_UTILS_H_

    /**
	 * @file
	 * @brief #include "utils_memoria/memoria-utils.h"
	 */

typedef struct {
    int pid;
    char** instrucciones;           // array dinámico de instrucciones
    int cant_instrucciones;
} t_procesoEnMemoria;

typedef struct {
    t_procesoEnMemoria* procesos;              // array dinámico de procesos
    int cant_procesos;
} t_memoriaDelSistema;

#define MAX_LINEA 256
char** leer_instrucciones(const char* pathArchivoPseudocodigo int* cantidad);
void agregar_proceso(t_memoriaDelSistema* memoria, t_pcbMemoria* pcb);
int finalizar_proceso(t_memoriaDelSistema* memoria, int pid);
char* obtener_instruccion(t_memoriaDelSistema* memoria, int pid, int pc);

#endif // MEMORIA_UTILS_H_