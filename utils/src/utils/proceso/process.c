#include "process.h"


t_pcb* iniciarPCB(char* path, int tamanio, int pid) 
{
    t_pcb* nuevoPCB = malloc(sizeof(t_pcb));
    nuevoPCB->pathArchivoPseudocodigo = path;
    nuevoPCB->tamanioMemoria = tamanio;
    nuevoPCB->path_length = strlen(path);
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
    t_contador* contador = malloc(sizeof(t_contador));

    contador->valor = 0;
    pthread_mutex_init(&(contador->mutex), NULL);
    return contador;
}

void incrementar_contador(t_contador* contador){
    pthread_mutex_lock(&contador->mutex);
    contador->valor++;
    pthread_mutex_unlock(&contador->mutex);

}
void enviarProceso_A_Memoria(t_pcb proceso, int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(uint32_t) * 2 + (proceso.path_length);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &proceso.pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &proceso.tamanioMemoria, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, &proceso.path_length, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, proceso.pathArchivoPseudocodigo, proceso.path_length);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_MEMORIA;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


void enviar_proceso_a_finalizar_Memoria(t_pcb proceso, int socket_cliente) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(uint32_t) * 2 + (proceso.path_length);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &proceso.pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &proceso.tamanioMemoria, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, &proceso.path_length, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, proceso.pathArchivoPseudocodigo, proceso.path_length);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_FINALIZAR;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_pcbMemoria* deserializarProceso(t_buffer* buffer) { 
    t_pcbMemoria* procesoMemo = malloc(sizeof(t_pcbMemoria));
    void* stream = buffer->stream;

    memcpy(&(procesoMemo->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
    memcpy(&(procesoMemo->tamanioMemoria), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(procesoMemo->path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    procesoMemo->pathArchivoPseudocodigo = malloc(procesoMemo->path_length);
    memcpy(procesoMemo->pathArchivoPseudocodigo, stream, procesoMemo->path_length);

    return procesoMemo;
}

