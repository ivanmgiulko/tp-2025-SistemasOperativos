#ifndef CPU_H_
#define CPU_H_
#include "./cpu-gestor.h"
#include "./cpu-k-interrupt.h"
#include "./cpu-utils.h"
#include "./cpu-k-dispatch.h"
#include "./cpu-memoria.h"
#include "./instrucciones.h"
bool receptor_habilitado;
t_log* logger_cpu;
pthread_mutex_t mutex_cpu;
pthread_mutex_t mutex_conexion_memoria;
pthread_cond_t condicion_reactivacion_recepcion_memoria;
sem_t sem_cpu;
sem_t sem_memoria;
sem_t sem_read;
sem_t sem_write;
t_memoria_cache* memoria_cache;
mmu_t* mmu;
tlb_t* tlb;
algoritmo_tlb_t algoritmo;
sem_t sem_cpu_kernel;
t_peticion_instruccion* pcb_actual;
bool flag_interrupt = false;
int fd_conexion_kernel_interrupt;
int fd_conexion_kernel_dispatch;
int fd_conexion_memoria;
int valor_sem_cpu;
uint32_t  proxima_a_reemplazar= 0;
uint32_t contador_accesos_tlb = 0;
int32_t ultimo_marco_obtenido = 0;
char* ultima_escritura = NULL;
char* ultima_lectura = NULL;

uint32_t entradas_cache;
uint32_t retardo_cache;
char* algoritmo_reemplazo_cache;
#endif // CPU_H_