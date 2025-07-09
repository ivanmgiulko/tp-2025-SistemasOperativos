#include "manejar-conexion-cpu.h"
#include <utils_kernel/utils-complementarios/conexion-con-cpu/utils-kernel-cpu.h>

void _iniciar_server_para_cpu_interrupt() 
{ 
    char* puerto_kernel_interrupt = configuracion_kernel->PUERTO_ESCUCHA_INTERRUPT;

    fd_server_kernel_interrupt = iniciar_servidor(puerto_kernel_interrupt, logger_kernel);
	if(fd_server_kernel_interrupt == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }

    pthread_t hilo_servidor_kernel_interrupt;
    pthread_create(&hilo_servidor_kernel_interrupt, NULL, (void*)manejar_conexion_kernel_interrupt, NULL);
    pthread_detach(hilo_servidor_kernel_interrupt);
}

void _iniciar_server_para_cpu_dispatch() 
{ 
    char* puerto_kernel_dispatch = configuracion_kernel->PUERTO_ESCUCHA_DISPATCH;

    fd_server_kernel_dispatch = iniciar_servidor(puerto_kernel_dispatch, logger_kernel);
	if(fd_server_kernel_dispatch == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }

    pthread_t hilo_servidor_kernel_dispatch;
    pthread_create(&hilo_servidor_kernel_dispatch, NULL, (void*)manejar_conexion_kernel_dispatch, NULL);
    pthread_detach(hilo_servidor_kernel_dispatch);
}

void manejar_conexion_kernel_interrupt() {
    while (1) {
        int socket_interrupt = esperar_cliente(fd_server_kernel_interrupt, logger_kernel);
        if (socket_interrupt == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en interrupt");
            continue;
        }
        pthread_mutex_lock(&lista_cpus->mutex_lista);
        uint8_t id_cpu = _recibir_handshake_de_cpu(socket_interrupt, SOCKET_INTERRUPT);
        pthread_mutex_unlock(&lista_cpus->mutex_lista);

        _agregar_socket_en_cpu(id_cpu, SOCKET_INTERRUPT, socket_interrupt);

        pthread_t hilo_cliente_interrupt;
        pthread_create(&hilo_cliente_interrupt, NULL, (void*)manejar_cliente_interrupt, (void*)&socket_interrupt);
        pthread_detach(hilo_cliente_interrupt);
    }
}

void manejar_conexion_kernel_dispatch() {
    while (1) {
        socket_dispatch = esperar_cliente(fd_server_kernel_dispatch, logger_kernel);
        if (socket_dispatch == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en dispatch");
            continue;
        }

        pthread_mutex_lock(&lista_cpus->mutex_lista);
        uint8_t id_cpu = _recibir_handshake_de_cpu(socket_dispatch, SOCKET_DISPATCH);
        pthread_mutex_unlock(&lista_cpus->mutex_lista);

        _agregar_socket_en_cpu(id_cpu, SOCKET_DISPATCH, socket_dispatch);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_dispatch;
        pthread_create(&hilo_cliente_dispatch, NULL, (void*)manejar_cliente_dispatch, (void*)&socket_dispatch);
        pthread_detach(hilo_cliente_dispatch);
    }
}

uint8_t _recibir_handshake_de_cpu(int socket_cliente_cpu, int parte_cpu) {

    uint8_t handshake;  // Valor del CPU ID
    uint8_t resultOk = 0;
    uint8_t resultError = -1;

    recv(socket_cliente_cpu, &handshake, sizeof(uint8_t), MSG_WAITALL);
        
    if (handshake < 256 && handshake >= 0) {
        switch (parte_cpu) {
        case SOCKET_INTERRUPT:
            log_debug(logger_kernel, "Nueva conexión en Interrupt: socket %d", socket_cliente_cpu);
            break;
        
        case SOCKET_DISPATCH:
            log_debug(logger_kernel, "Nueva conexión en Dispatch: socket %d", socket_cliente_cpu);
            break;
        }
        send(socket_cliente_cpu, &resultOk, sizeof(uint8_t), 0);
    } else {
        send(socket_cliente_cpu, &resultError, sizeof(uint8_t), 0);
    }

    return handshake;
}