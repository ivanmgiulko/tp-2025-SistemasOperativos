#ifndef UTILS_KERNEL_IO_H_
#define UTILS_KERNEL_IO_H_

    #include "kernel-gestor.h"

    extern t_lista_io* lista_de_io;

	/**
	 * @brief inicia un modulo de IO y lo mete en la lista de IOs
	 * @param char nombre del interfaz agregada en la lista
	 * @param int socket de la conexion IO - Kernel
	 */
    t_io* inicializar_io(char* nombre_io, int socket_io);

	void insertar_nueva_instancia_io(t_list* instancias, int socket_io);

	void encolar_pcb_en_interfaz(t_io* interfaz, uint8_t* pid);

	void alternar_estado_io(t_io* io);

	void eliminar_proceso_de_io(t_list* procesos_en_io, uint8_t pid);

	void eliminar_interfaz(t_io* nombre);
	
#endif // UTILS_KERNEL_IO_H_

