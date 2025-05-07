#ifndef CPU_H_
#define CPU_H_
#include "./cpu-gestor.h"
#include "./cpu-k-interrupt.h"
#include "./cpu-utils.h"
#include "./cpu-k-dispatch.h"
#include "./cpu-memoria.h"
#include "./instrucciones.h"
// En el header definimos structs, enums, firma de funciones, y creo que nada mas

t_log* logger_cpu;

sem_t sem_cpu;

int fd_conexion_kernel_interrupt;
int fd_conexion_kernel_dispatch;
int fd_conexion_memoria;



#endif // CPU_H_