#include "memoria-config.h"

t_memoria_config* inicializar_config_kernel(t_config* config_memoria_estructurada){
    t_memoria_config* config_memoria = malloc(sizeof(*config_memoria));

    config_memoria->PUERTO_ESCUCHA = config_get_string_value(config_memoria_estructurada, "PUERTO_ESCUCHA");
    config_memoria->TAM_MEMORIA = config_get_string_value(config_memoria_estructurada, "TAM_MEMORIA");
    config_memoria->TAM_PAGINA = config_get_string_value(config_memoria_estructurada, "TAM_PAGINA");
    config_memoria->ENTRADAS_POR_TABLA = config_get_string_value(config_memoria_estructurada, "ENTRADAS_POR_TABLA");
    config_memoria->CANTIDAD_NIVELES = config_get_string_value(config_memoria_estructurada, "CANTIDAD_NIVELES");
    config_memoria->RETARDO_MEMORIA = config_get_string_value(config_memoria_estructurada, "RETARDO_MEMORIA");
    config_memoria->PATH_SWAPFILE = config_get_string_value(config_memoria_estructurada, "PATH_SWAPFILE");
    config_memoria->RETARDO_SWAP = config_get_string_value(config_memoria_estructurada, "RETARDO_SWAP");
    config_memoria->LOG_LEVEL = config_get_string_value(config_memoria_estructurada, "LOG_LEVEL");
    config_memoria->DUMP_PATH = config_get_string_value(config_memoria_estructurada, "DUMP_PATH");
    config_memoria->PATH_INSTRUCCIONES = config_get_string_value(config_memoria_estructurada, "PATH_INSTRUCCIONES");
    return config_memoria;
}

t_memoria_config* crear_config_memoria(char* path_a_config, t_log* logger){
    t_config* config_memoria = iniciar_config(path_a_config);
    if(config_memoria == NULL){
        log_error(logger, "Error al iniciar config de MEMORIA");
        abort();
    }

    t_memoria_config* config_memoria_estructurada =  inicializar_config_kernel(config_memoria);
    return config_memoria_estructurada;
}