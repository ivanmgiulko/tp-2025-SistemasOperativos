#include "serializacion.h"

// Funciones utilizadas desde el lado del cliente (para serializar)

// Funciones del buffer:

void crear_buffer(t_paquete* paquete){
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

// Funciones de Paquete

t_paquete* crear_paquete(void){
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

t_paquete* crear_paquete_instruccion(void){
	t_paquete* paquete = crear_paquete_con_codigo(INSTRUCCION);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor_a_agregar, uint32_t tamanio_del_valor)
{
	 // Se realoca (redimensiona) el buffer del paquete para poder agregar:
    // - el tamaño del nuevo dato (uint32_t)
    // - el dato en sí (tamanio_del_valor bytes)
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio_del_valor + sizeof(uint32_t));
	// Si la realocación falla, se retorna sin hacer nada (para evitar fallos).
	if (paquete->buffer->stream == NULL){
		return;
	}
	//agrega el tamaño del nuevo valor al final del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_del_valor, sizeof(uint32_t));
	//agrega el nuevo valor al final del buffer
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), valor_a_agregar, tamanio_del_valor);

	paquete->buffer->size += tamanio_del_valor + sizeof(uint32_t);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	if (magic == NULL) {
        // Manejar error de memoria
        return NULL;
    }

	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(uint32_t)); 		desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t)); 			desplazamiento+= sizeof(uint32_t);
	if (paquete->buffer->size > 0 && paquete->buffer->stream != NULL) {
        memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    }
	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + sizeof(int) + sizeof(uint32_t); //tam_buffer + tam_tam_buffer(32) + tam_opcode(32)
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

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

// Funciones utilizadas desde el lado del servidor (para deserializar)
int32_t recibir_cod_operacion(int socket_cliente)
{
	int32_t cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int32_t), MSG_WAITALL) > 0)
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
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
}

void recibir_pid(int socket_cliente, t_paquete* paquete)
{
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint8_t), MSG_WAITALL);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
}

//funcion temporal, queda pendiente unificarla con la de arriba (recibir_paqute)
void recibir_buffer_en_paquete(int socket_cliente, t_paquete* paquete)
{
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
}

char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento) {
    int tamanio;
    memcpy(&tamanio, buffer->stream + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    char* string = malloc(tamanio);
    memcpy(string, buffer->stream + *desplazamiento, tamanio);
    *desplazamiento += tamanio;

    return string;
}

uint8_t leer_uint8_desde_buffer(t_buffer* buffer, int* desplazamiento){
    int tamanio;
    memcpy(&tamanio, buffer->stream + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    uint8_t valor;
    memcpy(&valor, buffer->stream + *desplazamiento, sizeof(uint8_t));
    *desplazamiento += sizeof(uint8_t);

    return valor;
}

uint32_t leer_uint32_desde_buffer(t_buffer* buffer, int* desplazamiento){
    int tamanio;
    memcpy(&tamanio, buffer->stream + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    uint32_t valor;
    memcpy(&valor, buffer->stream + *desplazamiento, sizeof(uint32_t));
    *desplazamiento += sizeof(uint32_t);

    return valor;
}

void loggear_mensaje_desde_buffer(t_buffer* buffer, t_log* logger){
	int desplazamiento = 0;
	char* mensaje = leer_string_desde_buffer(buffer, &desplazamiento);
	log_debug(logger, "Me llego el mensaje :<%s/>", mensaje);
	free(mensaje);
}

//ME PARECE MEDIO AL PEDO IVAN :C
// t_paquete* recibir_paquete_sin_codigo(int socket) {
//     t_paquete* paquete = malloc(sizeof(t_paquete));
//     crear_buffer(paquete);

//     // Leer tamaño del buffer
//     if (recv(socket, &(paquete->buffer->size), sizeof(int), MSG_WAITALL) <= 0) {
//         free(paquete->buffer);
//         free(paquete);
//         return NULL; // Error al recibir
//     }
    
//     // Leer contenido del buffer
//     paquete->buffer->stream = malloc(paquete->buffer->size);
//     if (recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL) <= 0) {
//         free(paquete->buffer->stream);
//         free(paquete->buffer);
//         free(paquete);
//         return NULL; // Error al recibir
//     }

//     return paquete;
// }