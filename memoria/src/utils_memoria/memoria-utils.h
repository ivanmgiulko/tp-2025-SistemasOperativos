#ifndef MEMORIA_UTILS_H_
#define MEMORIA_UTILS_H_
#define MAX_LINEA 256
#include "memoria-gestor.h"

    /**
	 * @file
     * @brief #include "utils_memoria/memoria-utils.h"
	 */

typedef struct {
    char* direccion;
    char* datos;
} t_write;

typedef struct {
    int pid;
    char** instrucciones;           // array dinámico de instrucciones
    int cant_instrucciones;
} t_proceso_en_memoria;

typedef struct {
    t_proceso_en_memoria* procesos;              // array dinámico de procesos
    int cant_procesos;
} t_memoria_del_sistema;

extern t_memoria_del_sistema* memoria_del_sistema; // variable global para almacenar la memoria del sistema

char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad);
char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento);
void agregar_proceso(t_pcbMemoria* pcb);
int finalizar_proceso(int pid);
char* obtener_instruccion(int pid, int pc);
t_memoria_del_sistema crear_memoria_del_sistema();

#endif // MEMORIA_UTILS_H_