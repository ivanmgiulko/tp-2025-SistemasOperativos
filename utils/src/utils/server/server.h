#ifndef SERVER_H_
#define SERVER_H_

#include "./utils/utils.h"

void* recibir_buffer(int*, int);
void iterator(char*);
int iniciar_servidor(char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
int manejar_conexion(int);
void manejar_hilos(int);

#endif // SERVER_H_