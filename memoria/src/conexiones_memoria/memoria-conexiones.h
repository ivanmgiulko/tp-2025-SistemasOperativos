#ifndef MEMORIA_CONEXIONES_H_
#define MEMORIA_CONEXIONES_H_

    #include "memoria-gestor.h"
	#include "./utils_memoria/memoria-utils.h"

    /**
	 * @file
	 * @brief #include "conexiones_memoria/memoria-conexiones-main.h"
	 */
    

    /**
	* @brief Crea un hilo cuando se conecta el Kernel o CPU
	* @param server_fd: Socket de escucha de Mmemoria
	* 
	*/
    void manejar_hilos_clientes(int server_fd);

    /**
	* @brief maneja las peticiones que le van llegando al modulo de Memoria
	* @param socket_cliente: Socket del cliente que se conecta a Memoria
	* @returns 0 (finalizando la conexion previamente) en caso de que se termine de manejar la conexion dada entre la Memo y Kernel o CPU
	*/
    int manejar_conexion_cliente(int socket_cliente);

	
	void manejar_peticion_de_instruccion(int socket_cliente,t_paquete* paquete, t_log* logger);

	void manejar_escritura_memoria(int socket_cliente, t_paquete* paquete, t_log* logger);

	void manejar_lectura_memoria(int socket_cliente, t_paquete* paquete, t_log* logger);

	void enviar_respuesta_kernel(char* mensaje, int socket_cliente);

    void enviar_proceso_terminado(uint8_t, t_paquete* , int );

	t_pcb* recibir_proceso_a_dumpear_desde_kernel(t_buffer* buffer);

	void enviar_respuesta_dump_memory(uint8_t pid, bool respuesta, int socket_cliente);

	bool realizar_dump_memory(int pid);

	void avisar_kernel_mande_otro_proceso(int socket_cliente);

	void enviar_datos_a_cpu(int );
					
	
#endif // MEMORIA_CONEXIONES_H_