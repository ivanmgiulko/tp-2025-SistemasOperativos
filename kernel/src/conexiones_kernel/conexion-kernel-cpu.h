#ifndef CONEXION_KERNEL_CPU_H_
#define CONEXION_KERNEL_CPU_H_

    #include "kernel-gestor.h"
	#include "conexion-kernel-io.h"

    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-cpu.h"
	 */

	 typedef enum{
		SOCKET_INTERRUPT,
		SOCKET_DISPATCH
	 }t_sockets_cpu;

	typedef struct { 
		int socket_interrupt;
		int socket_dispatch;
		uint8_t id_cpu;
		uint8_t pid_en_cpu;
	} t_cpu_conectada;

	// STRUCTS PARA MANEJAR LAS SYSCALLS
	typedef struct {
        char* dispositivo;
        int64_t tiempo;
    } t_syscall_io;

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

	t_syscall_io _deserializar_syscall_io(int , t_paquete* );

	int _deserializar_pid(int offset, t_paquete* paquete);

	bool _tiene_el_pid(void* ptr, void* data);

	t_pcb* _sacar_pcb_de_exec(int pid);

	void _enviar_a_finalizar_proceso(uint8_t pid);

	uint8_t _recibir_handshake_de_cpu(int socket_cliente_cpu, int parte_cpu);

	void _agregar_cpu_en_lista_cpus(uint8_t);

	void _agregar_socket_en_cpu(uint8_t, t_sockets_cpu, int);

	uint8_t _recibir_handshake_de_cpu(int , int );

	t_cpu_conectada* _agregar_cpu_en_lista(uint8_t );

	t_cpu_conectada* _buscar_cpu_en_lista(uint8_t);


	/**
	* @brief recibe un proceso nuevo originado por la SYSCALL INIT_PROC
	* @param t_buffer*: Contenido del proceso a poner en NEW
	* @return devuelve un proceso creado a raiz de la syscall
	*/
	t_pcb* proceso_syscall_prueba(t_buffer* buffer);

#endif // CONEXION_KERNEL_CPU_H_