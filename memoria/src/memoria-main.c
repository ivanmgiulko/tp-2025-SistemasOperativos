#include "memoria-main.h"


int main(int argc, char* argv[]) {

    if(argc <= CANT_MINIMA_ARGUMENTOS){
		fprintf(stderr, "Falta la ruta al config de memoria webon... \n");
		return EXIT_FAILURE;
	}

    char* puerto_servidor_memoria; 

    char* path_relativo = string_duplicate("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/prueba/");
    //  char* path_relativo = string_duplicate("/home/utnso/so-deploy/tp-2025-1c-FAMILIA-MATRIX/prueba/");

    // argv[1] = "prueba_cortoplazo/memoria.config";

	string_append(&path_relativo, argv[1]);
    
    logger_memoria = log_create("./memoria.log", "log", true, LOG_LEVEL_TRACE);
    config_memoria = crear_config_memoria(path_relativo); // Creo el config instanciado globalmente
    config_destroy(config);  
    cantMemoria = atoi(config_memoria->TAM_MEMORIA);
    memoria_del_sistema = malloc(sizeof(t_memoria_del_sistema));
    *memoria_del_sistema = crear_memoria_del_sistema();
    retardo_memoria = atoi(config_memoria->RETARDO_MEMORIA);
    procesos_en_swap = malloc(sizeof(procesos_en_swap_t));
    *procesos_en_swap = crear_lista_procesos_en_swap(); 
   
    inicializar_swap();

    puerto_servidor_memoria = config_memoria->PUERTO_ESCUCHA;

    int server_memoria_fd = iniciar_servidor(puerto_servidor_memoria, logger_memoria);
    // VALIDAR QUE EL VALOR DEL FILE DESCRIPTOR DEL SERVER ES CORRECTO
    if(server_memoria_fd == -1){
        log_error(logger_memoria, "Error al iniciar servidor de memoria");
        abort();
    }

    log_debug(logger_memoria, "Servidor de memoria iniciado con exito");
    log_trace(logger_memoria, "Servidor de memoria iniciado en el puerto: %s", puerto_servidor_memoria);

    //CREACION DE HILO PARA EL SERVIDOR DE MEMORIA
    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, (void*)manejar_hilos_clientes, (void*)server_memoria_fd);
    pthread_join(hilo_servidor, NULL);
    
    return 0;
}
