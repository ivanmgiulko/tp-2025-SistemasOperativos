#include "cambio-estado-proceso.h"
#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>

void pasar_pcb_a_new(t_pcb* pcb) 
{
    encolar_pcb_en_estado(estado_new, pcb);

    pcb->metricas_estado->cantVecesNew++;
    temporal_resume(pcb->metricas_tiempo->tiempoEnNew);

    pcb->estadoProceso = NEW;
    log_info(logger_kernel, "## %d Pasa al estado NEW", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_new_a_ready(t_pcb* pcb)
{ 
    encolar_pcb_en_estado(estado_ready, pcb);

    pcb->metricas_estado->cantVecesReady++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnNew);
    temporal_resume(pcb->metricas_tiempo->tiempoEnReady);

    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado NEW al estado READY", pcb->pid);

    sem_post(&sem_cantidad_pcbs_en_ready); 
    sem_post(&bin_replanificar_srt);
}

void pasar_pcb_susp_ready_a_ready(t_pcb* pcb) { 
    encolar_pcb_en_estado(estado_ready, pcb);

    pcb->metricas_estado->cantVecesReady++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnSuspReady);
    temporal_resume(pcb->metricas_tiempo->tiempoEnReady);

    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado SUSP-READY al estado READY", pcb->pid);

    pcb->estimacion_actual = calcular_estimacion_actual(pcb->tiempo_rafaga, pcb->estimacion_rafaga_anterior);

    sem_post(&sem_cantidad_pcbs_en_ready); 
    sem_post(&bin_replanificar_srt);
}

void pasar_pcb_blocked_a_ready(t_pcb* pcb) 
{ 
    encolar_pcb_en_estado(estado_ready, pcb);

    pcb->metricas_estado->cantVecesReady++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnBlocked);
    temporal_resume(pcb->metricas_tiempo->tiempoEnReady);

    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado BLOCKED al estado READY", pcb->pid);

    pcb->estimacion_rafaga_anterior = pcb->estimacion_actual;
    pcb->estimacion_actual = calcular_estimacion_actual(pcb->tiempo_rafaga, pcb->estimacion_rafaga_anterior);

    sem_post(&sem_cantidad_pcbs_en_ready); 
    sem_post(&bin_replanificar_srt);
}

void pasar_pcb_exec_a_ready(t_pcb* pcb) 
{ 
    encolar_pcb_en_estado(estado_ready, pcb);

    pcb->metricas_estado->cantVecesReady++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnExec);
    temporal_resume(pcb->metricas_tiempo->tiempoEnReady);

    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado EXEC al estado READY", pcb->pid);

    pcb->estimacion_rafaga_anterior = pcb->estimacion_actual;
    pcb->estimacion_actual = calcular_estimacion_actual(pcb->tiempo_rafaga, pcb->estimacion_rafaga_anterior);

    sem_post(&sem_cantidad_pcbs_en_ready); 
    sem_post(&bin_replanificar_srt);
}

uint64_t calcular_estimacion_actual(int64_t rafagas_hechas, uint64_t estimacion_anterior) {
    float alfa = atof(configuracion_kernel->ALFA);
    uint64_t estimacion = alfa * rafagas_hechas + (1 - alfa) * estimacion_anterior;
    return estimacion;
}

void pasar_pcb_blocked_a_suspblocked(t_pcb* pcb) 
{
    encolar_pcb_en_estado(estado_susp_blocked, pcb);

    pcb->metricas_estado->cantVecesSuspBlocked++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnBlocked);
    temporal_resume(pcb->metricas_tiempo->tiempoEnSuspBlocked);

    pcb->estadoProceso = SUSP_BLOCEKD;
    log_info(logger_kernel, "## %d Pasa del estado BLOCKED al estado SUSP-BLOCKED", pcb->pid);
}

void pasar_pcb_suspblocked_a_suspready(t_pcb* pcb) 
{
    encolar_pcb_en_estado(estado_susp_ready, pcb);

    pcb->metricas_estado->cantVecesSuspReady++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnSuspBlocked);
    temporal_resume(pcb->metricas_tiempo->tiempoEnSuspReady);

    pcb->estadoProceso = SUSP_READY;
    log_info(logger_kernel, "## %d Pasa del estado SUSP-BLOCKED al estado SUSP-READY", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_blocked_a_exit(t_pcb* pcb) 
{ 
    encolar_pcb_en_estado(estado_exit, pcb);

    pcb->metricas_estado->cantVecesExit++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnBlocked);
    temporal_resume(pcb->metricas_tiempo->tiempoEnExit);

    pcb->estadoProceso = EXIT;
    log_info(logger_kernel, "## %d Pasa del estado BLOCKED al estado EXIT", pcb->pid);
    _enviar_a_finalizar_proceso(pcb);
    
}

void pasar_pcb_ready_a_exec(t_pcb* pcb) 
{ 
    encolar_pcb_en_estado(estado_exec, pcb);

    pcb->metricas_estado->cantVecesExec++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnReady);
    temporal_resume(pcb->metricas_tiempo->tiempoEnExec);

    pcb->estadoProceso = EXEC;
    log_info(logger_kernel, "## %d Pasa del estado READY al estado EXEC", pcb->pid);
}

void pasar_de_exec_a_blocked(t_pcb* pcb)
{
    encolar_pcb_en_estado(estado_blocked, pcb);

    pcb->metricas_estado->cantVecesBlocked++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnExec);
    temporal_resume(pcb->metricas_tiempo->tiempoEnBlocked);

    pcb->estadoProceso = BLOCKED;
    log_info(logger_kernel, "## %d Pasa del estado EXEC al estado BLOCKED", pcb->pid);
}

void pasar_de_exec_a_exit(t_pcb* pcb)
{
    encolar_pcb_en_estado(estado_exit, pcb);

    pcb->metricas_estado->cantVecesExit++;
    temporal_stop(pcb->metricas_tiempo->tiempoEnExec);
    temporal_resume(pcb->metricas_tiempo->tiempoEnExit);

    pcb->estadoProceso = EXIT;
    log_info(logger_kernel, "## %d Pasa del estado EXEC al estado EXIT", pcb->pid);
    _enviar_a_finalizar_proceso(pcb);
}

t_pcb* _sacar_pcb_de_cola(int pid, t_estado* estado) 
{ 
    pthread_mutex_lock(&(estado->mutex));
    t_pcb* proceso_a_devolver = NULL;
    for (int i = 0; i < list_size(estado->cola); i++) {
        t_pcb* pcb = list_get(estado->cola, i);
        if (pcb->pid == pid) {
            proceso_a_devolver = list_remove(estado->cola, i); // Eliminar el elemento
            break; // Salir del bucle una vez encontrado
        }
    }

    pthread_mutex_unlock(&(estado->mutex));
    return proceso_a_devolver;
}

void _enviar_a_finalizar_proceso(t_pcb* proceso_a_finalizar)
{ 
    sem_wait(&bin_proceso_eliminar);
    char* ip_memoria = configuracion_kernel->IP_MEMORIA;
    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    // Falta realizar prueba
    enviar_proceso_a_finalizar_Memoria(*proceso_a_finalizar, fd_conexion_memoria);

    manejar_conexion_kernel_memoria(fd_conexion_memoria);
}