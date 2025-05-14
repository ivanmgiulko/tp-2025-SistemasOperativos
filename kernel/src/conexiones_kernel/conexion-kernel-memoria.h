#ifndef CONEXIONES_K_MEMORIA_H_
#define CONEXIONES_K_MEMORIA_H_
    
    #include "kernel-gestor.h"

    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-k-memoria.h"
	 */

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde la memoria
	* @param socket_cliente socket del cliente que se conecta a Kernel 
	* @returns 0 (finalizando la conexion previ) en caso de que se termine de manejar la conexion dada entre Kernel y Memoria
	*/
    int manejar_conexion_kernel_memoria(int socket_cliente);

	/**
	* @brief envia el tamanio del proceso a la memoria 
	* @param tam_proceso tamanio del proceso a serializar
	* @param socket_cliente socket de a quien le enviamos el paquete serializado
	*/
	void enviar_tamanio_proceso(char* tam_proceso, int socket_cliente);

#endif // CONEXIONES_K_MEMORIA_H_