#ifndef BUSQUEDA_DE_STRUCTS_H_
#define BUSQUEDA_DE_STRUCTS_H_

    #include <utils_kernel/kernel-structs-enums.h>
    #include "kernel-gestor.h"

    t_cpu_conectada* _buscar_cpu_libre();

    t_info_proceso_en_io* buscar_proceso_en_io(t_list* lista_procesos, uint8_t pid);

    t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid);

    t_cpu_conectada* _buscar_cpu_en_lista(uint8_t);

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

    t_pcb* buscar_proceso_en_cola_exit(t_list* cola_exit, uint8_t pid);

    t_io* buscar_io_en_lista(t_list*, uint8_t);

    t_info_proceso_en_io* buscar_proceso_en_elemento(t_list* , uint8_t );

#endif // BUSQUEDA_DE_STRUCTS_H_