#ifndef MODULO_CPU_H_
#define MODULO_CPU_H_

    #include <./utils_kernel/kernel-structs-enums.h>

   /**
	* @brief envia el proceso a CPU (via dispatch) para que este lo manipule como se debe
	* @param t_peticion_instruccion*: PID y PC del proceso
	* @param int*: Socket de escucha de CPU - Dispatch 
	*/
	void enviar_proc_cpu(t_peticion_instruccion pcbInfo, int socket_cliente);

    /**
	* @brief recibe un proceso nuevo originado por la SYSCALL INIT_PROC
	* @param t_buffer*: Contenido del proceso a poner en NEW
	* @return devuelve un proceso creado a raiz de la syscall
	*/
	t_pcb* proceso_syscall_prueba(t_buffer* buffer);

    t_syscall_io _deserializar_syscall_io(int* , t_paquete* );

    char* deserializar_archivo_instrucciones(int* offset, t_paquete* paquete);

    int deserializar_tamanio_proceso(int* offset, t_paquete* paquete);

	void enviar_proceso_a_dumpear_en_memoria(int socket_cliente, t_pcb proceso);

#endif // MODULO_CPU_H_