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
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				free(paquete);
				break;
			case CPU_PIDE_DATOS:
				log_info(logger_memoria, "Recibí petición de datos desde CPU");
				recibir_mensaje(socket_cliente, logger_memoria);
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				enviar_datos_a_cpu(socket_cliente);
				log_info(logger_memoria, "Datos enviados a CPU");
				free(paquete);
				break;
			case PROCESO_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				pthread_mutex_lock(&memoria_del_sistema->mutex);
				
				recibir_paquete(socket_cliente, paquete);
			
				t_pcbMemoria* proceso_a_inicializar = deserializarProceso(paquete->buffer);
				log_trace(logger_memoria, "PID recibido para inicializar: %d", proceso_a_inicializar->pid);

				//log_debug(logger_memoria, "Cantidad de memoria antes: %d", cantMemoria);
				cantMemoria -= proceso_a_inicializar->tamanioMemoria;
				log_warning(logger_memoria, "Cantidad de memoria restante: %d", cantMemoria);
				
				if(cantMemoria < 0) {
					
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

				pthread_mutex_unlock(&memoria_del_sistema->mutex);

				// Liberar memoria del paquete recibido
				eliminar_paquete(paquete);
				
				break; 
		
			case PROCESO_SUSPENDIDO_MEMORIA:
				
				pthread_mutex_lock(&memoria_del_sistema->mutex);

				//RETARDO DE MEMORIA
				usleep( atoi(config_memoria->RETARDO_SWAP) * 1000);
				recibir_paquete(socket_cliente, paquete);
			
				t_pcbMemoria* proceso_suspendido = deserializarProceso(paquete->buffer);

				cantMemoria += proceso_suspendido->tamanioMemoria;

				// Mover proceso de RAM a SWAP

				avisar_kernel_mande_otro_proceso(socket_cliente);

				pthread_mutex_unlock(&memoria_del_sistema->mutex);

				break;

			case PROCESO_FINALIZAR:
				
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);

				recibir_paquete(socket_cliente, paquete);

				t_pcbMemoria* proceso_a_finalizar = deserializarProceso(paquete->buffer);
 
				int pidParaEliminar = proceso_a_finalizar->pid;
				log_warning(logger_memoria, "PID recibido para finalizar: %d", pidParaEliminar);
				int pidEliminado = finalizar_proceso(pidParaEliminar);

				cantMemoria += proceso_a_finalizar->tamanioMemoria;
				log_warning(logger_memoria, "el tamanio de la memo es ahora: %d", cantMemoria);

				log_info(logger_memoria, "Se elimino el proceso con PID: %d de memoria", pidEliminado);
				//enviar_proceso_terminado("NO FINALIZA EL PROCESO :(", socket_cliente);
				
				t_paquete* paquete_proceso_eliminado = malloc(sizeof(t_paquete));
				crear_buffer(paquete_proceso_eliminado);
				
				enviar_proceso_terminado(pidEliminado, paquete_proceso_eliminado, socket_cliente);
			
				break; 
			case INSTRUCCION:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);

				log_info(logger_memoria, "Recibi la petición de instruccion desde CPU");
				t_paquete* paquete_tmp = recibir_paquete_instruccion(socket_cliente);
				if (paquete_tmp == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de instrucción");
					break;
				}
				//	log_info(logger_memoria, "Tamaño del buffer recibido: %d", paquete_tmp->buffer->size);
				manejar_peticion_de_instruccion(socket_cliente, paquete_tmp);

				// Libera el paquete
				eliminar_paquete(paquete_tmp);

				eliminar_paquete(paquete);
				break;

			case OBTENER_MARCO_CORRESPONDIENTE:
				//RETARDO DE MEMORIA

				usleep( retardo_memoria * atoi(config_memoria->CANTIDAD_NIVELES) * 1000);

				log_info(logger_memoria, "Recibí solicitud de ACCESO A TABLA DE PAGINAS	");
				t_paquete* paquete_marco= recibir_paquete_instruccion(socket_cliente);
				if (!paquete_marco) {
					log_error(logger_memoria, "Error al recibir paquete de traducción");
					break;
				}
				manejar_acceso_tablas_de_paginas(socket_cliente, paquete_marco);
				eliminar_paquete(paquete_marco);
				eliminar_paquete(paquete);
				break;

			case WRITE_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de WRITE");
				t_paquete* paquete_write = recibir_paquete_instruccion(socket_cliente);
				if (paquete_write == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de WRITE");
					break;
				}
				manejar_escritura_memoria(socket_cliente, paquete_write);

				eliminar_paquete(paquete_write);
				eliminar_paquete(paquete);
				break;

			case READ_MEMORIA:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de READ");
				t_paquete* paquete_read = recibir_paquete_instruccion(socket_cliente);
				if (paquete_read == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de READ");
					break;
				}
				manejar_lectura_memoria(socket_cliente, paquete_read);
			
				eliminar_paquete(paquete_read);
				eliminar_paquete(paquete);
				break;

			case PROCESO_DUMPEAR:
				//RETARDO DE MEMORIA
				usleep( retardo_memoria * 1000);
				log_info(logger_memoria, "Recibí paquete de ejecución de DUMP_MEMORY");
				recibir_paquete(socket_cliente, paquete);
				if (paquete == NULL) {
					log_error(logger_memoria, "Fallo al recibir paquete de DUMP_MEMORY");
					break;
				}

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
				eliminar_paquete(paquete);
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

