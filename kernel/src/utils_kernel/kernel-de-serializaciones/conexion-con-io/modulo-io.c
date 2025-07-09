#include "modulo-io.h"

void enviar_proceso_a_io_para_bloqueo(uint8_t pid, int64_t tiempo, int socket_cliente, int cod_op) { 
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(int64_t);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &tiempo, sizeof(int64_t)); offset += sizeof(int64_t);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = cod_op;
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

uint8_t _recibir_proceso_bloqueado(t_buffer* buffer) { 
	uint8_t pid;
    
    memcpy(&pid, buffer->stream, sizeof(uint8_t)); 

    return pid;
    
    
}