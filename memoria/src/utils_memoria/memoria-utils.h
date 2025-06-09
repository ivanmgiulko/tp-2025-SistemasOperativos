#ifndef MEMORIA_UTILS_H_
#define MEMORIA_UTILS_H_
#define MAX_LINEA 256
#include "memoria-gestor.h"
#include "config_memoria/memoria-config.h"

    /**
	 * @file
     * @brief #include "utils_memoria/memoria-utils.h"
	 */

typedef enum {
    NIVEL_INTERMEDIO,
    NIVEL_FINAL
} tipo_tabla;

typedef struct {
    bool presente;
    uint32_t marco;
    bool uso;
    bool modificado;
} t_entrada_pagina;

typedef struct t_tabla_pagina {
    tipo_tabla tipo;
    union {
        struct t_tabla_pagina** subtablas;       // si es NIVEL_INTERMEDIO entonces tengo más tablas
        t_entrada_pagina* entradas;              // si es NIVEL_FINAL entonces tengo las entradas de la tabla
    };
    int cantidad_entradas;
} t_tabla_pagina;

typedef struct {
    int pid;
    char** instrucciones;           // array dinámico de instrucciones
    int cant_instrucciones;
    metricas_proceso metricas_proceso;
    t_tabla_pagina* tabla_primera; // una tabla de paginas raíz por cada proceso
} t_proceso_en_memoria;

typedef struct {
    t_proceso_en_memoria* procesos;              // array dinámico de procesos
    int cant_procesos;
    pthread_mutex_t mutex; 
} t_memoria_del_sistema;

extern t_memoria_del_sistema* memoria_del_sistema; // variable global para almacenar la memoria del sistema

char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad);
char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento);
void agregar_proceso(t_pcbMemoria* pcb);
int finalizar_proceso(int pid);
char* obtener_instruccion(int pid, int pc);
t_memoria_del_sistema crear_memoria_del_sistema();
t_tabla_pagina* crear_tabla_paginacion(int nivel_actual, int cantidad_niveles, int entradas_por_tabla);
void liberar_tabla(t_tabla_pagina* tabla);
t_proceso_en_memoria* buscar_proceso_en_memoria(int pid);

#endif // MEMORIA_UTILS_H_