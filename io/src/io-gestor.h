#ifndef IO_GESTOR_H_
#define IO_GESTOR_H_

    #include <./utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/serializacion/serializacion.h>
    #include <utils/cliente/client.h>
    #include <utils/server/server.h>

    /**
	* @file
	* @brief #include "io-gestor.h"
    *        El proposito de este Gestor es almacenar structs, variables, etc; que sean utilizados por los modulos
    *        y tenerelos a disposicion cuando sean requeridos.  
    * 
    * @note El manejo de errores relacionados a todos los structs, variables, etc; es contemplado en los archivos que utilizan
    *       los dichos tipos de datos. 
	*/ 


    // LOGS
    extern t_log* logger_io;

    // CONEXIONES
    extern int conexion_kernel_fd;

#endif // IO_GESTOR_H_