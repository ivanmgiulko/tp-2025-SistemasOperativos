#ifndef BUSQUEDA_DE_STRUCTS_H_
#define BUSQUEDA_DE_STRUCTS_H_

    #include <utils_kernel/kernel-structs-enums.h>
    #include "kernel-gestor.h"

    t_cpu_conectada* _buscar_cpu_libre();

    t_instancia_io* buscar_proceso_en_elemento(t_io* io, int socket);

    t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid);

    t_cpu_conectada* _buscar_cpu_en_lista(uint8_t);

    t_cpu_conectada* buscar_cpu_que_usa_proceso(t_list* cpus, uint8_t pid);

    /**
	 * @brief busca en la lista de IOs la que le pasemos por parametro
	 * @param char nombre de la IO a buscar
	 * @return devuelve TRUE si existe la lista que la pasamos, o FALSE caso contrario
	 */
    t_io* buscar_io(char* nombre_io);

    /**
	 * @brief nos confirma si es que existe o no la interfaz que debe usar el proceso al bloquearse
	 * @param char nombre de la interfaz que usa el proceso
	 * @return devuelve un t_instancia_io en caso de busqueda satisfactoria y NULL caso contrario
	 */
    t_instancia_io* devolver_instancia_disponible(char* nombreInterfaz);

    t_instancia_io* buscar_instancia_disponible(t_io* io);
    
    t_instancia_io* eliminar_y_devolver_instancia(t_io* io, int socket_io);

    t_io* buscar_io_en_lista( int socket);

    t_pcb* buscar_proceso_en_cola(t_estado* estado, uint8_t pid);
#endif // BUSQUEDA_DE_STRUCTS_H_