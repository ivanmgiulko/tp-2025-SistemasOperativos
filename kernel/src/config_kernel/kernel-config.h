#ifndef CONFIG_KERNEL_H
#define CONFIG_KERNEL_H

    #include <./utils/utils.h>

    /**
	 * @file
	 * @brief #include "config_kernel/kernel-config.h"
	 */
    
    /**
	 * @struct t_kernel_config
	 * @brief Manejo del config asociado al Kernel. Inicializarlo con 'crear_config_kernel(char*, t_log*)'
	 */
    typedef struct
    {
        char *IP_MEMORIA;
        char *PUERTO_MEMORIA;
        char *PUERTO_ESCUCHA_DISPATCH;
        char *PUERTO_ESCUCHA_INTERRUPT;
        char *PUERTO_ESCUCHA_IO;
        char *ALGORITMO_CORTO_PLAZO;
        char *ALGORITMO_INGRESO_A_READY;
        char *ALFA;
        char *ESTIMACION_INICIAL;
        char *TIEMPO_SUSPENSION;
        char *LOG_LEVEL;
    } t_kernel_config;

    /**
	* @brief Crea el config que tiene el Kernel asociado para poder utilizar los datos de este
	* @param char*: Path del config del Kernel
    * @param t_log*: Logger del Kernel
	* @returns el config creado para ser usado
	*/
    t_kernel_config* crear_config_kernel(char*, t_log*);

#endif

