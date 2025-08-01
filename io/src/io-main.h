#ifndef IO_MAIN_H_
#define IO_MAIN_H_

    #include "io-gestor.h"
    #include "utils_io/io-utils.h"

    /**
	* @file
	* @brief #include "io-main.h"
	*/ 

    /**
	 * @def CANT_MINIMA_ARGUMENTOS
	 * @brief Cantidad minima de argumentos que se debe ingresar al ejecutar una 
     *        instancia de IO (./bin/io [arg1]. Ejemplo: ./bin/io MOUSE
	 */
    #define CANT_MINIMA_ARGUMENTOS 1

    // LOGS
    t_log* logger_io;

    t_config* config_io;

    // CONEXIONES
    int conexion_kernel_fd;

    // FUNCIONES


#endif // IO_MAIN_H_