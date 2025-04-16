#ifndef PROCESS_H_
#define PROCESS_H_

#include "utils.h"

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
    metricas_estado metricasEstado;
    metricas_tiempo metricasTiempo;
    p_estados estadoProceso;

} process; 

metricas_estado iniciarMetricasEstado();
metricas_tiempo iniciarMetricasTiempo();
process iniciarProceso(int);

#endif // PROCESS_H_
