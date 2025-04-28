#ifndef MEMORIA_HEADER_H_
#define MEMORIA_HEADER_H_
#include <utils/server/server.h>
#include <utils/contrato/contrato.h>

extern t_log* logger_servidor;
extern t_config* config_memoria;
void manejar_conexion_memoria(int);

#endif // MEMORIA_HEADER_H_H