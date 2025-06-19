#ifndef MEMORIA_H_
#define MEMORIA_H_

    #include "config_memoria/memoria-config.h"
    #include "conexiones_memoria/memoria-conexiones.h"
    #include "utils_memoria/memoria-utils.h"
    #include "memoria-gestor.h"
    /**
	 * @file
	 * @brief #include "memoria-main.h"
	 */
    
    // LOGS
    t_log* logger_memoria;
    
    // CONFIG
    t_memoria_config* config_memoria;

    // VARIABLES
    int cantMemoria;
    t_memoria_del_sistema* memoria_del_sistema; 

#endif // MEMORIA_H_