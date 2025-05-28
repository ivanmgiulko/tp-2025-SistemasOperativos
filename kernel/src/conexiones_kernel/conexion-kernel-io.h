#ifndef CONEXION_KERNEL_IO_
#define CONEXION_KERNEL_IO_
    
    #include "kernel-gestor.h"
	
    /**
	 * @file
	 * @brief #include "conexiones_kernel/conexion-kernel-io.h"
	 */

	extern t_list* lista_de_io;

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
	 * @brief inicia la lista que va a contener a las interfaces
	 */
	void inicializar_lista_io();

	/**
	 * @brief inicia un modulo de IO y lo mete en la lista de IOs
	 * @param char nombre del interfaz agregada en la lista
	 * @param int socket de la conexion IO - Kernel
	 */
    void inicializar_io(char* nombre_io, int socket_io);

	/**
	 * @brief busca en la lista de IOs la que le pasemos por parametro
	 * @param t_list lista de IOs
	 * @param char nombre de la IO a buscar
	 * @return devuelve TRUE si existe la lista que la pasamos, o FALSE caso contrario
	 */
    t_io* buscar_io(t_list* lista_de_io, char* nombre_io);

	/**
	 * @brief nos confirma si es que existe o no la interfaz que debe usar el proceso al bloquearse
	 * @param char nombre de la interfaz que usa el proceso
	 * @return devuelve TRUE si existe la lista que la pasamos, o FALSE caso contrario
	 */
    t_io* funcion_syscall_IO(char* nombreInterfaz);

	void encolar_pcb_en_interfaz(t_io* interfaz, t_info_proceso_en_io* pcb);

#endif  /*CONEXION_KERNEL_IO_*/  
