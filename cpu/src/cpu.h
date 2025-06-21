#ifndef CPU_H_
#define CPU_H_
#include "./cpu-gestor.h"
#include "./cpu-k-interrupt.h"
#include "./cpu-utils.h"
#include "./cpu-k-dispatch.h"
#include "./cpu-memoria.h"
#include "./instrucciones.h"

t_log* logger_cpu;
pthread_mutex_t mutex_cpu;
sem_t sem_cpu;
sem_t sem_read;
sem_t sem_write;
 mmu_t* mmu;
sem_t sem_cpu_kernel;
t_peticion_instruccion* pcb_actual;
bool flag_interrupt = false;
int fd_conexion_kernel_interrupt;
int fd_conexion_kernel_dispatch;
int fd_conexion_memoria;
int valor_sem_cpu;
char* ultima_escritura = NULL;
#endif // CPU_H_