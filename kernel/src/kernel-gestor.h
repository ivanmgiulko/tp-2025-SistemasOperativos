#ifndef KERNEL_GESTOR_H_
#define KERNEL_GESTOR_H_

    #include <utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/serializacion/serializacion.h>
    #include <utils/cliente/client.h>
    #include <utils/server/server.h>
    #include <utils/contrato/contrato.h>
    
    #include "config_kernel/kernel-config.h"
    #include "utils_kernel/utils-complementarios/archivo-planificador/utils-planificador.h"

    #include "utils_kernel/kernel-structs-enums.h"

    /**
	 * @file
	 * @brief #include "kernel-gestor.h"
     *        El proposito de este Gestor es almacenar structs, variables, etc; que sean utilizados por los modulos
     *        y tenerelos a disposicion cuando sean requeridos.  
     * 
     * @note El manejo de errores relacionados a todos los structs, variables, etc; es contemplado en los archivos que utilizan
     *       los dichos tipos de datos. 
	 */ 

    // CONFIG
    extern t_kernel_config* configuracion_kernel;

    // LOG
    extern t_log* logger_kernel;

    // LISTAS
    extern t_lista_io* lista_de_io;
    extern t_lista_cpus* lista_cpus;

    //CONEXIONES
    extern int fd_server_io;
    extern int fd_conexion_memoria;
    extern int fd_server_kernel_interrupt;
    extern int fd_server_kernel_dispatch;
    extern int socket_dispatch;
    extern int socket_io;

#endif // KERNEL_GESTOR_H_