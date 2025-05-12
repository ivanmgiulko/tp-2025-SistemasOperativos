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
    t_config* config_memoria;

    //STRUCTS
    
    
    // VARIABLES
    int cantMemoria;
    
    t_memoriaDelSistema* memoriaDelSistema; 
#endif // MEMORIA_H_