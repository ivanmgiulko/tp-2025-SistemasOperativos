#include "modulo-cpu.h"

void enviar_proc_cpu(t_peticion_instruccion pcbInfo, int socket_cliente) { 
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(int) * 2;
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pcbInfo.pid, sizeof(int)); offset += sizeof(int);
    memcpy(buffer->stream + offset, &pcbInfo.pc, sizeof(int)); offset += sizeof(int);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = INFO_PROC_EXEC;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

t_pcb* proceso_syscall_prueba(t_buffer* buffer) { 
    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    int tamanioProceso;

    void* stream = buffer->stream;

    memcpy(&(tamanioProceso), stream, sizeof(int)); stream += sizeof(int);
    memcpy(&(path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    pathArchivoPseudocodigo = malloc(path_length);
    memcpy(pathArchivoPseudocodigo, stream, path_length);

    t_pcb* proceso_syscall_prueba = iniciarPCB(pathArchivoPseudocodigo, tamanioProceso, asignar_pid());

    return proceso_syscall_prueba;
}

t_syscall_io _deserializar_syscall_io(int* offset, t_paquete* paquete) 
{ 
    t_syscall_io _syscall_io_recibida;

    int len_dispositivo = 0;
    memcpy(&len_dispositivo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    char* dispositivo = malloc(len_dispositivo);
    memcpy(dispositivo, paquete->buffer->stream + *offset, len_dispositivo); *offset += len_dispositivo;
    _syscall_io_recibida.dispositivo = dispositivo;

    // Leer tiempo
    int tamanio_tiempo;
    memcpy(&tamanio_tiempo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    int tiempo;
    memcpy(&tiempo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    _syscall_io_recibida.tiempo = tiempo;

    return _syscall_io_recibida;
}

char* deserializar_archivo_instrucciones(int* offset, t_paquete* paquete) {
    int len_archivo=0;
    
    memcpy(&len_archivo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    char* archivo = malloc(len_archivo);
    memcpy(archivo, paquete->buffer->stream + *offset, len_archivo); *offset += len_archivo;

    return archivo;
}

int deserializar_tamanio_proceso(int* offset, t_paquete* paquete) {

    int tamanio_tamanio, tamanio;
    memcpy(&tamanio_tamanio, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&tamanio, paquete->buffer->stream + *offset, tamanio_tamanio); *offset += tamanio_tamanio;

    return tamanio;
}

void enviar_proceso_a_dumpear_en_memoria(int socket_cliente, t_pcb proceso)  {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size =  sizeof(uint8_t) + sizeof(uint16_t) + sizeof(metricas_estado) + 
                    sizeof(p_estados) + sizeof(uint32_t) * 2 + (proceso.path_length + 1);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &proceso.pid, sizeof(uint8_t));                       offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &proceso.pc, sizeof(uint16_t));                       offset += sizeof(uint16_t);
    memcpy(buffer->stream + offset, &proceso.metricas_estado,   sizeof(metricas_estado)); offset += sizeof(metricas_estado);
    memcpy(buffer->stream + offset, &proceso.estadoProceso,     sizeof(p_estados));       offset += sizeof(p_estados);
    memcpy(buffer->stream + offset, &proceso.tamanioMemoria,    sizeof(uint32_t));        offset += sizeof(uint32_t);

    memcpy(buffer->stream + offset, &proceso.path_length,       sizeof(uint32_t));        offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, proceso.pathArchivoPseudocodigo, proceso.path_length);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_DUMPEAR;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));   offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}