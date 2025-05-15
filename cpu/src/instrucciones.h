
#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_
#include "./cpu-gestor.h"
typedef struct {
    char* direccion;
    char* datos;
} t_param_write;

typedef struct {
    char* direccion;
    int tamanio;
} t_param_read;

typedef struct {
    int valor;
} t_param_goto;

typedef struct {
    char* dispositivo;
    int tiempo;
} t_param_io2;

typedef struct {
    char* archivo;
    int tamanio;
} t_param_init_proc;

typedef struct {
} t_param_vacio;

typedef union {
    t_param_write write;
    t_param_read read;
    t_param_io2 io;
    t_param_goto go_to;
    t_param_init_proc init_proc;
    t_param_vacio vacio;
} parametros_instruccion;  // ✅ Ahora está definido antes de usarlo

typedef enum {
    INSTR_NOOP,
    INSTR_WRITE,
    INSTR_READ,
    INSTR_GOTO,
    INSTR_IO,
    INSTR_INIT_PROC,
    INSTR_DUMP_MEMORY,
    INSTR_EXIT,
    CANT_INSTRUCCIONES
} instruccion_t;

typedef struct {
    instruccion_t tipo;
    parametros_instruccion parametros;
} t_instruccion;

t_instruccion* decode(char* instruccion);


t_instruccion* parse_noop(char* linea);
t_instruccion* parse_write(char* linea);
t_instruccion* parse_read(char* linea);
t_instruccion* parse_goto(char* linea);
t_instruccion* parse_io(char* linea);
t_instruccion* parse_init_proc(char* linea);
t_instruccion* parse_dump_memory(char* linea);
t_instruccion* parse_exit(char* linea);

void ejecutar_instruccion(t_instruccion* );
void free_instruccion(t_instruccion* );
typedef t_instruccion* (*func_parse_t)(char* linea);
char* obtener_nombre_instruccion(instruccion_t );

extern func_parse_t parse_funciones[CANT_INSTRUCCIONES];
    

typedef struct {
    char* nombre;
    instruccion_t tipo;
} instruccion_map_entry;

extern instruccion_map_entry mapa_instrucciones[];

#endif // INSTRUCCIONES_H_
