

#include "./cpu-utils.h"
#include <math.h>

// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_instruccion_a_memoria(t_peticion_instruccion* infoPCB){
	
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");

	//Serializa la petición
	int size_peticion = 0;
	void* peticion_serializada = serializar_peticion_instruccion(infoPCB, &size_peticion);
    if(peticion_serializada == NULL) {
        log_warning(logger_cpu, "Error al serializar la peticion de instruccion");
        return;
	}
	
	//Envía la peticion serializada a MEMORIA
	log_info(logger_cpu, "Size_peticion= %d", size_peticion);
	log_debug(logger_cpu, "Petición envíada, aguardo respuesta");
	send(fd_conexion_memoria, peticion_serializada, size_peticion, 0);
	free(peticion_serializada);
}	
	

void manejar_respuesta_de_instruccion(t_paquete* paquete){

	//Deserializa la instrucción recibida
	t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(paquete->buffer->stream);
	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	instruccion = decode(respuesta->instruccion);
	if (!instruccion) {
        log_error(logger_cpu, "Error al decodificar la instrucción");
        // No olvides liberar respuesta->instruccion antes de salir
        free(respuesta->instruccion);
        free(respuesta);
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return;  // Finaliza la función si no se pudo decodificar la instrucción
    }
	log_debug(logger_cpu, "Instrucción decodificada: %d", instruccion->tipo);
	
	ejecutar_instruccion(instruccion);
	//ejecutar_instruccion(instruccion);
	//Se chequea luego de excecute, si llego una interrupcion.


	log_trace(logger_cpu, "Valor de sem_cpu: %d", valor_sem_cpu);
	pthread_t hilo_interrupt;
	pthread_create(&hilo_interrupt, NULL, (void*)check_interrupt, NULL);
	pthread_detach(hilo_interrupt);
	
	log_trace(logger_cpu, "Valor de sem_cpu: %d", valor_sem_cpu);

	// si hay interrupcion, se detiene la ejecucion del proceso actual y se envia a kernel el proceso desalojado.
	// si no hay interrupcion, se pide la siguiente instruccion a memoria.
	
	
	free_instruccion(instruccion);	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(respuesta->instruccion);
	free(respuesta);
}

void _crear_conexion_kernel_interrupt(char* ip_kernel, char* puerto_kernel_interrupt, char* cpu_id) 
{ 
	fd_conexion_kernel_interrupt = crear_conexion(ip_kernel, puerto_kernel_interrupt);
	
	if(fd_conexion_kernel_interrupt == -1){
		log_error(logger_cpu, "Error al iniciar conexion de interrupt");
		abort();
	}

	_handshake_kernel_con_cpu_id(fd_conexion_kernel_interrupt, cpu_id);
}

void _crear_conexion_kernel_dispatch(char* ip_kernel, char* puerto_kernel_dispatch, char* cpu_id) 
{ 
	fd_conexion_kernel_dispatch = crear_conexion(ip_kernel, puerto_kernel_dispatch);

	if(fd_conexion_kernel_dispatch == -1){
        log_error(logger_cpu, "Error al iniciar conexion de dispatch");
        abort();
    }

	_handshake_kernel_con_cpu_id(fd_conexion_kernel_dispatch, cpu_id);
}

void _crear_conexion_cpu_memoria(char* ip_memoria, char* puerto_memoria) { 
	fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	
	if(fd_conexion_memoria == -1){
		log_error(logger_cpu, "Error al iniciar conexion de MEMORIA");
		abort();
	}
	
	pedir_datos("Te saludo desde el modulo [[CPU]]", fd_conexion_memoria);
}

void _handshake_kernel_con_cpu_id(int fd_conexion, char* cpu_id) {
	size_t bytes;

	uint8_t handshake = atoi(cpu_id);
	uint8_t result;

	bytes = send(fd_conexion, &handshake, sizeof(uint8_t), 0);
	bytes = recv(fd_conexion, &result, sizeof(uint8_t), MSG_WAITALL);

	if (result == 0) {
    	log_debug(logger_cpu, "Handshake con [KERNEL] exitoso!");
	} else {
		log_error(logger_cpu, "Handshake con [KERNEL] fallido!");
		abort();
	}
}

