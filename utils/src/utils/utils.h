#ifndef UTILS_H_
#define UTILS_H_

#include<stdlib.h>
#include<stdio.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<readline/history.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<assert.h>
#include<pthread.h>
#include<semaphore.h>


typedef enum
{
	MENSAJE,
	PAQUETE,

	INSTRUCCION,
	RESPUESTA_MEMORIA, 
	PROCESO_MEMORIA,
	
	
	TAMANIO_PROCESO, 	// Kernel -> tamanio de proceso para inicializarlo
	INTERFAZ
} op_code;


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

void saludar(char* quien);
t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void log_obligatorio(t_log* log, int pid, char* texto);

#endif
