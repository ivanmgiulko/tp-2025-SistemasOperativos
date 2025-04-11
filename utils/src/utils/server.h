#ifndef SERVER_H_
#define SERVER_H_

#include "utils.h"

void* recibir_buffer(int*, int);
void iterator(char* value, t_log* logger_servidor);
int iniciar_servidor(char*, t_log* );
int esperar_cliente(int,  t_log* );
t_list* recibir_paquete(int);
void recibir_mensaje(int, t_log*);
int recibir_operacion(int);
int manejar_conexion(int socket_server, int socket_cliente, t_log* logger_servidor);
void manejar_hilos(int server_fd, t_log* logger_servidor);

#endif // SERVER_H_