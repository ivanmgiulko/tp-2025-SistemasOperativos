#include"kernel-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

void _chequear_segundo_argumento_es_numero(char* numero_en_string) 
{
    int longitud_string = string_length(numero_en_string);

    int num_prueba = atoi(numero_en_string);
    char* num_prueba_en_string = string_itoa(num_prueba);
    int num_prueba_en_string_length = string_length(num_prueba_en_string);
    free(num_prueba_en_string);
    
    if(!(longitud_string == num_prueba_en_string_length)) { 
        fprintf(stderr, "Bro realmente puso mal el tamanio en memoria \n");
        abort();
    }
}

// HILOS PARA MANEJAR LAS PETICIONES
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

void _iniciar_server_para_io() 
{ 
    char* puerto_io = configuracion_kernel->PUERTO_ESCUCHA_IO;

    fd_server_io = iniciar_servidor(puerto_io, logger_kernel);
	if(fd_server_io == -1){
        log_error(logger_kernel, "Error al iniciar servidor de IO");
        abort();
    }

    pthread_t hilo_servidor_io;
    pthread_create(&hilo_servidor_io, NULL, (void*)manejar_conexion_kernel_io, NULL);
    pthread_detach(hilo_servidor_io);
}
