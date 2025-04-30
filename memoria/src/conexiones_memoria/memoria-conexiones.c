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
			log_info(logger_memoria, "Recibi la instruccion desde CPU");
			t_paquete* paquete = recibir_paquete_instruccion(socket_cliente);
			log_info(logger_memoria, "%d", paquete->buffer->size);
			if (paquete == NULL) {
				log_error(logger_memoria, "Error al recibir el paquete de instruccion");
				return EXIT_FAILURE;
			}
			manejar_instruccion(socket_cliente, paquete, logger_memoria);
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

void manejar_instruccion(int socket_cliente, t_paquete* paquete, t_log* logger) {
    if (paquete->buffer->size < sizeof(int) * 2) {
        log_error(logger, "El tamaño del buffer es insuficiente para deserializar la instrucción");
        return;
    }

    t_peticion_instruccion* peticion = deserializar_peticion_instruccion(paquete->buffer->stream);

    log_info(logger, "PID recibido: %d", peticion->pid);
    log_info(logger, "PC recibido: %d", peticion->pc);

    free(peticion);
}
