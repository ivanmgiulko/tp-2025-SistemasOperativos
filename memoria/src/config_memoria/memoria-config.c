#include "memoria-config.h"

t_memoria_config* inicializar_config_memo(t_config* config){
    t_memoria_config* config_memoria = malloc(sizeof(*config_memoria));

    config_memoria->PUERTO_ESCUCHA      = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_memoria->TAM_MEMORIA         = strdup(config_get_string_value(config, "TAM_MEMORIA"));
    config_memoria->TAM_PAGINA          = strdup(config_get_string_value(config, "TAM_PAGINA"));
    config_memoria->ENTRADAS_POR_TABLA  = strdup(config_get_string_value(config, "ENTRADAS_POR_TABLA"));
    config_memoria->CANTIDAD_NIVELES    = strdup(config_get_string_value(config, "CANTIDAD_NIVELES"));
    config_memoria->RETARDO_MEMORIA     = strdup(config_get_string_value(config, "RETARDO_MEMORIA"));
    config_memoria->PATH_SWAPFILE       = strdup(config_get_string_value(config, "PATH_SWAPFILE"));
    config_memoria->RETARDO_SWAP        = strdup(config_get_string_value(config, "RETARDO_SWAP"));
    config_memoria->LOG_LEVEL           = strdup(config_get_string_value(config, "LOG_LEVEL"));
    config_memoria->DUMP_PATH           = strdup(config_get_string_value(config, "DUMP_PATH"));
    // config_memoria->PATH_INSTRUCCIONES  = strdup(config_get_string_value(config, "PATH_INSTRUCCIONES"));
    return config_memoria;
}

t_memoria_config* crear_config_memoria(char* path_a_config){
    config = iniciar_config(path_a_config);
    if(config == NULL){
        log_error(logger_memoria, "Error al iniciar config de MEMORIA");
        abort();
    }

    t_memoria_config* config_memoria_estructurada =  inicializar_config_memo(config);

    return config_memoria_estructurada;
}

void destruir_config_memoria(t_memoria_config* config) {
    free(config->PUERTO_ESCUCHA);
    free(config->TAM_MEMORIA);
    free(config->TAM_PAGINA);
    free(config->ENTRADAS_POR_TABLA);
    free(config->CANTIDAD_NIVELES);
    free(config->RETARDO_MEMORIA);
    free(config->PATH_SWAPFILE);
    free(config->RETARDO_SWAP);
    free(config->LOG_LEVEL);
    free(config->DUMP_PATH);
    free(config->PATH_INSTRUCCIONES);
    free(config);
}