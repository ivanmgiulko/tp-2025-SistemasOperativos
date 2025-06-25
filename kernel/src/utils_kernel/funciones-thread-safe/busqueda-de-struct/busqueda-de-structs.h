#ifndef BUSQUEDA_DE_STRUCTS_H_
#define BUSQUEDA_DE_STRUCTS_H_

    #include <utils_kernel/kernel-structs-enums.h>
    #include "kernel-gestor.h"

    t_cpu_conectada* _buscar_cpu_libre();

    t_instancia_io* buscar_proceso_en_elemento(t_list* lista_procesos_de_io, int socket);

    t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid);

    t_cpu_conectada* _buscar_cpu_en_lista(uint8_t);

    t_cpu_conectada* buscar_cpu_que_usa_proceso(t_list* cpus, uint8_t pid);

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
	 * @return devuelve un t_instancia_io en caso de busqueda satisfactoria y NULL caso contrario
	 */
    t_instancia_io* devolver_instancia_disponible(char* nombreInterfaz);

    t_instancia_io* buscar_instancia_disponible(t_list* lista_de_instancias);
    
    t_instancia_io* eliminar_y_devolver_instancia(t_list* lista_de_instancias, int socket_io);

    t_pcb* buscar_proceso_en_cola_exit(t_list* cola_exit, uint8_t pid);

    t_io* buscar_io_en_lista(t_list* lista_base, int socket);

#endif // BUSQUEDA_DE_STRUCTS_H_