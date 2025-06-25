#include "process.h"

t_pcb* iniciarPCB(char* path, int tamanio, int pid, uint64_t estimacion_inicial) 
{
    t_pcb* nuevoPCB = malloc(sizeof(t_pcb));
    nuevoPCB->pathArchivoPseudocodigo = path;
    nuevoPCB->tamanioMemoria = tamanio;
    nuevoPCB->path_length = strlen(path);
    nuevoPCB->pid = pid;
    nuevoPCB->pc = 0;
    nuevoPCB->estadoProceso = NEW;

    nuevoPCB->estimacion_aux = 0;
    nuevoPCB->estimacion_rafaga_anterior = 0; // Estn(n)
    nuevoPCB->estimacion_actual = estimacion_inicial; // Est(n + 1)
    nuevoPCB->tiempo_rafaga = 0; // R(n)

    nuevoPCB->metricas_estado = iniciarMetricasEstado();
    nuevoPCB->metricas_tiempo = iniciarMetricasTiempo();

    t_datos_io* datos_io = malloc(sizeof(t_datos_io));
    datos_io->dispositivo = "";
    datos_io->tiempo = 0;
    datos_io->instancia_utilizada = NULL;
    nuevoPCB->datos_io = datos_io;

    return nuevoPCB;
}

metricas_estado* iniciarMetricasEstado()
{
    metricas_estado* estados = malloc(sizeof(metricas_estado));
    
    estados->cantVecesBlocked       = 0;
    estados->cantVecesExec          = 0;
    estados->cantVecesExit          = 0;
    estados->cantVecesNew           = 0;
    estados->cantVecesReady         = 0;
    estados->cantVecesSuspBlocked   = 0;
    estados->cantVecesSuspReady     = 0;

    return estados;
}

metricas_tiempo* iniciarMetricasTiempo()
{
    metricas_tiempo* tiempos = malloc(sizeof(metricas_tiempo));

    tiempos->tiempoEnBlocked = temporal_create();
    temporal_stop(tiempos->tiempoEnBlocked);
    tiempos->tiempoEnBlocked->elapsed_ms = 0;

    tiempos->tiempoEnExec = temporal_create();
    temporal_stop(tiempos->tiempoEnExec);
    tiempos->tiempoEnExec->elapsed_ms = 0;

    tiempos->tiempoEnExit = temporal_create();
    temporal_stop(tiempos->tiempoEnExit);
    tiempos->tiempoEnExit->elapsed_ms = 0;

    tiempos->tiempoEnNew = temporal_create();
    temporal_stop(tiempos->tiempoEnNew);
    tiempos->tiempoEnNew->elapsed_ms = 0;

    tiempos->tiempoEnReady = temporal_create();
    temporal_stop(tiempos->tiempoEnReady);
    tiempos->tiempoEnReady->elapsed_ms = 0;

    tiempos->tiempoEnSuspBlocked = temporal_create();
    temporal_stop(tiempos->tiempoEnSuspBlocked);
    tiempos->tiempoEnSuspBlocked->elapsed_ms = 0;

    tiempos->tiempoEnSuspReady = temporal_create();
    temporal_stop(tiempos->tiempoEnSuspReady);
    tiempos->tiempoEnSuspReady->elapsed_ms = 0;

    return tiempos;
}

metricas_proceso iniciarMetricasProceso(){
    metricas_proceso metricas;
    metricas.cantVecesTP            = 0;
    metricas.cantVecesInstrucciones = 0;
    metricas.cantVecesSWAP          = 0;
    metricas.cantVecesMP            = 0;
    metricas.cantVecesRead          = 0;
    metricas.cantVecesWrite         = 0;
    return metricas;
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

void enviar_proceso_a_finalizar_kernel(t_pcb proceso, int socket_cliente) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(uint32_t) * 2 + (proceso.path_length);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &proceso.pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &proceso.tamanioMemoria, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, &proceso.path_length, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, proceso.pathArchivoPseudocodigo, proceso.path_length);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_FINALIZADO;
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
    memcpy(&(procesoMemo->path_length), stream, sizeof(uint32_t));  stream += sizeof(uint32_t);
    procesoMemo->pathArchivoPseudocodigo = malloc(procesoMemo->path_length);
    memcpy(procesoMemo->pathArchivoPseudocodigo, stream, procesoMemo->path_length);

    return procesoMemo;
}

int _deserializar_pid(int* offset, t_paquete* paquete) 
{ 
    int pid = 0, tamanio_pid = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pid, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&pid, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    return pid;
}

int _deserializar_pc(int* offset, t_paquete* paquete) 
{ 
    int pc = 0, tamanio_pc = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pc, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&pc, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    return pc;
}