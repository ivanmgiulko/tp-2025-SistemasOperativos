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
void enviar_proceso_desalojado(int , int , int );







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
void recibir_datos_de_memoria(mmu_t*);

//FUNCIONES Y DEFINICIONES DE DIRECCIONES
typedef struct {
    uint32_t  nro_pagina;
    uint32_t  desplazamiento;
    uint32_t  * entrada_nivel; // entradas de cada nivel
} t_pre_direccion_fisica;

t_pre_direccion_fisica calcular_pre_direccion_fisica(int direccion_logica);
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica);


//FUNCIONES Y DEFINICIONES DE TLB

typedef enum {
    FIFO,
    LRU,
} algoritmo_tlb_t;

typedef struct{
    uint32_t nro_pagina;
    uint32_t marco_asociado;
    int bit_en_uso;
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
typedef struct{
    int nro_pagina;
    void* contenido;
    bool bit_uso;        // CLOCK y CLOCK-M
    bool bit_modificado; // solo CLOCK-M
} t_pagina_de_cache;

typedef struct{
    uint32_t cantidad_paginas;
    t_pagina_de_cache* paginas;
} t_memoria_cache;

extern t_memoria_cache* memoria_cache;

t_memoria_cache* inicializar_cache(uint32_t cant_paginas, uint32_t tam_pagina);
void destruir_cache(t_memoria_cache* cache, uint32_t tam_pagina);
int buscar_pagina_en_cache(t_memoria_cache* cache, int nro_pagina_buscado); 
//void actualizar_memoria_principal();


#endif // CPU_UTILS_H