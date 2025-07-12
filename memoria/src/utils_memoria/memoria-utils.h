#ifndef MEMORIA_UTILS_H_
#define MEMORIA_UTILS_H_
#define MAX_LINEA 256
#include <math.h>
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
    int32_t marco;
    uint32_t num_pagina;
    bool uso;
    bool modificado;
} t_entrada_pagina;

typedef struct t_tabla_pagina {
    tipo_tabla tipo;
    union {
        struct t_tabla_pagina** subtablas;       // si es NIVEL_INTERMEDIO entonces tengo más tablas
        t_entrada_pagina* entradas;              // si es NIVEL_FINAL entonces tengo las entradas de la tabla
    };
    int cant_entradas;
} t_tabla_pagina;

typedef struct {
    uint32_t  nro_pagina;
    uint32_t  desplazamiento;
    uint32_t  * entrada_nivel;
} t_pre_direccion_fisica;

typedef struct {
    int pid;
    char** instrucciones;           // array dinámico de instrucciones
    int cant_instrucciones;
    metricas_proceso metricas_proceso;
    t_tabla_pagina* tabla_primera; // una tabla de paginas raíz por cada proceso
    uint32_t tamanioMemoria;
} t_proceso_en_memoria;

typedef struct {
    uint32_t pid;
    uint32_t* posiciones_swap; // índice de cada página del proceso en SWAP
    uint32_t cantidad_paginas;
} t_proceso_swap;

typedef struct {
    t_proceso_en_memoria* procesos;              // array dinámico de procesos
    int cant_procesos;
    void* memoria_principal;
    int tam_memoria;
    int tam_pagina;
    int cant_marcos;
    bool* bitmap_marcos_ocupados;
    pthread_mutex_t mutex; 
} t_memoria_del_sistema;

extern t_memoria_del_sistema* memoria_del_sistema; // variable global para almacenar la memoria del sistema


//Funciones de serializacion
char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento);
uint32_t  leer_uint32_desde_buffer(t_buffer* buffer, int* desplazamiento);

//funciones de buscar instrucciones
char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad);
char* obtener_instruccion(int pid, int pc);

//funciones de Inicio/fin/susp procesos
void agregar_proceso(t_pcbMemoria* pcb);
void informar_metricas_memoria(int pid);
int finalizar_proceso(int pid);

//Funciones de memoria
t_memoria_del_sistema crear_memoria_del_sistema();
t_tabla_pagina* crear_tabla_paginacion(int nivel_actual, int cantidad_niveles, int entradas_por_tabla, int* pagina_actual, int paginas_totales);
t_proceso_en_memoria* buscar_proceso_en_memoria(int pid);
int buscar_indice_de_proceso_en_memoria(int pid);

//Funciones de tablas
void liberar_tabla(t_tabla_pagina* tabla);
void asignar_marcos_tabla(t_tabla_pagina* tabla, t_memoria_del_sistema* memoria, int paginas_necesarias, int* paginas_asignadas);
void liberar_marcos_tabla(t_tabla_pagina* tabla, t_memoria_del_sistema* memoria);
int buscar_marco_libre(t_memoria_del_sistema* memoria);
int32_t buscar_marco_en_tabla(t_tabla_pagina* tabla_primera, uint32_t* entradas_por_nivel, int cantidad_niveles);
int buscar_marco_en_tabla_full(t_tabla_pagina* tabla_primera, int nro_pagina, int cantidad_niveles, int entradas_por_tabla);
void liberar_espacios_memoria_usuario(t_tabla_pagina* tabla_primera,t_memoria_del_sistema* memoria_del_sistema);

// Funciones de SWAP
void inicializar_swap();
void suspender_proceso(int pid);
void desuspender_proceso_swap(int pid);
t_proceso_swap* buscar_proceso_en_swap(int pid);

//FUNCIONES DE CONEXION
void manejar_peticion_de_instruccion(int socket_cliente,t_paquete* paquete);

void manejar_escritura_memoria(int socket_cliente, t_paquete* paquete);

void manejar_lectura_memoria(int socket_cliente, t_paquete* paquete);

void manejar_acceso_tablas_de_paginas(int socket_cliente, t_paquete* paquete);

void enviar_respuesta_kernel(char* mensaje, int socket_cliente);

void enviar_proceso_terminado(uint8_t pid, int socket_cliente);

t_pcb* recibir_proceso_a_dumpear_desde_kernel(t_buffer* buffer);

void enviar_respuesta_dump_memory(uint8_t pid, bool respuesta, int socket_cliente);

bool realizar_dump_memory(int pid);

void avisar_kernel_mande_otro_proceso(int socket_cliente);

void enviar_datos_a_cpu(int );
                
uint32_t* calcular_entradas_por_nivel(uint32_t nro_pagina, int cantidad_niveles, int entradas_por_tabla);

#endif // MEMORIA_UTILS_H_