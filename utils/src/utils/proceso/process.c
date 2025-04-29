#include "process.h"

t_proceso* iniciarProceso(char* path, int tamanio, int pid){
    t_proceso* nuevoProceso = malloc(sizeof(t_proceso));
    nuevoProceso->pathArchivoPseudocodigo = path;
    nuevoProceso->tamanioMemoria = tamanio;
    nuevoProceso->pcb = iniciarPCB(pid);
    return nuevoProceso;
}

t_pcb* iniciarPCB(int pid) 
{
    t_pcb* nuevoPCB = malloc(sizeof(t_pcb));
    nuevoPCB->pid = pid;
    nuevoPCB->pc = 0;
    nuevoPCB->estadoProceso = NEW;
    nuevoPCB->metricas_estado = iniciarMetricasEstado();
    nuevoPCB->metricas_tiempo = iniciarMetricasTiempo();
    return nuevoPCB;
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

t_contador* inicializar_contador(){
    t_contador* contador = malloc(sizeof(contador));

    contador->valor = 0;
    pthread_mutex_init(&(contador->mutex), NULL);
    return contador;
}

void incrementar_contador(t_contador* contador){
    pthread_mutex_lock(&contador->mutex);
    contador->valor++;
    pthread_mutex_unlock(&contador->mutex);

}
