#include "kernel-config.h"

t_kernel_config* inicializar_config_kernel(t_config* config_kernel_estructurada){
    t_kernel_config* config_kernel = malloc(sizeof(t_kernel_config));

    config_kernel->IP_MEMORIA                = strdup(config_get_string_value(config_kernel_estructurada, "IP_MEMORIA"));
    config_kernel->PUERTO_MEMORIA            = strdup(config_get_string_value(config_kernel_estructurada, "PUERTO_MEMORIA"));
    config_kernel->PUERTO_ESCUCHA_DISPATCH   = strdup(config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_DISPATCH"));
    config_kernel->PUERTO_ESCUCHA_INTERRUPT  = strdup(config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_INTERRUPT"));
    config_kernel->PUERTO_ESCUCHA_IO         = strdup(config_get_string_value(config_kernel_estructurada, "PUERTO_ESCUCHA_IO"));
    config_kernel->ALGORITMO_CORTO_PLAZO     = strdup(config_get_string_value(config_kernel_estructurada, "ALGORITMO_CORTO_PLAZO"));
    config_kernel->ALGORITMO_INGRESO_A_READY = strdup(config_get_string_value(config_kernel_estructurada, "ALGORITMO_INGRESO_A_READY"));
    config_kernel->ALFA                      = strdup(config_get_string_value(config_kernel_estructurada, "ALFA"));
    config_kernel->ESTIMACION_INICIAL        = strdup(config_get_string_value(config_kernel_estructurada, "ESTIMACION_INICIAL"));
    config_kernel->TIEMPO_SUSPENSION         = strdup(config_get_string_value(config_kernel_estructurada, "TIEMPO_SUSPENSION"));
    config_kernel->LOG_LEVEL                 = strdup(config_get_string_value(config_kernel_estructurada, "LOG_LEVEL"));    return config_kernel;
}

t_kernel_config* crear_config_kernel(char* path_a_config, t_log* logger){
    t_config* config_kernel = config_create(path_a_config);
    if(config_kernel == NULL){
        log_error(logger, "Error al iniciar config de KERNEL");
        abort();
    } else {
        t_kernel_config* config_kernel_estructurada =  inicializar_config_kernel(config_kernel);
        config_destroy(config_kernel);
        return config_kernel_estructurada;
    }
    
}