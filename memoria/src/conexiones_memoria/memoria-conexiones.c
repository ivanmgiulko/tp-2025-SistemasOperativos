#include "memoria-conexiones.h"

void manejar_hilos_clientes(int server_fd){

    while(1){
        int socket_cliente = esperar_cliente(server_fd, logger_memoria);
		log_info(logger_memoria, "## CLIENTE Conectado - FD del socket: %d", socket_cliente);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion_cliente, (void*)socket_cliente);
        pthread_detach(hilo_cliente);
    }

}

int manejar_conexion_cliente(int socket_cliente){
	
	while (1) {
		t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);
		paquete->codigo_operacion = recibir_cod_operacion(socket_cliente);
		log_warning(logger_memoria, "[RECV] Recibido código de operación: %s (nro: %d)", convertir_cod_op_a_string(paquete->codigo_operacion), paquete->codigo_operacion);
		recibir_buffer_en_paquete(socket_cliente, paquete);
		log_debug(logger_memoria, "Tamaño del buffer recibido: %d", paquete->buffer->size);
		//Verifico que el contenido no sea nulo
		if(paquete->buffer->stream == NULL){
			log_error(logger_memoria, "ERROR al recibir paquete (contenido NULL)");
			continue;
		}
		switch (paquete->codigo_operacion) {

			case MENSAJE:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				//recibir_mensaje(socket_cliente, logger_memoria);
				loggear_mensaje_desde_buffer(paquete->buffer, logger_memoria);
				break;

			case CPU_PIDE_DATOS:
				log_info(logger_memoria, "Recibí petición de datos desde CPU");
				//recibir_mensaje(socket_cliente, logger_memoria);
				loggear_mensaje_desde_buffer(paquete->buffer, logger_memoria);
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);

				enviar_datos_a_cpu(socket_cliente);
				log_info(logger_memoria, "Datos enviados a CPU");
				
				break;

			case PROCESO_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				pthread_mutex_lock(&memoria_del_sistema->mutex);

				t_pcbMemoria* proceso_a_inicializar = deserializar_proceso(paquete->buffer);
				log_trace(logger_memoria, "PID recibido para inicializar: %d", proceso_a_inicializar->pid);

				//log_debug(logger_memoria, "Cantidad de memoria antes: %d", cantMemoria);
				cantMemoria -= proceso_a_inicializar->tamanioMemoria;
				log_warning(logger_memoria, "Cantidad de memoria restante: %d", cantMemoria);
				
				if(cantMemoria < 0) {
					
					log_info(logger_memoria, "No se puedo agregar el proceso con PID: %d en memoria por falta de espacio", proceso_a_inicializar->pid);
					cantMemoria += proceso_a_inicializar->tamanioMemoria;
					enviar_respuesta_kernel("No hay espacio en memoria", socket_cliente);
				} else {
					// Hay memoria para este proceso
					// le mandamos a Kernel el num de tabla de primer nivel

					//Agrego el proceso (ver que pasa si hay error aca)
					if(buscar_indice_de_proceso_en_memoria(proceso_a_inicializar->pid) != -1){
						log_trace(logger_memoria, "Solicita Desuspender el proceso con PID: %d", proceso_a_inicializar->pid);
						desuspender_proceso_swap(proceso_a_inicializar->pid);
					}else {
						agregar_proceso(proceso_a_inicializar);
					}
					//falta agregar caso de error para el log
					log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", proceso_a_inicializar->pid, proceso_a_inicializar->tamanioMemoria);
					enviar_respuesta_kernel("Hay espacio en memoria", socket_cliente);
				}

				pthread_mutex_unlock(&memoria_del_sistema->mutex);
				break; 
		
			case PROCESO_SUSPENDIDO_MEMORIA:
				
				pthread_mutex_lock(&memoria_del_sistema->mutex);

				//RETARDO DE MEMORIA
				usleep( atoi(config_memoria->RETARDO_SWAP) * 1000);
			
				t_pcbMemoria* proceso_suspendido = deserializar_proceso(paquete->buffer);

				cantMemoria += proceso_suspendido->tamanioMemoria;

				// Mover proceso de RAM a SWAP
				suspender_proceso_swap(proceso_suspendido->pid);

				avisar_kernel_mande_otro_proceso(socket_cliente);

				pthread_mutex_unlock(&memoria_del_sistema->mutex);

				break;

			case PROCESO_FINALIZAR:
				
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);

				t_pcbMemoria* proceso_a_finalizar = deserializar_proceso(paquete->buffer);
 
				int pidParaEliminar = proceso_a_finalizar->pid;
				log_warning(logger_memoria, "PID recibido para finalizar: %d", pidParaEliminar);
				int pidEliminado = finalizar_proceso(pidParaEliminar);				

				cantMemoria += proceso_a_finalizar->tamanioMemoria;
				log_warning(logger_memoria, "el tamanio de la memo es ahora: %d", cantMemoria);

				log_info(logger_memoria, "Se elimino el proceso con PID: %d de memoria", pidEliminado);
				//enviar_proceso_terminado("NO FINALIZA EL PROCESO :(", socket_cliente);
				
				enviar_proceso_terminado(pidEliminado, socket_cliente);
			
				break; 

			case INSTRUCCION:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);

				log_info(logger_memoria, "Recibi la petición de instruccion desde CPU");
				
				manejar_peticion_de_instruccion(socket_cliente, paquete);
				break;

			case OBTENER_MARCO_CORRESPONDIENTE:
				//RETARDO DE MEMORIA
				//(retardo equivalene a acceder a cada nivel)
				usleep( retardo_memoria * atoi(config_memoria->CANTIDAD_NIVELES) * 1000);

				log_info(logger_memoria, "Recibí solicitud de ACCESO A TABLA DE PAGINAS	");
				
				manejar_acceso_tablas_de_paginas(socket_cliente, paquete);
				break;

			case WRITE_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de WRITE");
				manejar_escritura_memoria(socket_cliente, paquete);
				break;

			case READ_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de READ");
				manejar_lectura_memoria(socket_cliente, paquete);
				break;

			case PROCESO_DUMPEAR:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de DUMP_MEMORY");
				

				t_pcb* proceso_a_dumpear = recibir_proceso_a_dumpear_desde_kernel(paquete->buffer);
				log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", proceso_a_dumpear->pid);

				// realizar el DUMP de "proceso_a_dumpear"
				bool resultado = realizar_dump_memory(proceso_a_dumpear->pid);
				if (!resultado){
					log_error(logger_memoria, "Fallo al realizar el DUMP_MEMORY");
				}

				if (!resultado) { // Sale mal
					log_error(logger_memoria, "Fallo al realizar el DUMP_MEMORY");
					enviar_respuesta_dump_memory(proceso_a_dumpear->pid, false, socket_cliente);
				} else { // Sale bien
					log_info(logger_memoria, "Éxito al realizar el DUMP_MEMORY: enviando al kernel");
					enviar_respuesta_dump_memory(proceso_a_dumpear->pid, true, socket_cliente);
				}
				break;

			case LINUS_TORVALDS:
				log_error(logger_memoria, "LINUS TORVALD TE MALDIGO");
				log_error(logger_memoria, "el cliente se desconecto.");
				eliminar_paquete(paquete);
				return EXIT_FAILURE;
				break;
			default:
				log_warning(logger_memoria, "Operacion desconocida. No quieras meter la pata");
				break;
		}
		// Liberar memoria del paquete recibido
		eliminar_paquete(paquete);
	}

	return EXIT_SUCCESS;
}
