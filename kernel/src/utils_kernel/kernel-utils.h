#ifndef KERNEL_UTILS_H_
#define KERNEL_UTILS_H_

    #include "kernel-gestor.h"

    void _chequear_segundo_argumento_es_numero(char* numero_en_string);

    void _iniciar_server_para_cpu_interrupt();

    void _iniciar_server_para_cpu_dispatch();

    void _iniciar_server_para_io();

#endif // KERNEL_UTILS_H_