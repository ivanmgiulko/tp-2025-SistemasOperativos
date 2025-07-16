#ifndef CONTRATO_H_
#define CONTRATO_H_
#include <utils/proceso/process.h>
#include <utils/server/server.h>
#include <utils/utils.h>
#include <utils/serializacion/serializacion.h>


// STRUCT DE PETICION DE INSTRUCCION A MEMORIA
typedef struct {
    uint8_t pid;
    uint16_t pc;
} t_peticion_instruccion;

void* serializar_peticion_instruccion(t_peticion_instruccion*, int*);
t_peticion_instruccion* deserializar_peticion_instruccion(void*);

void* serializar_respuesta_instruccion(char*, int*);

//char* obtener_instruccion(int, int, char*, t_log*);

#endif // CONTRATO_H_