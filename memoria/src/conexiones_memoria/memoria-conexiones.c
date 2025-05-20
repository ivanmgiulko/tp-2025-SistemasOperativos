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
	
	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_cliente);

		switch (paquete->codigo_operacion) {
			case MENSAJE:
				recibir_mensaje(socket_cliente, logger_memoria);
				break;
				
			case PROCESO_MEMORIA:
				
				recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);

				t_pcbMemoria* proceso_a_inicializar = deserializarProceso(paquete->buffer);
				log_debug(logger_memoria, "PID recibido: %d", proceso_a_inicializar->pid);

				log_debug(logger_memoria, "Cantidad de memoria antes: %d", cantMemoria);
				cantMemoria -= proceso_a_inicializar->tamanioMemoria;
				log_debug(logger_memoria, "Cantidad de memoria despues: %d", cantMemoria);
				
				if(cantMemoria < 0) {
					// NO hay memoria para este proceso
					// enviar a Kernel que no se pudo
					log_info(logger_memoria, "No se puedo crear el proceso con PID: %d en memoria por falta de espacio", proceso_a_inicializar->pid);
					cantMemoria += proceso_a_inicializar->tamanioMemoria;
					enviar_respuesta_kernel("No hay espacio en memoria", socket_cliente);
				} else {
					// Hay memoria para este proceso
					// le mandamos a Kernel el num de tabla de primer nivel

					//Agrego el proceso (ver que pasa si hay error aca)
					agregar_proceso(proceso_a_inicializar);
					//falta agregar caso de error para el log
					log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", proceso_a_inicializar->pid, proceso_a_inicializar->tamanioMemoria);
					enviar_respuesta_kernel("Hay espacio en memoria", socket_cliente);
				}

				// Liberar memoria del paquete recibido
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);
				break; 

			case PROCESO_FINALIZAR:

				recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);

				t_pcbMemoria* proceso_a_finalizar = deserializarProceso(paquete->buffer);
				cantMemoria += proceso_a_finalizar->tamanioMemoria;
				log_warning(logger_memoria, "el tamanio de la memo es ahora: %d", cantMemoria);

				int pidParaEliminar = proceso_a_finalizar->pid;
				int pidEliminado = finalizar_proceso(pidParaEliminar);
				if(pidEliminado != -1){
					log_info(logger_memoria, "Se elimino el proceso con PID: %d de memoria", pidEliminado);
					enviar_proceso_a_finalizar_kernel(*proceso_a_finalizar, socket_cliente);
				}
				else{
					log_error(logger_memoria, "No se pudo eliminar el proceso con PID: %d de memoria", pidParaEliminar);
					//Aca hay que ver que pasa en el lado de kernell en el caso de que no se elimine y como avisarles
					enviar_proceso_terminado("NO FINALIZA EL PROCESO :(", socket_cliente);
				}
				break; 

			case INSTRUCCION:
				log_info(logger_memoria, "Recibi la petición de instruccion desde CPU");
				t_paquete* paquete_tmp = recibir_paquete_instruccion(socket_cliente);
				if (paquete_tmp == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de instrucción");
					break;
				}
				//	log_info(logger_memoria, "Tamaño del buffer recibido: %d", paquete_tmp->buffer->size);
				manejar_peticion_de_instruccion(socket_cliente, paquete_tmp, logger_memoria);

				// Libera el paquete
				free(paquete_tmp->buffer->stream);
				free(paquete_tmp->buffer);
				free(paquete_tmp);
				break;
			case WRITE_MEMORIA:
				log_info(logger_memoria, "Recibí paquete de ejecución de WRITE");
				t_paquete* paquete_write = recibir_paquete_instruccion(socket_cliente);
				if (paquete_write == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de WRITE");
					break;
				}
				manejar_escritura_memoria(socket_cliente, paquete_write, logger_memoria);

				// Enviar confirmación de éxito al cliente
				t_paquete* paquete_confirmacion_write = malloc(sizeof(t_paquete));
				paquete_confirmacion_write->codigo_operacion = WRITE_MEMORIA;
				paquete_confirmacion_write->buffer = malloc(sizeof(t_buffer));
				char* mensaje_confirmacion_write = "WRITE completado con éxito";
				paquete_confirmacion_write->buffer->size = strlen(mensaje_confirmacion_write) + 1;
				paquete_confirmacion_write->buffer->stream = malloc(paquete_confirmacion_write->buffer->size);
				memcpy(paquete_confirmacion_write->buffer->stream, mensaje_confirmacion_write, paquete_confirmacion_write->buffer->size);

				int bytes_confirmacion_write = paquete_confirmacion_write->buffer->size + 2 * sizeof(int);
				void* a_enviar_write = serializar_paquete(paquete_confirmacion_write, bytes_confirmacion_write);
				send(socket_cliente, a_enviar_write, bytes_confirmacion_write, 0);
				log_info(logger_memoria, "Enviando confirmación de WRITE a CPU");
				log_info(logger_memoria, "Mensaje de confirmación: %s", mensaje_confirmacion_write);
				free(a_enviar_write);
				eliminar_paquete(paquete_confirmacion_write);

				free(paquete_write->buffer->stream);
				free(paquete_write->buffer);
				free(paquete_write);
				break;
			case READ_MEMORIA:
				log_info(logger_memoria, "Recibí paquete de ejecución de READ");
				t_paquete* paquete_read = recibir_paquete_instruccion(socket_cliente);
				if (paquete_read == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de READ");
					break;
				}
				manejar_lectura_memoria(socket_cliente, paquete_read, logger_memoria);
			
				// Enviar confirmación de éxito al cliente
				t_paquete* paquete_confirmacion_read = malloc(sizeof(t_paquete));
				paquete_confirmacion_read->codigo_operacion = READ_MEMORIA;
				paquete_confirmacion_read->buffer = malloc(sizeof(t_buffer));
				char* mensaje_confirmacion_read = "READ completado con éxito";
				paquete_confirmacion_read->buffer->size = strlen(mensaje_confirmacion_read) + 1;
				paquete_confirmacion_read->buffer->stream = malloc(paquete_confirmacion_read->buffer->size);
				memcpy(paquete_confirmacion_read->buffer->stream, mensaje_confirmacion_read, paquete_confirmacion_read->buffer->size);

				int bytes_confirmacion_read = paquete_confirmacion_read->buffer->size + 2 * sizeof(int);
				void* a_enviar_read = serializar_paquete(paquete_confirmacion_read, bytes_confirmacion_read);
				send(socket_cliente, a_enviar_read, bytes_confirmacion_read, 0);
				log_info(logger_memoria, "Enviando confirmación de READ a CPU");
				log_info(logger_memoria, "Mensaje de confirmación: %s", mensaje_confirmacion_read);
				free(a_enviar_read);
				eliminar_paquete(paquete_confirmacion_read);

				free(paquete_read->buffer->stream);
				free(paquete_read->buffer);
				free(paquete_read);
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

	return EXIT_SUCCESS;
}

