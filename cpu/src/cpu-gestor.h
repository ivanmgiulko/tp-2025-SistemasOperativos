#ifndef CPU_GESTOR_
#define CPU_GESTOR_
#include <./utils/utils.h>
#include <utils/cliente/client.h>
#include <utils/server/server.h>
#include <utils/proceso/process.h>
#include <utils/serializacion/serializacion.h>
#include <utils/contrato/contrato.h>
#include <cpu-utils.h>
#include <stdint.h>
#include <limits.h>

extern bool receptor_habilitado;
extern t_log* logger_cpu;
extern sem_t sem_cpu;
extern sem_t sem_cpu_kernel;
extern sem_t sem_memoria;
extern sem_t sem_read;
extern sem_t sem_write;
extern pthread_mutex_t mutex_cpu;
extern pthread_mutex_t mutex_conexion_memoria;
extern pthread_cond_t condicion_reactivacion_recepcion_memoria;

extern uint32_t entradas_cache;
extern uint32_t retardo_cache;
extern char* algoritmo_reemplazo_cache;
extern t_peticion_instruccion* pcb_actual;
extern int valor_sem_cpu;
extern int32_t ultimo_marco_obtenido;
extern char* ultima_escritura;
extern char* ultima_lectura;
extern bool flag_interrupt;
//conexiones
extern int fd_conexion_kernel_interrupt;
extern int fd_conexion_kernel_dispatch;
extern int fd_conexion_memoria;
#endif // CPU_GESTOR_