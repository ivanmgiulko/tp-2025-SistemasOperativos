#ifndef MEMORIA_HEADER_H_
#define MEMORIA_HEADER_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include<pthread.h>


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger_memoria;

void* recibir_buffer(int*, int);

int iniciar_servidor(char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void manejar_hilos(int);
int manejar_conexion(int);
#endif // MEMORIA_HEADER_H_H