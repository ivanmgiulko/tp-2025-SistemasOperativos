#ifndef CONEXION_KERNEL_IO_
#define CONEXION_KERNEL_IO_
    
    #include "kernel-gestor.h"
	#include "io_kernel/kernel-io.h"
    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-io.h"
	 */

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde la IO
	* @returns 0 (finalizando la conexion previamente) en caso de que se termine de manejar la conexion dada entre Kernel y IO
	*/
    int manejar_conexion_kernel_io();

#endif  /*CONEXION_KERNEL_IO_*/  
