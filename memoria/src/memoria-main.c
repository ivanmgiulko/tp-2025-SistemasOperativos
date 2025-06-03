#include "memoria-main.h"

int main(int argc, char* argv[]) {
    char* puerto_servidor_memoria;
    
    logger_memoria = log_create("./memoria.log", "log", true, LOG_LEVEL_TRACE);
    t_memoria_config* config_memoria = crear_config_memoria("./memoria.config", logger_memoria);
    cantMemoria = atoi(config_memoria->TAM_MEMORIA);
    memoria_del_sistema = malloc(sizeof(t_memoria_del_sistema));
    *memoria_del_sistema = crear_memoria_del_sistema(config_memoria);

    puerto_servidor_memoria = config_memoria->PUERTO_ESCUCHA;

    int server_memoria_fd = iniciar_servidor(puerto_servidor_memoria, logger_memoria);

    // VALIDAR QUE EL VALOR DEL FILE DESCRIPTOR DEL SERVER ES CORRECTO
    if(server_memoria_fd == -1){
        log_error(logger_memoria, "Error al iniciar servidor de memoria");
        abort();
    }

    log_info(logger_memoria, "Servidor de memoria iniciado con exito");
    
    //CREACION DE HILO PARA EL SERVIDOR DE MEMORI
    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, (void*)manejar_hilos_clientes, (void*)server_memoria_fd);
    pthread_join(hilo_servidor, NULL);
    
    return 0;
}
