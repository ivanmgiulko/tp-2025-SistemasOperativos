#ifndef SERVER_H_
#define SERVER_H_

#include "./utils/utils.h"

int iniciar_servidor(char*);
int esperar_cliente(int);
void manejar_hilos(int);
int manejar_conexion(int);
void iterator(char*);

#endif // SERVER_H_