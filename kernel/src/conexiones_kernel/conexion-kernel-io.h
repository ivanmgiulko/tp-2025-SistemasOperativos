#ifndef CONEXION_KERNEL_IO_
#define CONEXION_KERNEL_IO_
    
    #include "kernel-gestor.h"
	
    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-io.h"
	 */

    /**
	* @brief Maneja las peticiones que le llegan al Kernel desde la IO
	* @returns 0 (finalizando la conexion previamente) en caso de que se termine de manejar la conexion dada entre Kernel y IO
	*/
    int manejar_conexion_kernel_io();

	/**
	* @brief envia un proceso al modulo de IO para que se bloquee
	* @param uint8_t PID enviado al modulo para que se bloquee
	* @param int64_t tiempo que el proceso va a estar bloqueado
	* @param int socket de conexion IO con Kernel
	*/
	void enviar_proceso_a_io_para_bloqueo(uint8_t pid, int64_t tiempo, int socket_cliente);

	/**
	* @brief recibe un proceso que fue desbloqueado desde el modulo de IO
	* @param t_buffer buffer donde esta contenido el PID del proceso desbloqueado
	* @return el PID dle proceso recien desbloqueado
	*/
	uint8_t _recibir_proceso_bloqueado(t_buffer* buffer);

	/**
	 * @struct t_io
	 * @brief asocia un modulo IO a su lista de procesos pendientes de ejecucion
	 */
    typedef struct {
        char* nombre;
        t_list* procesos;
        int socket;
    } t_io;

    extern t_list* lista_de_io;

    void inicializar_io(char* nombre_io, int socket_io);

    void inicializar_lista_io();

    t_io* buscar_io(t_list* lista_de_io, char* nombre_io);

    bool funcion_syscall_IO(char* nombreInterfaz, int64_t tiempo);

#endif  /*CONEXION_KERNEL_IO_*/  