void manejar_peticion_de_instruccion(int socket_cliente, t_paquete* paquete, t_log* logger) {
    if (paquete->buffer->size < sizeof(int) * 2) {
        log_error(logger, "El tamaño del buffer es insuficiente para deserializar la instrucción");
        return;
    }

    t_peticion_instruccion* peticion = deserializar_peticion_instruccion(paquete->buffer->stream);

    log_info(logger, "PID recibido: %d", peticion->pid);
    log_info(logger, "PC recibido: %d", peticion->pc);

	//Obtengo la instruccion correspondiente al PID y PC recibido de cpu
	t_respuesta_instruccion* respuesta = malloc(sizeof(t_respuesta_instruccion));
	respuesta->instruccion = obtener_instruccion(peticion->pid, peticion->pc);
	//Entra a este if cuando el pc es mayor a cant de instrucciones
	if(strcmp(respuesta->instruccion, "PC FINALIZADO")== 0){
	
		log_info(logger, "No hay más instrucciones a ejecutar para este proceso");

		log_debug(logger, "Serializando paquete:");
		log_debug(logger, "Código de operación: %d", FIN_PID);

		t_paquete* paquete = malloc(sizeof(t_paquete));

		paquete->codigo_operacion = FIN_PID;
		int bytes = sizeof(int);
		void* paquete_pc_fin = malloc(bytes);
		memcpy(paquete_pc_fin, &(paquete->codigo_operacion), sizeof(int));
		log_debug(logger, "Tamaño del buffer: %d", bytes);
		send(socket_cliente, paquete_pc_fin, bytes, 0);

		free(paquete->buffer);
		free(paquete_pc_fin);
		free(paquete);
		return;
	}

	//Entra aca si Memoria del sistema no incializada o pid no encontrado
	else if(respuesta->instruccion == NULL){
	 	log_error(logger, "pid no encontrado o memoria_del_sistema/procesos no están inicializados");
		return;
	} 

	//Entra acá si encontro el proceso y la instrucción
	else{
		log_info(logger, "Instrucción encontrada: %s", respuesta->instruccion);
		//Serializo la respuesta
		int size_respuesta;
		void* respuesta_serializada = serializar_respuesta_instruccion(respuesta, &size_respuesta);
		if(respuesta_serializada == NULL) {
			log_warning(logger, "Error al serializar la respuesta de instruccion");
			return;
		}
		log_debug(logger, "Serializando paquete:");
		log_debug(logger, "Código de operación: %d", INSTRUCCION);
		log_debug(logger, "Tamaño del buffer: %ld", size_respuesta - sizeof(op_code) - sizeof(uint32_t));
		log_debug(logger, "Instrucción: %s", respuesta->instruccion);

		//Envio la instruccion serializada envio a CPU 
		//log_info(logger, "Size_respuesta= %d", size_respuesta);
		log_info(logger, "Enviando Instrucción a CPU");
		int bytes_enviados = send(socket_cliente, respuesta_serializada, size_respuesta, 0);
		if (bytes_enviados <= 0) {
			log_error(logger, "Fallo al enviar la instrucción al CPU");
		}
	}

	//Libero memoria
    free(peticion);
	log_trace(logger_memoria, "se llega a los frees");
	free(respuesta->instruccion);
	free(respuesta);
}

void manejar_escritura_memoria(int socket_cliente, t_paquete* paquete, t_log* logger) {
    t_buffer* buffer = paquete->buffer;

    int desplazamiento = 0;

    char* direccion = leer_string_desde_buffer(buffer, &desplazamiento);
    char* datos = leer_string_desde_buffer(buffer, &desplazamiento);

    log_info(logger, "[MEMORIA] WRITE recibido - Direccion: %s | Datos: %s", direccion, datos);

    // Aca va la lógica para escribir el dato en memoria física (resta crear la tabla de páginas y la MMU)

    free(direccion);
    free(datos);
}

void manejar_lectura_memoria(int socket_cliente, t_paquete* paquete, t_log* logger) {
    t_buffer* buffer = paquete->buffer;

    int desplazamiento = 0;

    char* direccion = leer_string_desde_buffer(buffer, &desplazamiento);

    log_info(logger, "[MEMORIA] READ recibido - Direccion: %s", direccion);

    // Aca va la lógica para leer el dato en memoria física (resta crear la tabla de páginas y la MMU)

    free(direccion);
}

void enviar_respuesta_kernel(char* mensaje, int socket_cliente)
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

void enviar_proceso_terminado(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = PROCESO_FINALIZADO;
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
