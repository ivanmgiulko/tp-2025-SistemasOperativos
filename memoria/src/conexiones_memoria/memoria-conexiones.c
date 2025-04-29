#include "memoria-conexiones.h"

void manejar_hilos_clientes(int server_fd){

    while(1){
        int socket_cliente = esperar_cliente(server_fd, logger_memoria);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion_cliente, (void*)socket_cliente);
        pthread_detach(hilo_cliente);
    }

}

int manejar_conexion_cliente(int socket_cliente){
	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_memoria);
			break;
        case TAMANIO_PROCESO:
			char* tamProceso = recibir_tamProceso(socket_cliente);
			int tamProcesoEnInt = atoi(tamProceso);
			log_info(logger_memoria, "Tamanio de la memoria antes: %d", cantMemoria);
			log_info(logger_memoria, "Recibi el tamProceso desde KERNEL: %d", tamProcesoEnInt);
			cantMemoria -= tamProcesoEnInt;
			log_info(logger_memoria, "Tamanio de la memoria despues: %d", cantMemoria);
			break;
		case INSTRUCCION:
			break;
			
		case -1:
			log_error(logger_memoria, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_memoria, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_cliente);
	return EXIT_SUCCESS;
}

