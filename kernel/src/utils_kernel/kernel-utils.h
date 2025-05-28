#ifndef KERNEL_UTILS_H_
#define KERNEL_UTILS_H_

    #include "kernel-gestor.h"

    void _chequear_segundo_argumento_es_numero(char* numero_en_string);

    void _iniciar_server_para_cpu_interrupt();

    void _iniciar_server_para_cpu_dispatch();

    void _iniciar_server_para_io();

    void _enviar_desde_new_a_ready(bool , char* );

    void _enviar_desde_susp_ready_a_ready(bool , char* );

    void _iniciar_cuando_apreta_enter();

    t_io* buscar_io_en_lista(t_list*, uint8_t);

    t_info_proceso_en_io* buscar_proceso_en_elemento(t_list* , uint8_t );

#endif // KERNEL_UTILS_H_