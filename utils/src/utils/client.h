#ifndef CLIENT_H_
#define CLIENT_H_

#include "utils.h"

int crear_conexion(char*, char*);
void enviar_mensaje(char*, int);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete*, void*, int);
void enviar_paquete(t_paquete*, int);
void liberar_conexion(int);
void eliminar_paquete(t_paquete*);
void* serializar_paquete(t_paquete*, int);

#endif // CLIENT_H_