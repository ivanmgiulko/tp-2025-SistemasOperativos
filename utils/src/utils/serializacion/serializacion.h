#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <./utils/utils.h>

extern t_log* logger_servidor; 
// Funciones de serializacion junto su respectiva deserializacion
void enviar_nombreInterfaz(char* mensaje, int socket_cliente);
char* recibir_tamProceso(int socket_cliente);

void enviar_mensaje(char*, int);
void recibir_mensaje(int, t_log* );

// Funciones que complementan a la serializacion
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete*, void*, int);
void enviar_paquete(t_paquete*, int);
void liberar_conexion(int);
void eliminar_paquete(t_paquete*);
void* serializar_paquete(t_paquete*, int);

// Funciones que complementan a la deserializacion
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
int recibir_operacion(int);

#endif // SERIALIZACION_H_