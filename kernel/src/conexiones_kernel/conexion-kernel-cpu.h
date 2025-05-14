#ifndef CONEXION_KERNEL_CPU_H_
#define CONEXION_KERNEL_CPU_H_

    #include "kernel-gestor.h"
	#include "conexion-kernel-io.h"

    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-cpu.h"
	 */

	typedef struct {
        char* dispositivo;
        uint32_t dispositivo_length;
        int64_t tiempo;
    } t_param_io;

    // Funciones 
    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU Interrupt
	*/
    void manejar_conexion_kernel_interrupt();

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde el CPU Dispatch
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
	*/
    void* manejar_cliente_dispatch(void* socket_cliente_ptr);

	/**
	* @brief envia el proceso a CPU (via dispatch) para que este lo manipule como se debe
	* @param t_peticion_instruccion*: PID y PC del proceso
	* @param int*: Socket de escucha de CPU - Dispatch 
	*/
	void enviar_proc_cpu(t_peticion_instruccion pcbInfo, int socket_cliente);

	/**
	* @brief recibe la SYSCALL IO enviada desde el CPU
	* @param t_buffer*: Contenido de la interfaz a recibir
	* @return devuelve la interfaz a la que llama el proceso desde CPU
	*/
	t_param_io* deserializar_syscall_io(t_buffer* buffer);

	/**
	* @brief recibe un proceso nuevo originado por la SYSCALL INIT_PROC
	* @param t_buffer*: Contenido del proceso a poner en NEW
	* @return devuelve un proceso creado a raiz de la syscall
	*/
	t_pcb* proceso_syscall_prueba(t_buffer* buffer);

#endif // CONEXION_KERNEL_CPU_H_