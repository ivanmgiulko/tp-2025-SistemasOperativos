#include "manejar-conexion-io.h"

void _iniciar_server_para_io() 
{ 
    char* puerto_io = configuracion_kernel->PUERTO_ESCUCHA_IO;

    fd_server_io = iniciar_servidor(puerto_io, logger_kernel);
	if(fd_server_io == -1){
        log_error(logger_kernel, "Error al iniciar servidor de IO");
        abort();
    }

    pthread_t hilo_servidor_io;
    pthread_create(&hilo_servidor_io, NULL, (void*)manejar_conexion_kernel_io, NULL);
    pthread_detach(hilo_servidor_io);
}


void manejar_conexion_kernel_io() {
    while (1) {
        socket_io = esperar_cliente(fd_server_io, logger_kernel);
        if (fd_server_io == -1) {
            log_error(logger_kernel, "Error al aceptar cliente de IO");
            continue;
        }

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_io;
        pthread_create(&hilo_cliente_io, NULL, (void*)manejar_cliente_io, (void*)&socket_io);
        pthread_detach(hilo_cliente_io);
    }
}