void check_interrupt(){
	sem_wait(&sem_cpu);

	if(flag_interrupt){
		log_warning(logger_cpu, "Interrupcion detectada, se procede a desalojar el proceso actual");
		// Enviar el proceso desalojado a kernel
		flag_interrupt = false; 
		sem_post(&sem_cpu_kernel);
		enviar_proceso_desalojado(fd_conexion_kernel_interrupt, pcb_actual->pid, pcb_actual->pc);
	
	} else {
		log_info(logger_cpu, "No hay interrupciones, se procede a pedir la siguiente instrucción a memoria");
		pedir_instruccion_a_memoria(pcb_actual);
	}
	
}


void enviar_proceso_desalojado(int socket_servidor, int pid, int pc) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(int) + sizeof(int);
	buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(int)); offset += sizeof(int);
    memcpy(buffer->stream + offset, &pc, sizeof(int)); offset += sizeof(int);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_DESALOJADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));   offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_servidor, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);

}
//FUNCIONES DE MMU
mmu_t* inicializar_mmu(){
    mmu_t* mmu = malloc(sizeof(mmu_t));
    mmu->tamanio_pagina = 0;
    mmu->cantidad_entradas_tabla = 0; 
    mmu->cantidad_niveles = 0; 
	mmu->ultima_direccion_fisica_calculada = 0;
    log_info(logger_cpu, "MMU inicializada");
    return mmu;
}

void destruir_mmu(mmu_t* mmu) {
    free(mmu);  // no hay punteros internos, solo liberar la struct
}

void recibir_datos_de_memoria(mmu_t* mmu) {
    uint32_t buffer_size;
    // Recibir tamaño del buffer
    if (recv(fd_conexion_memoria, &buffer_size, sizeof(uint32_t), MSG_WAITALL) <= 0) {
        log_error(logger_cpu, "Error al recibir el tamaño del buffer");
        return;
    }

    // Recibir el contenido del buffer
    void* buffer_stream = malloc(buffer_size);
    if (recv(fd_conexion_memoria, buffer_stream, buffer_size, MSG_WAITALL) <= 0) {
        log_error(logger_cpu, "Error al recibir el contenido del buffer");
        free(buffer_stream);
        return;
    }

    // Ahora sí, deserializá los datos
    if (buffer_size < sizeof(uint32_t) * 3) {
        log_error(logger_cpu, "El tamaño del buffer es insuficiente para deserializar los datos de memoria");
        free(buffer_stream);
        return;
    }
    void* stream = buffer_stream;

    memcpy(&(mmu->tamanio_pagina), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(mmu->cantidad_niveles), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(mmu->cantidad_entradas_tabla), stream, sizeof(uint32_t));

    log_info(logger_cpu, "Datos de memoria recibidos: tamanio_pagina=%d, cantidad_niveles=%d, cantidad_entradas_tabla=%d",
             mmu->tamanio_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);
			 
	free(buffer_stream);
}

