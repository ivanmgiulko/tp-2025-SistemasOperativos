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
    
    #define CANT_MINIMA_ARGUMENTOS 1

    // LOGS
    t_log* logger_memoria;
    
    // CONFIG
    t_config* config;
    t_memoria_config* config_memoria; 

    // VARIABLES
    int cantMemoria;
    t_memoria_del_sistema* memoria_del_sistema;
    procesos_en_swap_t* procesos_en_swap; 
    int retardo_memoria;
    bool primera_conexion_a_memoria = true;
    // LISTA DE PROCESOS EN SWAP
    
#endif // MEMORIA_H_