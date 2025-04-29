#ifndef PROCESS_H_
#define PROCESS_H_

#include <./utils/utils.h>

typedef enum {
    NEW, 
    READY, 
    EXEC, 
    BLOCKED,
    EXIT,
    SUSP_READY,
    SUSP_BLOCEKD
} p_estados;

typedef struct metricas_estado
{
    int cantVecesNew;
    int cantVecesReady;
    int cantVecesExec;
    int cantVecesBlocked;
    int cantVecesSuspReady;
    int cantVecesSuspBlocked;
    int cantVecesExit;
} metricas_estado;

typedef struct metricas_tiempo
{
    float tiempoEnNew;
    float tiempoEnReady;
    float tiempoEnExec;
    float tiempoEnBlocked;
    float tiempoEnSuspReady;
    float tiempoEnSuspBlocked;
    float tiempoEnExit;
} metricas_tiempo;

typedef struct 
{
    int pid;
    int pc;
    metricas_estado metricas_estado;
    metricas_tiempo metricas_tiempo;
    p_estados estadoProceso;

} t_pcb; 

typedef struct 
{
    char* pathArchivoPseudocodigo;
    int tamanioMemoria;
    t_pcb* pcb;

} t_proceso; 

typedef struct {
    pthread_mutex_t mutex;
    int valor;
} t_contador;


metricas_estado iniciarMetricasEstado();
metricas_tiempo iniciarMetricasTiempo();
t_pcb* iniciarPCB(int pid);
t_proceso* iniciarProceso(char* path, int tamanio, int pid);

#endif // PROCESS_H_