//FUNCIONES DE DIRECCIONES
//Traduce una direccion logica de W/R a sus componentes utiles para calcular la direccion fisica
t_pre_direccion_fisica calcular_pre_direccion_fisica(int direccion_logica) {
    t_pre_direccion_fisica resultado;
	
	uint32_t  TAMANIO_PAGINA = mmu->tamanio_pagina;
	uint32_t  CANT_NIVELES = mmu->cantidad_niveles;
	uint32_t  CANT_ENTRADAS_TABLA = mmu->cantidad_entradas_tabla;
	log_debug(logger_cpu,"Tamanio Pagina = %d", TAMANIO_PAGINA);
	log_debug(logger_cpu,"Cantdad niveles = %d", CANT_NIVELES);
	log_debug(logger_cpu,"entradas tabla = %d", CANT_ENTRADAS_TABLA);
	log_debug(logger_cpu, "direccion logica = %d", direccion_logica);
	
    resultado.nro_pagina = direccion_logica / TAMANIO_PAGINA;
    resultado.desplazamiento = direccion_logica % TAMANIO_PAGINA;

	// Reservar memoria dinámica para el array de niveles
    resultado.entrada_nivel = malloc(sizeof(int) * CANT_NIVELES);
	if (resultado.entrada_nivel == NULL) {
		log_error(logger_cpu, "Error al reservar memoria para las entradas de nivel");
		exit(EXIT_FAILURE);
	}
	//Teniendo una cantidad de niveles N y un identificador X de cada nivel podemos utilizar las siguientes fórmulas:
	//entrada_nivel_X = floor(nro_página  / cant_entradas_tabla ^ (N - X)) % cant_entradas_tabla
    for (int i = 0; i < CANT_NIVELES; i++) {
        int divisor = (int)pow(CANT_ENTRADAS_TABLA, CANT_NIVELES - (i + 1));
        resultado.entrada_nivel[i] = (resultado.nro_pagina / divisor) % CANT_ENTRADAS_TABLA;
    }
	//Devuelve nro de pagina, desplazamiento y entradas de cada nivel
	log_debug(logger_cpu, "Dirección lógica %d traducida a dirección física: Página %d, Desplazamiento %d", 
			  direccion_logica, resultado.nro_pagina, resultado.desplazamiento);
    return resultado;
}

// Calcula la direccion fisica real para una procesos con su pid y marco + desplazamiento
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica){
	uint32_t direccion_fisica_final = (marco * mmu->tamanio_pagina) + pre_direccion_fisica.desplazamiento;
	log_debug(logger_cpu, "Dirección física final calculada: %d", direccion_fisica_final);
	return direccion_fisica_final;
}

//FUNCIONES DE TLB

tlb_t* inicializar_tlb(uint32_t maximas_entradas_tlb) {
	tlb_t* tlb = malloc(sizeof(tlb_t));
	tlb->cantidad_entradas = maximas_entradas_tlb;
	tlb->entradas = malloc(sizeof(entradas_tlb_t) * tlb->cantidad_entradas);
	for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
		tlb->entradas[i].bit_en_uso = 0;
	}
	log_info(logger_cpu, "TLB inicializada, CANTIDAD DE ENTRADAS <%d>", tlb->cantidad_entradas);
	return tlb;
}

void destruir_tlb(tlb_t* tlb) {
    if (!tlb) return;

    free(tlb->entradas);  // liberar array de entradas
    free(tlb);            // liberar estructura completa
}

