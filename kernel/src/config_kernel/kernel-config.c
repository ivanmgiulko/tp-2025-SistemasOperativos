#include "kernel-config.h"

t_kernel_config* inicializar_config_kernel(t_config* config_kernel_estructurada){
    t_kernel_config* config_kernel = malloc(sizeof(*config_kernel));

    config_kernel->IP_MEMORIA = config_get_string_value(config_kernel_estructurada, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_string_value(config_kernel_estructurada, "PUERTO_MEMORIA");
    config_kernel->PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_DISPATCH");
    config_kernel->PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_INTERRUPT");
    config_kernel->PUERTO_ESCUCHA_IO = config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_IO");
    config_kernel->ALGORITMO_CORTO_PLAZO = config_get_string_value(config_kernel_estructurada, "ALGORITMO_CORTO_PLAZO");
    config_kernel->ALGORITMO_INGRESO_A_READY = config_get_string_value(config_kernel_estructurada, "ALGORITMO_INGRESO_A_READY");
    config_kernel->ALFA = config_get_string_value(config_kernel_estructurada, "ALFA");
    config_kernel->TIEMPO_SUSPENSION = config_get_string_value(config_kernel_estructurada, "TIEMPO_SUSPENSION");
    config_kernel->LOG_LEVEL = config_get_string_value(config_kernel_estructurada, "LOG_LEVEL");
    return config_kernel;
}

t_kernel_config* crear_config_kernel(char* path_a_config, t_log* logger){
    t_config* config_kernel = iniciar_config(path_a_config);
    if(config_kernel == NULL){
        log_error(logger, "Error al iniciar config de KERNEL");
        abort();
    }

    t_kernel_config* config_kernel_estructurada =  inicializar_config_kernel(config_kernel);
    return config_kernel_estructurada;
}