void manejar_peticion_de_instruccion(int socket_cliente, t_paquete* paquete) {
    if (paquete->buffer->size < sizeof(int) * 2) {
        log_error(logger_memoria, "El tamaño del buffer es insuficiente para deserializar la instrucción");
        return;
    }

    t_peticion_instruccion* peticion = deserializar_peticion_instruccion(paquete->buffer->stream);

    log_info(logger_memoria, "PID recibido: %d", peticion->pid);
    log_info(logger_memoria, "PC recibido: %d", peticion->pc);

	//Obtengo la instruccion correspondiente al PID y PC recibido de cpu
	t_respuesta_instruccion* respuesta = malloc(sizeof(t_respuesta_instruccion));
	respuesta->instruccion = string_duplicate(obtener_instruccion(peticion->pid, peticion->pc));
	//Entra a este if cuando el pc es mayor a cant de instrucciones
	if(strcmp(respuesta->instruccion, "PC FINALIZADO")== 0){
	
		log_info(logger_memoria, "No hay más instrucciones a ejecutar para este proceso");
		log_debug(logger_memoria, "Serializando paquete:");
		log_debug(logger_memoria, "Código de operación: %d", FIN_PID);

		t_paquete* paquete = malloc(sizeof(t_paquete));

		paquete->codigo_operacion = FIN_PID;
		int bytes = sizeof(int);
		void* paquete_pc_fin = malloc(bytes);
		memcpy(paquete_pc_fin, &(paquete->codigo_operacion), sizeof(int));
		log_debug(logger_memoria, "Tamaño del buffer: %d", bytes);
		send(socket_cliente, paquete_pc_fin, bytes, 0);

		free(paquete->buffer);
		free(paquete_pc_fin);
		free(paquete);
		return;
	}

	//Entra aca si Memoria del sistema no incializada o pid no encontrado
	else if(strcmp(respuesta->instruccion, "NULL")== 0){
	 	log_error(logger_memoria, "pid no encontrado o memoria_del_sistema/procesos no están inicializados");
		return;
	} 

	//Entra acá si encontro el proceso y la instrucción
	else{
		log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s",peticion->pid, peticion->pc, respuesta->instruccion);
		//Serializo la respuesta
		int size_respuesta;
		void* respuesta_serializada = serializar_respuesta_instruccion(respuesta, &size_respuesta);
		if(respuesta_serializada == NULL) {
			log_warning(logger_memoria, "Error al serializar la respuesta de instruccion");
			return;
		}
		log_debug(logger_memoria, "Serializando paquete:");
		log_debug(logger_memoria, "Código de operación: %d", INSTRUCCION);
		log_debug(logger_memoria, "Tamaño del buffer: %ld", size_respuesta - sizeof(op_code) - sizeof(uint32_t));
		log_debug(logger_memoria, "Instrucción: %s", respuesta->instruccion);

		//Envio la instruccion serializada envio a CPU 
		//log_info(logger, "Size_respuesta= %d", size_respuesta);
		log_info(logger_memoria, "Enviando Instrucción a CPU");
		int bytes_enviados = send(socket_cliente, respuesta_serializada, size_respuesta, 0);
		if (bytes_enviados <= 0) {
			log_error(logger_memoria, "Fallo al enviar la instrucción al CPU");
		}
	}

	//Libero memoria
    free(peticion);
	log_trace(logger_memoria, "se llega a los frees");
	free(respuesta->instruccion);
	free(respuesta);
}

