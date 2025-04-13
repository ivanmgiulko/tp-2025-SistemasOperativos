#include <utils/server.h>
// #include <memoria-header.h>  

int main(int argc, char* argv[]) {
    char* puerto_servidor_memoria;
    int cliente_fd;
    t_log* logger_memoria= log_create("./memoria.log", "log", true, LOG_LEVEL_TRACE);
    t_config* config_memoria = iniciar_config("./memoria.config");
  

    // VALIDAR QUE LA CONFIG SE HAYA CREADO CORRECTAMENTE
    if(config_memoria == NULL){
        log_error(logger_memoria, "Error al crear config de memoria");
        abort();
    }

    log_info(logger_memoria, "Config de memoria creado con exito");
    
    puerto_servidor_memoria =  config_get_string_value(config_memoria, "PUERTO_ESCUCHA");

    int server_memoria_fd = iniciar_servidor(puerto_servidor_memoria, logger_memoria);

    // VALIDAR QUE EL VALOR DEL FILE DESCRIPTOR DEL SERVER ES CORRECTO
    if(server_memoria_fd == -1){
        log_error(logger_memoria, "Error al iniciar servidor de memoria");
        abort();
    }

    log_info(logger_memoria, "Servidor de memoria iniciado con exito");
    
    // CREACION DE HILO PARA EL SERVIDOR DE MEMORIA
    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, (void*)manejar_hilos, (void*)server_memoria_fd, (void*)logger_memoria);
    pthread_detach(hilo_servidor);
    
    while(1){
        
    }

    return 0;
}
