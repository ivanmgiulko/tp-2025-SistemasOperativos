#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "./cpu-gestor.h"
#include "./instrucciones.h"

typedef struct{
    uint32_t tamanio_pagina;
    uint32_t cantidad_entradas_tabla;
    uint32_t cantidad_niveles;
}mmu_t;

extern mmu_t* mmu;

void recibir_datos_de_memoria(mmu_t*);

void pedir_instruccion_a_memoria(t_peticion_instruccion*);

mmu_t* inicializar_mmu();

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

#endif // CPU_UTILS_H