void manejar_escritura_memoria(int socket_cliente, t_paquete* paquete) {
    //t_buffer* buffer = paquete->buffer;
	
	//Deserializo el paquete:
    int offset = 0;
	uint32_t pid = leer_uint32_desde_buffer(paquete->buffer, &offset);
	uint32_t direccion_fisica = leer_uint32_desde_buffer(paquete->buffer, &offset);
	char* datos = leer_string_desde_buffer(paquete->buffer, &offset);

	memcpy(memoria_del_sistema->memoria_principal + direccion_fisica, datos, strlen(datos));
	// Log obligatorio
	log_trace(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d ", pid, direccion_fisica);
	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesWrite++;
    // Enviar confirmación de éxito al cpu
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
	log_info(logger_memoria, "Enviando confirmación de WRITE a CPU: %s", mensaje_confirmacion_write);

	free(datos);
	free(a_enviar_write);
	eliminar_paquete(paquete_confirmacion_write);
}

void manejar_lectura_memoria(int socket_cliente, t_paquete* paquete) {

    //Deserializo el paquete:
    int offset = 0;
	uint32_t pid = leer_uint32_desde_buffer(paquete->buffer, &offset);
	uint32_t direccion_fisica = leer_uint32_desde_buffer(paquete->buffer, &offset);
	uint32_t tamanio_a_leer = leer_uint32_desde_buffer(paquete->buffer, &offset);

    log_info(logger_memoria, "[MEMORIA] READ recibido - Dirección: %d | Tamaño: %d", direccion_fisica, tamanio_a_leer);

	void* datos_leidos = malloc(tamanio_a_leer);
	memcpy(datos_leidos, memoria_del_sistema->memoria_principal + direccion_fisica, tamanio_a_leer);
	char* datos_como_string = calloc(tamanio_a_leer + 1, sizeof(char));
	memcpy(datos_como_string, datos_leidos, tamanio_a_leer);
	// Log obligatorio
	log_trace(logger_memoria, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tamanio_a_leer);
	log_trace(logger_memoria, "Contenido leído: %s", datos_como_string);
	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesWrite++;
    // Enviar confirmación de éxito al cliente (reemplazar luego por lógica de lectura en memoria)
	t_paquete* paquete_confirmacion_read = malloc(sizeof(t_paquete));
	paquete_confirmacion_read->codigo_operacion = READ_MEMORIA;
	paquete_confirmacion_read->buffer = malloc(sizeof(t_buffer));
	char* mensaje_confirmacion_read = datos_como_string;
	paquete_confirmacion_read->buffer->size = strlen(mensaje_confirmacion_read) + 1;
	paquete_confirmacion_read->buffer->stream = malloc(paquete_confirmacion_read->buffer->size);
	memcpy(paquete_confirmacion_read->buffer->stream, mensaje_confirmacion_read, paquete_confirmacion_read->buffer->size);

	int bytes_confirmacion_read = paquete_confirmacion_read->buffer->size + 2 * sizeof(int);
	void* a_enviar_read = serializar_paquete(paquete_confirmacion_read, bytes_confirmacion_read);
	send(socket_cliente, a_enviar_read, bytes_confirmacion_read, 0);
	log_info(logger_memoria, "Enviando lectura de READ a CPU: %s", mensaje_confirmacion_read);


	free(a_enviar_read);
	free(datos_leidos);
	free(datos_como_string);
	eliminar_paquete(paquete_confirmacion_read);
}

void manejar_acceso_tablas_de_paginas(int socket_cliente, t_paquete* paquete) {
	
	//deserializo el paquete pid, nro pagina, entradas por nivel
	int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);
    //int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    //int tam_pagina = atoi(config_memoria->TAM_PAGINA);

    int desplazamiento = 0;
    uint32_t pid = leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);
	t_pre_direccion_fisica direccion;
    direccion.nro_pagina= leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);
    //uint32_t despl = leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);


    direccion.entrada_nivel = malloc(sizeof(uint32_t) * cantidad_niveles);

    for (int i = 0; i < cantidad_niveles; i++){
        direccion.entrada_nivel[i] = leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);
	}
	log_debug(logger_memoria, "Iniciando busqueda de marco de pagina %d para proceso PID: %d", direccion.nro_pagina, pid);

    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "PID %d no encontrado en memoria", pid);
        return;
    }

    uint32_t marco = buscar_marco_en_tabla(proceso->tabla_primera, direccion.entrada_nivel, cantidad_niveles);
    if (marco == -1) {
        log_error(logger_memoria, "No se pudo encontrar el marco solicitado de la pagina %d para PID %d",direccion.nro_pagina, pid);
        return;
    }
	
    log_trace(logger_memoria, "PID: %d - Página: %d - Marco: %d", pid, direccion.nro_pagina, marco);

	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesTP += config_memoria->CANTIDAD_NIVELES;

    send(socket_cliente, &marco, sizeof(uint32_t), 0);
	free(direccion.entrada_nivel);
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

