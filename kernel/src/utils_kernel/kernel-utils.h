#ifndef KERNEL_UTILS_H_
#define KERNEL_UTILS_H_
#include "kernel-gestor.h"

    typedef struct {
        char* archivo;
        int tamanio;
    } t_param_init_proc;

    bool funcion_syscall_IO(char* nombreInterfaz, int64_t tiempo);

#endif // KERNEL_UTILS_H_