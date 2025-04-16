#include "process.h"

process iniciarProceso(int pid) 
{
    process nuevoProceso;
    nuevoProceso.pid = pid;
    nuevoProceso.pc = 0;
    nuevoProceso.estadoProceso = NEW;
    nuevoProceso.metricas_estado = iniciarMetricasEstado();
    nuevoProceso.metricas_tiempo = iniciarMetricasTiempo();
    return nuevoProceso;
}

metricas_estado iniciarMetricasEstado()
{
    metricas_estado estados;
    estados.cantVecesBlocked = 0;
    estados.cantVecesExec = 0;
    estados.cantVecesExit = 0;
    estados.cantVecesNew = 0;
    estados.cantVecesReady = 0;
    estados.cantVecesSuspBlocked = 0;
    estados.cantVecesSuspReady = 0;
    return estados;
}

metricas_tiempo iniciarMetricasTiempo()
{
    metricas_tiempo tiempos;
    tiempos.tiempoEnBlocked = 0;
    tiempos.tiempoEnExec = 0;
    tiempos.tiempoEnExit = 0;
    tiempos.tiempoEnNew = 0;
    tiempos.tiempoEnReady = 0;
    tiempos.tiempoEnSuspBlocked = 0;
    tiempos.tiempoEnSuspReady = 0;
    return tiempos;
}