void enviar_proceso_terminado(uint8_t pid, t_paquete* paquete, int socket_cliente) {
	int bytes=	0;
    paquete->codigo_operacion = PROCESO_FINALIZADO;
    agregar_a_paquete(paquete, &pid, sizeof(uint8_t));
    bytes = sizeof(int)+ sizeof(int) + paquete->buffer->size;
    void* paquete_exit = serializar_paquete(paquete, bytes);
            
    send(socket_cliente, paquete_exit, bytes, 0);

    free(paquete_exit);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_pcb* recibir_proceso_a_dumpear_desde_kernel(t_buffer* buffer) 
{
	t_pcb* proceso_a_dumpear = malloc(sizeof(t_pcb));

	void* stream = buffer->stream;

    memcpy(&(proceso_a_dumpear->pid), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);

    return proceso_a_dumpear;
}

void enviar_respuesta_dump_memory(uint8_t pid, bool respuesta, int socket_cliente) {

	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size =  sizeof(uint8_t) + sizeof(bool);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &respuesta, sizeof(bool)); offset += sizeof(bool);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = RESPUESTA_DUMPEO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));   offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

bool realizar_dump_memory(int pid) {
	// Buscamos proceso a dumpear
    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "DUMP_MEMORY: proceso %d no registrado en memoria", pid);
        return false;
    }

	int tam_pagina = atoi(config_memoria->TAM_PAGINA);
    int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);

    // Creo directorio
    char* dump_path = config_memoria->DUMP_PATH;
    int resultado = mkdir(dump_path, 0755);
	if (resultado == 0) {
        log_trace(logger_memoria, "DUMP_MEMORY: directorio creado con éxito");
    } else {
        log_trace(logger_memoria, "DUMP_MEMORY: el directorio ha sido creado anteriormente");
    }

    // Creo el timestamp para el nombre del archivo a dumpear
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", &tm);

    // Armamos nombre completo del timestamp anashei
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%d-%s.dmp", dump_path, pid, timestamp);

    // Abrimos puerta al nether
    FILE* file = fopen(filename, "w+b");
    if (!file) {
        log_error(logger_memoria, "DUMP_MEMORY: no pudo abrirse el archivo del dump %s", filename);
        return false;
    }

	// Recorrer páginas y volcar su contenido
    int paginas_totales = proceso->tamanioMemoria / tam_pagina;

    for (int nro_pagina = 0; nro_pagina < paginas_totales; nro_pagina++) {
		uint32_t* entradas_por_nivel = calcular_entradas_por_nivel(nro_pagina, cantidad_niveles, entradas_por_tabla);
		if (!entradas_por_nivel) {
			log_error(logger_memoria, "Error al calcular las entradas por nivel");
			break;
		}

        int marco = buscar_marco_en_tabla(proceso->tabla_primera, entradas_por_nivel, cantidad_niveles);
		free(entradas_por_nivel);

        if (marco == -1) {
            log_warning(logger_memoria, "Página %d no asignada (se saltea)", nro_pagina);
            char vacio[tam_pagina];
            memset(vacio, 0, tam_pagina);
            fwrite(vacio, 1, tam_pagina, file);  // rellena con ceros si no está asignada
        } else {
            void* origen = memoria_del_sistema->memoria_principal + (marco * tam_pagina);
            fwrite(origen, 1, tam_pagina, file);
        }
    }

    fclose(file);
    log_trace(logger_memoria, "DUMP_MEMORY: archivo creado %s", filename);
    return true;
}

void avisar_kernel_mande_otro_proceso(int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = SUSPENSION_HECHA;
	paquete->buffer = malloc(sizeof(t_buffer));
	
	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_datos_a_cpu(int socket_cliente){
	// Serializar los datos de memoria en el buffer del paquete
	uint32_t tam_pagina = atoi(config_memoria->TAM_PAGINA);
	uint32_t cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);
	uint32_t entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = DATOS_DE_MEMORIA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t) * 3;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	int offset = 0;
	memcpy(paquete->buffer->stream + offset, &tam_pagina, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + offset, &cantidad_niveles, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + offset, &entradas_por_tabla, sizeof(uint32_t));

	int bytes = paquete->buffer->size + sizeof(int) + sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

uint32_t* calcular_entradas_por_nivel(uint32_t nro_pagina, int cantidad_niveles, int entradas_por_tabla) {
    uint32_t* entradas = malloc(sizeof(uint32_t) * cantidad_niveles);
    if (!entradas) return NULL;

    for (int nivel = cantidad_niveles - 1; nivel >= 0; nivel--) {
        entradas[nivel] = nro_pagina % entradas_por_tabla;
        nro_pagina /= entradas_por_tabla;
    }

    return entradas;
}