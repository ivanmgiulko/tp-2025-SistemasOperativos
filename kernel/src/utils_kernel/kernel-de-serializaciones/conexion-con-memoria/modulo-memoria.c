#include "modulo-memoria.h"
t_respuesta_dump* recibir_respuesta_dump(t_buffer* buffer) {
    t_respuesta_dump* respuesta_dump = malloc(sizeof(t_respuesta_dump));
    int desplazamiento = 0;
    uint32_t tamanio;

    // Leer tamaño del PID
    memcpy(&tamanio, buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer PID
    memcpy(&(respuesta_dump->pid), buffer->stream + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    // Leer tamaño de la respuesta
    memcpy(&tamanio, buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer respuesta
    memcpy(&(respuesta_dump->respuesta), buffer->stream + desplazamiento, sizeof(bool));
    desplazamiento += sizeof(bool);

    printf("La respuesta del DUMP es: %d\n", respuesta_dump->respuesta);
    return respuesta_dump;
}

void enviar_tamanio_proceso(char* tam_proceso, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = TAMANIO_PROCESO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(tam_proceso) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, tam_proceso, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_a_liberar_memoria(int socket_memoria, t_pcb proceso) 
{
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(uint32_t) * 2 + (proceso.path_length);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &proceso.pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &proceso.tamanioMemoria, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, &proceso.path_length, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, proceso.pathArchivoPseudocodigo, proceso.path_length);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_SUSPENDIDO_MEMORIA;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_memoria, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}