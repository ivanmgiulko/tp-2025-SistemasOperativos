#ifndef CONTRATO_H_
#define CONTRATO_H_
#include <utils/proceso/process.h>
#include <utils/server/server.h>
#include <utils/utils.h>
#include <utils/serializacion/serializacion.h>


// STRUCT DE PETICION DE INSTRUCCION A MEMORIA
typedef struct {
    int pid;
    int pc;
} t_peticion_instruccion;

// STRUCT RESPUESTA DE INSTRUCCION A CPU
typedef struct {
    char* instruccion;
} t_respuesta_instruccion;

void* serializar_peticion_instruccion(t_peticion_instruccion*, int*);
t_peticion_instruccion* deserializar_peticion_instruccion(void*);

void* serializar_respuesta_instruccion(t_respuesta_instruccion*, int*);
t_respuesta_instruccion* deserializar_respuesta_instruccion(void*);

//char* obtener_instruccion(int, int, char*, t_log*);

#endif // CONTRATO_H_