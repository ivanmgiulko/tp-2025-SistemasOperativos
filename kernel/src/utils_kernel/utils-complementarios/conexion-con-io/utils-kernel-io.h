#ifndef UTILS_KERNEL_IO_H_
#define UTILS_KERNEL_IO_H_

    #include "kernel-gestor.h"

    extern t_lista_io* lista_de_io;

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

	void encolar_pcb_en_interfaz(t_io* interfaz, t_info_proceso_en_io* pcb);

	void alternar_estado_io(t_io* io);
	
#endif // UTILS_KERNEL_IO_H_

