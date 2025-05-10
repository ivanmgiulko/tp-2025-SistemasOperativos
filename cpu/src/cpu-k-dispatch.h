#ifndef CPU_K_DISPTACH_H_
#define CPU_K_DISPTACH_H_
#include "cpu-gestor.h"

// Prueba de mandar IO a Kernel


typedef struct {
    char* dispositivo;
    uint32_t dispositivo_length;
    int64_t tiempo;
} t_param_io; //eliminar cuando se termine de probar y usar el de 2 parametros que está en ./instrucciones.h

int manejar_conexion_kernel_dispatch();

t_peticion_instruccion* deserializar_info_pcb(t_buffer* buffer);

void enviar_io_kernel(t_param_io pruebaIO, int socket_cliente);

void enviar_syscall_init_proc_kernel(t_param_init_proc prueba_init_proc, int socket_cliente);

#endif // CPU_K_DISPTACH_H_