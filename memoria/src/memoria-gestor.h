#ifndef MEMORIA_GESTOR_H_
#define MEMORIA_GESTOR_H_

    #include <utils/server/server.h>
    #include <utils/cliente/client.h>
    #include <utils/proceso/process.h>
    #include <utils/serializacion/serializacion.h>
    #include <utils/contrato/contrato.h>
    #include <utils/utils.h>
    #include "config_memoria/memoria-config.h"
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>
    /**
	 * @file
	 * @brief #include "memoria-gestor.h"
     *        El proposito de este Gestor es almacenar structs, variables, etc; que sean utilizados por los modulos
     *        y tenerelos a disposicion cuando sean requeridos.  
     * 
     * @note El manejo de errores relacionados a todos los structs, variables, etc; es contemplado en los archivos que utilizan
     *       los dichos tipos de datos. 
	 */ 

    // LOGS
    extern t_log* logger_memoria;

    // CONFIG

    // VARIABLES
    extern int cantMemoria;
    extern int retardo_memoria;
    extern bool primera_conexion_a_memoria;
#endif // MEMORIA_GESTOR_H_