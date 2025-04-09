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

extern t_log* logger_kernel;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);


#endif // KERNEL_HEADER_H_H