#ifndef IO_COMUNICACION_H_
#define IO_COMUNICACION_H_
    
    #include "io-gestor.h"

    typedef struct 
    {
        uint8_t pid;
        int64_t tiempo;
    } t_info_proceso;
    
    void enviar_nombre_interfaz(char* mensaje, int socket_cliente);

    int manejar_conexion_io(int socket_cliente);

    t_info_proceso* recibir_proceso_bloqueado(t_buffer* buffer);

    void enviar_respuesta_kernel_IO(int socket_cliente, uint8_t pid);

#endif // IO_COMUNICACION_H_