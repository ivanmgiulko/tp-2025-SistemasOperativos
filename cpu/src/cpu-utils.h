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

typedef struct {
    uint32_t  nro_pagina;
    uint32_t  desplazamiento;
    uint32_t  * entrada_nivel; // entradas de cada nivel
} t_direccion_fisica;

t_direccion_fisica calcular_direccion_fisica(int direccion_logica);

typedef struct{
    uint32_t tamanio_pagina;
    uint32_t cantidad_entradas_tabla;
    uint32_t cantidad_niveles;
    uint32_t ultima_direccion_fisica_calculada;
}mmu_t;

typedef struct{
    uint32_t nro_pagina;
    uint32_t marco_asociado;
}entradas_tlb_t;
typedef struct{
    entradas_tlb_t* entradas;
    uint32_t cantidad_entradas;
}tlb_t;

extern mmu_t* mmu;
extern tlb_t* tlb;
tlb_t* inicializar_tlb(uint32_t);
mmu_t* inicializar_mmu();
void recibir_datos_de_memoria(mmu_t*);


typedef struct {
    uint32_t  nro_pagina;
    uint32_t  desplazamiento;
    uint32_t  * entrada_nivel; // entradas de cada nivel
} t_pre_direccion_fisica;

t_pre_direccion_fisica calcular_pre_direccion_fisica(int direccion_logica);
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica);
extern tlb_t* tlb;
#endif // CPU_UTILS_H