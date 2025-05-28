#ifndef CPU_UTILS_H
#define CPU_UTILS_H

    #include "./cpu-gestor.h"
    #include "./instrucciones.h"

    void pedir_instruccion_a_memoria(t_peticion_instruccion*);
    
    void manejar_respuesta_de_instruccion(t_paquete* paquete);

    void _crear_conexion_kernel_interrupt(char*, char*, char*);

    void _crear_conexion_kernel_dispatch(char*, char*, char*);

    void _crear_conexion_cpu_memoria(char*, char*);

    void _handshake_kernel_con_cpu_id(int, char*);

#endif // CPU_UTILS_H