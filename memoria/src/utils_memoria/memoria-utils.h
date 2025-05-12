#ifndef MEMORIA_UTILS_H_
#define MEMORIA_UTILS_H_
#define MAX_LINEA 256
#include "memoria-gestor.h"

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

extern t_memoriaDelSistema* memoriaDelSistema; // variable global para almacenar la memoria del sistema

char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad);
void agregar_proceso(t_pcbMemoria* pcb);
int finalizar_proceso(int pid);
char* obtener_instruccion(int pid, int pc);
t_memoriaDelSistema crear_memoria_del_sistema();

#endif // MEMORIA_UTILS_H_