#ifndef KERNEL_HEADER_H_
#define KERNEL_HEADER_H_

    #include "kernel-gestor.h"
    #include "utils_kernel/kernel-utils.h"
    #include "config_kernel/kernel-config.h"

    #include "planificador_kernel/kernel-planificador.h"
    
    #include "conexiones_kernel/conexion-kernel-cpu.h"
    #include "conexiones_kernel/conexion-kernel-io.h"
    #include "conexiones_kernel/conexion-kernel-memoria.h"

    /**
	 * @file
	 * @brief #include "kernel-main.h"
	 */
    
    /**
	 * @def
	 * @brief Cantidad minima de argumentos que se debe ingresar al ejecutar una 
     *        instancia de Kernel (./bin/kernel [arg1] [arg2])
	 */
    #define CANT_MINIMA_ARGUMENTOS 2

    // CONFIG
    t_kernel_config* configuracion_kernel;

    // LOG
    t_log* logger_kernel;

    // LISTAS
    t_list* lista_interfaces;

    // CONEXIONES
    int fd_server_io;
    int fd_server_kernel_interrupt;
    int fd_server_kernel_dispatch;
    int socket_dispatch;
    int socket_io;
    
    // FUNCIONES 

#endif // KERNEL_HEADER_H_