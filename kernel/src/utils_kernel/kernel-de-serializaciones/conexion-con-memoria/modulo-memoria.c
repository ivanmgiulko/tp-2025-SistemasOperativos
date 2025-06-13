#include "modulo-memoria.h"

t_respuesta_dump* recibir_respuesta_dump(t_buffer* buffer)
{
	t_respuesta_dump* respuesta_dump = malloc(sizeof(t_respuesta_dump));

	void* stream = buffer->stream;

    memcpy(&(respuesta_dump->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
	memcpy(&(respuesta_dump->respuesta), stream, sizeof(bool)); stream += sizeof(bool);
	
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