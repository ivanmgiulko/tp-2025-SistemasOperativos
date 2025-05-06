#include "kernel-io.h"

t_list* lista_de_io = NULL;

void inicializar_lista_io() {
    lista_de_io = list_create();
    log_info(logger_kernel, "Lista de IO inicializada");
}

void inicializar_io(char* nombre_io, int socket_io) {
    
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->socket = socket_io;
    list_add(lista_de_io, io);

    t_io* io_prueba = malloc(sizeof(t_io));
    io_prueba->nombre = "MOUSE";
    io_prueba->procesos = list_create();
    io_prueba->socket = 100;
    list_add(lista_de_io, io_prueba);

    log_info(logger_kernel, "IO inicializado: %s", io->nombre);
    // t_io* io_encontrado = list_get(lista_de_io, 0);
    // log_info(logger_kernel, "Nombre %s", io_encontrado->nombre);
 }

t_io* buscar_io(t_list* lista_de_io, char* nombre_io) {
    bool _es_el_io(void* elemento) {
        t_io* io = (t_io*) elemento;
        return string_contains(io->nombre, nombre_io);
    }

    return list_find(lista_de_io, _es_el_io);
}