void agregar_a_tlb(uint32_t nro_pagina_entrante, uint32_t marco_asociado_entrante, algoritmo_tlb_t algoritmo){
	
 	for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
        if (tlb->entradas[i].bit_en_uso == 0) {
            tlb->entradas[i].nro_pagina = nro_pagina_entrante;
            tlb->entradas[i].marco_asociado = marco_asociado_entrante;
			tlb->entradas[i].bit_en_uso = 1; 
            log_info(logger_cpu, "Agregada a TLB: Pagina <%d> - Marco <%d> en posicion <%d>", nro_pagina_entrante, marco_asociado_entrante, i);
            log_warning(logger_cpu, "TLB: bit en uso <%d> - Marco <%d> en numero de pagina <%d>", tlb->entradas[i].bit_en_uso, tlb->entradas[i].marco_asociado, 
						tlb->entradas[i].nro_pagina);

			return;
        }
	}
	switch (algoritmo)
	{
	case FIFO:
        agregar_a_tlb_fifo(nro_pagina_entrante, marco_asociado_entrante);

		break;
	case LRU:
	    agregar_a_tlb_lru(nro_pagina_entrante, marco_asociado_entrante);

		break;
	default:
			log_error(logger_cpu, "Algoritmo re raro che....");
		break;
	}
    
}
void limpiar_tlb(){
	for(uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
		tlb->entradas[i].bit_en_uso = 0;

	}
}
int esta_en_tlb(uint32_t nro_pagina){
	int marco_asociado = 0;
	for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
		if ((tlb->entradas[i].nro_pagina == nro_pagina) && tlb->entradas[i].bit_en_uso == 1) {
			marco_asociado = tlb->entradas[i].marco_asociado;
			log_info(logger_cpu, "TLB HIT: PID <%d> - TLB HIT - Pagina: <%d>",pcb_actual->pid, nro_pagina);
			// TLB Hit: “PID: <PID> - TLB HIT - Pagina: <NUMERO_PAGINA>”

			return marco_asociado;
		}
	}
	log_info(logger_cpu, "TLB MISS: Página %d no encontrada en TLB", nro_pagina);
	return -1;
}
uint32_t tlb_miss(t_pre_direccion_fisica pre_direccion_fisica) {
	// TLB Miss: “PID: <PID> - TLB MISS - Pagina: <NUMERO_PAGINA>”
	uint32_t marco_correspondiente;
	t_paquete* paquete_solicitud_marco;
	void* a_enviar_peticion_marco;
	uint32_t bytes = 0;
 			paquete_solicitud_marco = crear_paquete_con_codigo(OBTENER_MARCO_CORRESPONDIENTE);
            agregar_a_paquete(paquete_solicitud_marco, &(pcb_actual->pid), sizeof(uint32_t));
            agregar_a_paquete(paquete_solicitud_marco, &pre_direccion_fisica.nro_pagina, sizeof(uint32_t));
            //agregar_a_paquete(paquete_solicitud_marco, &pre_direccion_fisica.desplazamiento, sizeof(uint32_t));
            for (int i = 0; i < mmu->cantidad_niveles; i++)
                agregar_a_paquete(paquete_solicitud_marco, &pre_direccion_fisica.entrada_nivel[i], sizeof(uint32_t)); 
            bytes = paquete_solicitud_marco->buffer->size + 2*sizeof(int);
            a_enviar_peticion_marco = serializar_paquete(paquete_solicitud_marco, bytes);
            send(fd_conexion_memoria, a_enviar_peticion_marco, bytes, 0);

            free(a_enviar_peticion_marco);
            eliminar_paquete(paquete_solicitud_marco);

            recv(fd_conexion_memoria, &marco_correspondiente, sizeof(uint32_t), MSG_WAITALL);

			agregar_a_tlb(pre_direccion_fisica.nro_pagina, marco_correspondiente, algoritmo);

	return marco_correspondiente;
}

algoritmo_tlb_t algoritmo_from_string(const char* str) {
    if (strcmp(str, "FIFO") == 0) return FIFO;
    if (strcmp(str, "LRU") == 0) return LRU;
    return EXIT_FAILURE;
}

 void agregar_a_tlb_fifo(uint32_t nro_pagina_entrante, uint32_t marco_asociado_entrante) {
    tlb->entradas[proxima_a_reemplazar].nro_pagina = nro_pagina_entrante;
    tlb->entradas[proxima_a_reemplazar].marco_asociado = marco_asociado_entrante;
    tlb->entradas[proxima_a_reemplazar].bit_en_uso = 1;
    tlb->entradas[proxima_a_reemplazar].instante_referencia = ++contador_accesos_tlb;
    log_info(logger_cpu, "TLB llena. Reemplazo FIFO en posicion <%d>: Pagina <%d> - Marco <%d>", proxima_a_reemplazar, nro_pagina_entrante, marco_asociado_entrante);
    proxima_a_reemplazar = (proxima_a_reemplazar + 1) % tlb->cantidad_entradas;
}

 void agregar_a_tlb_lru(uint32_t nro_pagina_entrante, uint32_t marco_asociado_entrante) {
    uint64_t min_ref = tlb->entradas[0].instante_referencia;
    int indice_lru = 0;
    for (uint32_t i = 1; i < tlb->cantidad_entradas; i++) {
        if (tlb->entradas[i].instante_referencia < min_ref) {
            min_ref = tlb->entradas[i].instante_referencia;
            indice_lru = i;
        }
    }
    tlb->entradas[indice_lru].nro_pagina = nro_pagina_entrante;
    tlb->entradas[indice_lru].marco_asociado = marco_asociado_entrante;
    tlb->entradas[indice_lru].bit_en_uso = 1;
    tlb->entradas[indice_lru].instante_referencia = ++contador_accesos_tlb;
    log_info(logger_cpu, "TLB llena. Reemplazo LRU en posicion <%d>: Pagina <%d> - Marco <%d>", 
            indice_lru, nro_pagina_entrante, marco_asociado_entrante);
}

