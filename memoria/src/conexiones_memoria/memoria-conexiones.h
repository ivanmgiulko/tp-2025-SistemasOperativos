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
    int manejar_conexion_cliente(void* socket_cliente);

#endif // MEMORIA_CONEXIONES_H_