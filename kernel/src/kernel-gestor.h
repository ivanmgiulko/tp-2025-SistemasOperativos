#ifndef KERNEL_GESTOR_H_
#define KERNEL_GESTOR_H_

    #include <./utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/serializacion/serializacion.h>
    #include <utils/cliente/client.h>
    #include <utils/server/server.h>
    #include <utils/contrato/contrato.h>

    #include "config_kernel/kernel-config.h"
    #include "conexiones_kernel/conexion-k-memoria.h"
    #include "conexiones_kernel/conexion-kernel-cpu.h"


    /**
	 * @file
	 * @brief #include "kernel-gestor.h"
     *        El proposito de este Gestor es almacenar structs, variables, etc; que sean utilizados por los modulos
     *        y tenerelos a disposicion cuando sean requeridos.  
     * 
     * @note El manejo de errores relacionados a todos los structs, variables, etc; es contemplado en los archivos que utilizan
     *       los dichos tipos de datos. 
	 */ 

    // QUEUES
    extern t_queue* cola_procesos_new;
    extern t_queue* cola_procesos_ready;
    extern t_queue* cola_procesos_exec;
    extern t_queue* cola_procesos_blocked;
    extern t_queue* cola_procesos_exit;
    extern t_queue* cola_procesos_suspReady;
    extern t_queue* cola_procesos_suspBlocked;

    // CONFIG
    extern t_kernel_config* configuracion_kernel;

    // LOG
    extern t_log* logger_kernel;

    // LISTAS
    extern t_list* lista_de_io;

    //CONEXIONES
    extern int fd_server_io;
    extern int fd_conexion_memoria;
    extern int fd_server_kernel_interrupt;
    extern int fd_server_kernel_dispatch;
    extern int socket_dispatch;

#endif // KERNEL_GESTOR_H_