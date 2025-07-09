#ifndef IO_COMUNICACION_H_
#define IO_COMUNICACION_H_
    
    #include "io-gestor.h"

    /**
	 * @file
	 * @brief #include "conexiones_io/conexion-io-kernel.h"
     *        
	 */ 

    /**
	 * @struct t_info_proceso
	 * @brief recibe el proceso desde kernel cuando atiende la syscall
	 */    
    typedef struct 
    {
        uint8_t pid;
        int64_t tiempo;
    } t_info_proceso;
    
    /**
	* @brief Envia el nombre de la interfaz al kernel para realizar el handshake y cargar la IO en el Kernel
	* @param mensaje: nombre de la interfaz
    * @param socket_cliente: socket de Kernel
	*/
    void enviar_nombre_interfaz(char* mensaje, int socket_cliente);

    /**
	* @brief Maneja los paquetes que le llega al modulo de IO desde Kernel
    * @param socket_cliente: socket de Kernel
	* @returns Un entero si es que finaliza bien o mal la conexion con Kernel
	*/
    int manejar_conexion_io(int socket_cliente);

    /**
	* @brief Recibe un PID y tiempo de bloqueo desde el Kernel
    * @param buffer: buffer del paquete recibido previamente en el manejo de la conexion
	* @returns un struct t_info_proceso* para realizar el bloqueo del proceso
	*/
    t_info_proceso* recibir_proceso_bloqueado(t_buffer* buffer);

    /**
    * @brief envia al Kernel un PID de un proceso que termino su bloqueo
    * @param socket_cliente: socket de Kernel
    * @param pid: pid de proceso desbloqueado
    * @param codigo_operacion: codigo de operacion que indica en que estado se encuentra el proceso en kernel
    * 
    */
    void enviar_pid_desbloqueado(int socket_cliente, uint8_t pid, int codigo_operacion);

#endif // IO_COMUNICACION_H_