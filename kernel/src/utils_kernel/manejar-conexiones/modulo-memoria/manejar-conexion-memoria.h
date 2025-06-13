#ifndef MANEJAR_CONEXION_MEMORIA_H_
#define MANEJAR_CONEXION_MEMORIA_H_
    
    #include "kernel-gestor.h"

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde la memoria
	* @param socket_cliente socket del cliente que se conecta a Kernel 
	* @returns 0 (finalizando la conexion previ) en caso de que se termine de manejar la conexion dada entre Kernel y Memoria
	*/
    int manejar_conexion_kernel_memoria(int socket_cliente);

#endif // MANEJAR_CONEXION_MEMORIA_H_
