#ifndef MANEJAR_CONEXION_IO_H_
#define MANEJAR_CONEXION_IO_H_
    
    #include "kernel-gestor.h"

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde la IO
	* @returns 0 (finalizando la conexion previamente) en caso de que se termine de manejar la conexion dada entre Kernel y IO
	*/
    void manejar_conexion_kernel_io();

	void _iniciar_server_para_io();

	int manejar_cliente_io();

#endif // MANEJAR_CONEXION_H_


