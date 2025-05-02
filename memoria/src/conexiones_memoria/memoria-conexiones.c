#include "memoria-conexiones.h"

void manejar_hilos_clientes(int server_fd){

    while(1){
        int socket_cliente = esperar_cliente(server_fd, logger_memoria);
		log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", socket_cliente);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion_cliente, (void*)socket_cliente);
        pthread_detach(hilo_cliente);
    }

}

int manejar_conexion_cliente(int socket_cliente){
	
	
	// Primero recibimos el codigo de operacion
	
	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);

		paquete->codigo_operacion = recibir_operacion(socket_cliente);
	
		// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
		// recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
		// log_debug(logger_memoria, "Tamaño del buffer recibido: %d", (int)paquete->buffer->size);
		// paquete->buffer->stream = malloc(paquete->buffer->size);
		// recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
		// log_debug(logger_memoria, "valor codop: %d", cod_op);
		
		switch (paquete->codigo_operacion) {
			case MENSAJE:
				recibir_mensaje(socket_cliente, logger_memoria);
				break;

        	case PROCESO_MEMORIA:
				t_pcbMemoria* proceso_A_inicializar = deserializarProceso(paquete->buffer);
				cantMemoria -= proceso_A_inicializar->tamanioMemoria;
				if(cantMemoria < 0) {
					// NO hay memoria para este proceso
					// enviar a Kernel que no se pudo
					cantMemoria += proceso_A_inicializar->tamanioMemoria;
					enviar_respuestaKernel("No hay espacio en memoria", socket_cliente);
				} else {
					// Hay memoria para este proceso
					// le mandamos a Kernel el num de tabla de primer nivel
					log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", proceso_A_inicializar->pid, proceso_A_inicializar->tamanioMemoria);
					enviar_respuestaKernel("Hay espacio en memoria", socket_cliente);
				}
				return EXIT_SUCCESS;
				break; 
			case INSTRUCCION:
				log_info(logger_memoria, "Recibi la instruccion desde CPU");
				paquete = recibir_paquete_instruccion(socket_cliente);
				log_info(logger_memoria, "Tamaño del buffer recibido: %d", paquete->buffer->size);				
				manejar_instruccion(socket_cliente, paquete, logger_memoria);
				break;
			
			case LINUS_TORVALDS:
				log_error(logger_memoria, "LINUS TORVALD TE MALDIGO");
				log_error(logger_memoria, "el cliente se desconecto.");
				return EXIT_FAILURE;
				break;
	
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

void enviar_respuestaKernel(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = PROCESO_MEMORIA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}