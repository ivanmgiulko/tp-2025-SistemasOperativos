#include "serializacion.h"

// Funciones utilizadas desde el lado del cliente (para serializar)
void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}
// Funcion temporal - Ver de normalizarla junto a crear_paquete
t_paquete* crear_paquete_con_codigo(op_code codigo) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo;
    crear_buffer(paquete);
    return paquete;
}
void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
t_paquete* crear_paquete_instruccion(void){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = INSTRUCCION;
	crear_buffer(paquete);
	return paquete;
}
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	if (paquete->buffer->stream == NULL){
		return;
	}
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	if (magic == NULL) {
        // Manejar error de memoria
        return NULL;
    }

	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int)); 		desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int)); 			desplazamiento+= sizeof(int);
	if (paquete->buffer->size > 0 && paquete->buffer->stream != NULL) {
        memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    }
	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

// Funciones utilizadas desde el lado del servidor (para deserializar)
int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_trace(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

void recibir_paquete(int socket_cliente, t_paquete* paquete)
{
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
}

t_paquete* recibir_paquete_instruccion(int socket) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete);
    // Leer código de operación
    // if (recv(socket, &(paquete->codigo_operacion), sizeof(int), MSG_WAITALL) <= 0) {
    //     free(paquete);
    //     return NULL; // Error al recibir
    // }
    
    // Leer tamaño del buffer
    if (recv(socket, &(paquete->buffer->size), sizeof(int), MSG_WAITALL) <= 0) {
        free(paquete->buffer);
        free(paquete);
        return NULL; // Error al recibir
    }
    
    // Leer contenido del buffer
    paquete->buffer->stream = malloc(paquete->buffer->size);
    if (recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL) <= 0) {
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return NULL; // Error al recibir
    }

    return paquete;
}