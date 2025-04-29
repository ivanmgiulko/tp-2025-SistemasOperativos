#ifndef IO_COMUNICACION_H_
#define IO_COMUNICACION_H_
    
    #include "io-gestor.h"

    void enviar_nombreInterfaz(char* mensaje, int socket_cliente);
    int manejar_conexion_io(int socket_cliente);

#endif // IO_COMUNICACION_H_