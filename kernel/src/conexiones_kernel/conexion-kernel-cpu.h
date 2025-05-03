#ifndef CONEXION_KERNEL_CPU_H_
#define CONEXION_KERNEL_CPU_H_

    #include "kernel-gestor.h"

    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-cpu.h"
	 */

    // Funciones 
    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU Interrupt
    *
	*/
    void manejar_conexion_kernel_interrupt();

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU Dispatch
    *
	*/
    void manejar_conexion_kernel_dispatch();

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU - Interrupt
	* @param void*: Socket de escucha de CPU - Interrupt 
	* @returns 0 (finalizando la conexion previ) en caso de que se termine de manejar la conexion dada entre Kernel y Memoria
	*/
    int manejar_cliente_interrupt(void*);

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU - Dispatch
	* @param void*: Socket de escucha de CPU - Dispatch 
	* @returns 0 (finalizando la conexion previ) en caso de que se termine de manejar la conexion dada entre Kernel y Memoria
	*/
    int manejar_cliente_dispatch(void*);

	void enviar_proc_cpu(t_peticion_instruccion pcbInfo, int socket_cliente);

#endif // CONEXION_KERNEL_CPU_H_