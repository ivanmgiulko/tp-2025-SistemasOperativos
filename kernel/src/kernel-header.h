#ifndef KERNEL_HEADER_H_
#define KERNEL_HEADER_H_
#include <utils/server/server.h>
// En el header definimos structs, enums, firma de funciones, y creo que nada mas

extern t_log* logger_servidor;

extern t_list* lista_interfaces;

void IO(char* nombre_interfaz, int milisegundos);

#endif // KERNEL_HEADER_H_H