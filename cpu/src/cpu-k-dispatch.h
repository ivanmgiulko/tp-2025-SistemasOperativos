#ifndef CPU_K_DISPTACH_H_
#define CPU_K_DISPTACH_H_
#include "cpu-gestor.h"
// Prueba de mandar IO a Kernel

int manejar_conexion_kernel_dispatch();
t_peticion_instruccion* deserializar_info_pcb(t_buffer* buffer);

#endif // CPU_K_DISPTACH_H_