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

	void enviar_proceso_a_io(uint8_t pid, int64_t tiempo, int socket_cliente);

	uint8_t recibir_proceso_bloqueado(t_buffer* buffer);

#endif  /*CONEXION_KERNEL_IO_*/  
