#ifndef MANEJAR_CONEXION_CPU_H_
#define MANEJAR_CONEXION_CPU_H_

    #include "kernel-gestor.h"

    // Funciones 

    void _iniciar_server_para_cpu_interrupt();

    void _iniciar_server_para_cpu_dispatch();

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

    uint8_t _recibir_handshake_de_cpu(int socket_cliente_cpu, int parte_cpu);

    t_pcb* sacar_proceso_de_exec(uint8_t pid, uint16_t pc);

    void actualizar_metricas_proceso(t_pcb* proceso, uint16_t pc);
#endif // MANEJAR_CONEXION_CPU_H_