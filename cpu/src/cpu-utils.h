#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "./cpu-gestor.h"
#include "./instrucciones.h"


void pedir_instruccion_a_memoria(t_peticion_instruccion*);

void manejar_respuesta_de_instruccion(t_paquete* paquete);

void _crear_conexion_kernel_interrupt(char*, char*, char*);

void _crear_conexion_kernel_dispatch(char*, char*, char*);

void _crear_conexion_cpu_memoria(char*, char*);

void _handshake_kernel_con_cpu_id(int, char*);

void check_interrupt();
void enviar_proceso_desalojado(int , uint8_t , uint16_t );

void enviar_read_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, uint32_t tamanio);
void enviar_write_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, char* datos, uint32_t tamanio);



char* deserializar_read_o_write_de_memoria(t_paquete* paquete);

//FUNCIONES Y DEFINICIONES DE MMU
typedef struct{
    uint32_t tamanio_pagina;
    uint32_t cantidad_entradas_tabla;
    uint32_t cantidad_niveles;
    uint32_t ultima_direccion_fisica_calculada;
}mmu_t;

extern mmu_t* mmu;

mmu_t* inicializar_mmu();
void destruir_mmu(mmu_t* mmu);
void pedir_datos_a_memoria(char* , int );
void recibir_datos_de_memoria(t_paquete* paquete, mmu_t*);


//FUNCIONES Y DEFINICIONES DE DIRECCIONES
typedef struct {
    uint32_t  nro_pagina;
    uint32_t  desplazamiento;
    uint32_t  * entrada_nivel; // entradas de cada nivel
} t_pre_direccion_fisica;

t_pre_direccion_fisica calcular_pre_direccion_fisica(int direccion_logica);
uint32_t* calcular_entradas_por_nivel(int nro_pagina, int cantidad_niveles, int cant_entradas_tabla);
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica);

int32_t solicitar_marco_a_memoria(t_pre_direccion_fisica pre_direccion_fisica, uint8_t pid);
int32_t recibir_marco_solicitado(t_paquete* paquete);

//FUNCIONES Y DEFINICIONES DE TLB

typedef enum {
    FIFO,
    LRU,
} algoritmo_tlb_t;

typedef struct{
    uint32_t nro_pagina;
    uint32_t marco_asociado;
    uint32_t bit_en_uso;
    uint32_t instante_referencia;
}entradas_tlb_t;

typedef struct{
    entradas_tlb_t* entradas;
    uint32_t cantidad_entradas;
}tlb_t;


extern tlb_t* tlb;
extern algoritmo_tlb_t algoritmo;

algoritmo_tlb_t algoritmo_from_string(const char* str);
tlb_t* inicializar_tlb(uint32_t);
void destruir_tlb(tlb_t* tlb);
int esta_en_tlb(uint32_t );
uint32_t tlb_miss(t_pre_direccion_fisica);
void agregar_a_tlb(uint32_t , uint32_t , algoritmo_tlb_t);
void agregar_a_tlb_fifo(uint32_t , uint32_t );
void agregar_a_tlb_lru(uint32_t , uint32_t );
extern uint32_t proxima_a_reemplazar;
extern uint32_t contador_accesos_tlb;
void limpiar_tlb();


//FUNCIONES Y DEFINICIONES DE CACHE
typedef enum {
    CLOCK,
    CLOCK_M
} t_algoritmo_cache;

typedef struct{
    int nro_pagina;
    char* contenido;
    bool bit_uso;        // CLOCK y CLOCK-M
    bool bit_modificado; // solo CLOCK-M
} t_pagina_de_cache;

typedef struct{
    int puntero_reemplazo;
    t_algoritmo_cache algoritmo_reemplazo;
    uint32_t cantidad_paginas;
    t_pagina_de_cache* paginas;
    uint32_t retardo;
} t_memoria_cache;

extern t_memoria_cache* memoria_cache;

t_algoritmo_cache algoritmo_cache_from_string(char* str);
t_memoria_cache* inicializar_cache(char* algoritmo, uint32_t cant_paginas, uint32_t tam_pagina, uint32_t retardo);
void destruir_cache(t_memoria_cache* cache, uint32_t tam_pagina);
bool cache_esta_activada();

int buscar_pagina_en_cache(t_memoria_cache* cache, int nro_pagina_buscado); 
char* leer_de_cache(int indice, uint32_t desplazamiento, uint32_t tamanio_a_leer);
void escribir_en_cache(int indice, uint32_t desplazamiento, char* datos_a_escribir);

int buscar_espacio_libre_en_cache(t_memoria_cache* cache);
void agregar_pagina_a_cache(uint32_t nro_pagina, int indice_libre, char* contenido);
int reemplazo_clock(t_memoria_cache* cache);

int manejar_cache_miss(t_pre_direccion_fisica pre_direccion_fisica);
void actualizar_memoria_principal_completa();


#endif // CPU_UTILS_H