// FUNCIONES DE CACHE
//Inicializo cache
t_memoria_cache* inicializar_cache(uint32_t cant_paginas, uint32_t tam_pagina) {
    t_memoria_cache* cache = malloc(sizeof(t_memoria_cache));
    cache->cantidad_paginas = cant_paginas;
    cache->paginas = malloc(sizeof(t_pagina_de_cache) * cant_paginas);

    for (uint32_t i = 0; i < cant_paginas; i++) {
        cache->paginas[i].nro_pagina = -1; // -1 significa que está libre
        cache->paginas[i].contenido = malloc(tam_pagina);
        cache->paginas[i].bit_uso = false;
        cache->paginas[i].bit_modificado = false;
    }

    return cache;
}

void destruir_cache(t_memoria_cache* cache, uint32_t tam_pagina) {
    if (!cache) return;

    for (uint32_t i = 0; i < cache->cantidad_paginas; i++) {
        free(cache->paginas[i].contenido);  // liberar el contenido de cada página
    }

    free(cache->paginas);  // liberar array de páginas
    free(cache);           // liberar la estructura completa
}

int buscar_pagina_en_cache(t_memoria_cache* cache, int nro_pagina_buscado) {
    for (int i = 0; i < cache->cantidad_paginas; i++) {
        if (cache->paginas[i].nro_pagina == nro_pagina_buscado) {
            cache->paginas[i].bit_uso = true;

            log_info(logger_cpu, "PID: %d - Cache Hit - Pagina: %d", pcb_actual->pid, nro_pagina_buscado);
            return i; // devuelve el índice de esa página dentro del array
        }
    }

    log_info(logger_cpu, "PID: %d - Cache Miss - Pagina: %d", pcb_actual->pid, nro_pagina_buscado);
    return -1; // no se encontró
}

//ACA SE HACE EL WRITE DE TODAS LAS PAGINAS DEL CACHE EN LA MEMORIA PRINCIPAL
// void actualizar_memoria_principal(){


// 	direccion_logica = atoi(instruccion->parametros.write.direccion);
//             pre_direccion_fisica = calcular_pre_direccion_fisica(direccion_logica);

//             //CACHE

//             //TLB
//             aux_tlb = esta_en_tlb(pre_direccion_fisica.nro_pagina);

//             if (aux_tlb != -1)
//                 marco_correspondiente = aux_tlb;
//             else
//                 marco_correspondiente = tlb_miss(pre_direccion_fisica);
         
//             direccion_fisica_final = calcular_direccion_fisica_final(marco_correspondiente, pre_direccion_fisica);
//             // Mandamos la ejecución con la dirección física final
//             t_paquete* paquete_write = crear_paquete_con_codigo(WRITE_MEMORIA);
//             agregar_a_paquete(paquete_write, &(pcb_actual->pid), sizeof(uint32_t));
//             agregar_a_paquete(paquete_write, &direccion_fisica_final, sizeof(uint32_t));
//             agregar_a_paquete(paquete_write, instruccion->parametros.write.datos, strlen(instruccion->parametros.write.datos) + 1);
//             bytes = paquete_write->buffer->size + 2*sizeof(int);

//             void* a_enviar_write = serializar_paquete(paquete_write, bytes);
//             mmu->ultima_direccion_fisica_calculada = direccion_fisica_final; 
//             if (ultima_escritura) 
//                 free(ultima_escritura);
//             ultima_escritura = strdup(instruccion->parametros.write.datos);
//             send(fd_conexion_memoria, a_enviar_write, bytes, 0);

//             free(pre_direccion_fisica.entrada_nivel);
//             free(a_enviar_write);
//             eliminar_paquete(paquete_write);
            
//             log_info(logger_cpu, "WRITE enviado a Memoria. Esperando respuesta...");

//             sem_post(&sem_write);

// }