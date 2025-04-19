#include"kernel-header.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

void IO(char* nombre_interfaz, int milisegundos) {
    if(!list_is_empty(lista_interfaces) && list_find(lista_interfaces, nombre_interfaz)) {
        log_info(logger_servidor, "La interfaz %ss se encuentra cargada en Kernel", nombre_interfaz);
        // Hacer el usleep en el IO
    } else { 
        log_error(logger_servidor, "La interfaz %s NO se encuentra cargada en Kernel", nombre_interfaz);
        // Mandar al proceso a la shitty
    }
}