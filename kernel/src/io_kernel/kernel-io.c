#include "kernel-io.h"

t_list* lista_de_io = NULL;

void inicializar_lista_io() {
    lista_de_io = list_create();
    log_info(logger_kernel, "Lista de IO inicializada");
}

void inicializar_io(void* nombre_io, int socket_io) {
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->socket = socket_io;
    list_add(lista_de_io, io);
    log_info(logger_kernel, "IO inicializado: %s", io->nombre);
    t_io* io_encontrado = list_get(lista_de_io, 0);
    log_info(logger_kernel, "Nombre %s", io_encontrado->nombre);
}

t_io* buscar_io(char* nombre_io) {
   
}

