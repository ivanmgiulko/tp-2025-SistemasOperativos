#ifndef CPU_H_
#define CPU_H_
#include "./cpu-gestor.h"
#include "./cpu-k-interrupt.h"
#include "./cpu-utils.h"
#include "./cpu-k-dispatch.h"
#include "./cpu-memoria.h"
// En el header definimos structs, enums, firma de funciones, y creo que nada mas

t_log* logger_cpu;

int fd_conexion_kernel_interrupt;
int fd_conexion_kernel_dispatch;
int fd_conexion_memoria;

#endif // CPU_H_