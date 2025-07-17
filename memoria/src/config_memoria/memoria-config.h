#ifndef CONFIG_KERNEL_H
#define CONFIG_KERNEL_H

     #include "memoria-gestor.h"
    #include <./utils/utils.h>

    /**
	 * @file
	 * @brief #include "config_memoria/memoria-config.h"
	 */
    
    /**
	 * @struct t_memoria_config
	 * @brief Manejo del config asociado al Kernel. Inicializarlo con 'crear_config_kernel(char*, t_log*)'
	 */
    typedef struct
    {
        char *PUERTO_ESCUCHA;
        char *TAM_MEMORIA;
        char *TAM_PAGINA;
        char *ENTRADAS_POR_TABLA;
        char *CANTIDAD_NIVELES;
        char *RETARDO_MEMORIA;
        char *PATH_SWAPFILE;
        char *RETARDO_SWAP;
        char *LOG_LEVEL;
        char *DUMP_PATH;
        char *PATH_INSTRUCCIONES;
    } t_memoria_config;

    /**
	* @brief Crea el config que tiene la memoria asociado para poder utilizar los datos de este
	* @param char*: Path del config de la memoria
    * @param t_log*: Logger de la memoria
	* @returns el config creado para ser usado
	*/
    t_memoria_config* crear_config_memoria(char*);
    t_memoria_config* inicializar_config_memo(t_config* );
    void destruir_config_memoria(t_memoria_config* config);
    extern t_memoria_config* config_memoria;
    extern t_config* config;
#endif

