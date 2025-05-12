#ifndef CPU_GESTOR_
#define CPU_GESTOR_
#include <./utils/utils.h>
#include <utils/cliente/client.h>
#include <utils/server/server.h>
#include <utils/proceso/process.h>
#include <utils/serializacion/serializacion.h>
#include <utils/contrato/contrato.h>
#include <cpu-utils.h>

extern t_log* logger_cpu;

extern sem_t sem_cpu;

extern t_peticion_instruccion* pcb_actual;

//conexiones
extern int fd_conexion_kernel_interrupt;
extern int fd_conexion_kernel_dispatch;
extern int fd_conexion_memoria;
#endif // CPU_GESTOR_