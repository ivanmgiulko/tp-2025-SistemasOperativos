#ifndef IO_GESTOR_H_
#define IO_GESTOR_H_

    #include <./utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/serializacion/serializacion.h>
    #include <utils/cliente/client.h>
    #include <utils/server/server.h>

    // LOGS
    extern t_log* logger_io;

    // CONEXIONES
    extern int conexion_kernel_fd;

#endif // IO_GESTOR_H_