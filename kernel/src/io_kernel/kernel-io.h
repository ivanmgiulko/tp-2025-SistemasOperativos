#ifndef KERNEL_IO_H_
#define KERNEL_IO_H_

    #include "kernel-gestor.h"

    /**
	 * @file
	 * @brief #include "io_kernel/kernel-io.h"
	 */
    
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
    void inicializar_io(void*, int);
    t_io* buscar_io(char*);
#endif

