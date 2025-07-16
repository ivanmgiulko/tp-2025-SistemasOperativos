#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <./utils/utils.h>

extern t_log* logger_servidor; 

// Funciones de serializacion junto su respectiva deserializacion
void enviar_nombreInterfaz(char* mensaje, int socket_cliente);
char* recibir_tamProceso(int socket_cliente);

void enviar_mensaje(char*, int);
void recibir_mensaje(int, t_log* );

// Funciones basicas de serializacion
void crear_buffer(t_paquete*);
t_paquete* crear_paquete(void);
t_paquete* crear_paquete_instruccion(void);
t_paquete* crear_paquete_con_codigo(op_code codigo);

void agregar_a_paquete(t_paquete*, void*, uint32_t);
void enviar_paquete(t_paquete*, int);

void recibir_paquete(int , t_paquete* );
void recibir_pid(int , t_paquete* );
//t_paquete* recibir_paquete_sin_codigo(int);

void liberar_conexion(int);

void eliminar_paquete(t_paquete*);
void* serializar_paquete(t_paquete*, int);

// Funciones que complementan a la deserializacion
void* recibir_buffer(int*, int);

void recibir_buffer_en_paquete(int socket_cliente, t_paquete* paquete);
int32_t recibir_cod_operacion(int);

char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento);
uint32_t leer_uint32_desde_buffer(t_buffer* buffer, int* desplazamiento);
uint8_t leer_uint8_desde_buffer(t_buffer* buffer, int* desplazamiento);

void loggear_mensaje_desde_buffer(t_buffer* buffer, t_log* logger);

#endif // SERIALIZACION_H_