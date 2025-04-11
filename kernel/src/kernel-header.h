#ifndef KERNEL_HEADER_H_
#define KERNEL_HEADER_H_
// En el header definimos structs, enums, firma de funciones, y creo que nada mas

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

#define PUERTO_KERNEL "40073"
typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

// STRUCTS DE CLIENTE
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

extern t_log* logger_kernel;

void* recibir_buffer(int*, int);
void iterator(char* value);
int iniciar_servidor(char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

// PARA COMUNICARSE CON MEMORIA 

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
int manejar_conexion(int, int);
#endif // KERNEL_HEADER_H_H