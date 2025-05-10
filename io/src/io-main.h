#ifndef IO_MAIN_H_
#define IO_MAIN_H_

    #include "io-gestor.h"
    #include "conexiones_io/conexion-io-kernel.h"

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

    // CONEXIONES
    int conexion_kernel_fd;

    // FUNCIONES


#endif // IO_MAIN_H_