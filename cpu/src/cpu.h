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
sem_t sem_respuesta_memo;
sem_t sem_cpu;
sem_t sem_read;
sem_t sem_write;
sem_t sem_rta_marco;
sem_t sem_instruccion;
t_memoria_cache* memoria_cache;
mmu_t* mmu;
tlb_t* tlb;
algoritmo_tlb_t algoritmo;
//sem_t sem_cpu_kernel;
t_peticion_instruccion* pcb_actual;
bool flag_interrupt = false;
bool flag_exit = false;
int fd_conexion_kernel_interrupt;
int fd_conexion_kernel_dispatch;
int fd_conexion_memoria;
int valor_sem_cpu;
uint32_t  proxima_a_reemplazar= 0;
uint32_t contador_accesos_tlb = 0;
int32_t ultimo_marco_obtenido = 0;
int32_t marco_global;
char* ultima_escritura = NULL;
char* ultima_lectura = NULL;
char* respuesta_memo;
char* respuesta_instruccion;
uint32_t entradas_cache;
uint32_t retardo_cache;
char* algoritmo_reemplazo_cache;
#endif // CPU_H_