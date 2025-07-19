

#include "./cpu-utils.h"
#include <math.h>

// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
void pedir_datos_a_memoria(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = crear_paquete_con_codigo(CPU_PIDE_DATOS);
    
	agregar_a_paquete(paquete, mensaje, strlen(mensaje) + 1);

    enviar_paquete(paquete, socket_cliente);

	eliminar_paquete(paquete);
}

void recibir_datos_de_memoria(t_paquete* paquete, mmu_t* mmu) {
    int offsett = sizeof(uint32_t);
    memcpy(&(mmu->tamanio_pagina), paquete->buffer->stream + offsett, sizeof(uint32_t)); offsett += sizeof(uint32_t)*2;
    memcpy(&(mmu->cantidad_niveles), paquete->buffer->stream + offsett, sizeof(uint32_t)); offsett += sizeof(uint32_t)*2;
    memcpy(&(mmu->cantidad_entradas_tabla), paquete->buffer->stream + offsett, sizeof(uint32_t));

    log_info(logger_cpu, "Datos de memoria recibidos: tamanio_pagina=%d, cantidad_niveles=%d, cantidad_entradas_tabla=%d",
          mmu->tamanio_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);

    // uint32_t buffer_size;
    // // Recibir tamaño del buffer
    // if (recv(fd_conexion_memoria, &buffer_size, sizeof(uint32_t), MSG_WAITALL) <= 0) {
    //     log_error(logger_cpu, "Error al recibir el tamaño del buffer");
    //     return;
    // }

    // // Recibir el contenido del buffer
    // void* buffer_stream = malloc(buffer_size);
    // if (recv(fd_conexion_memoria, buffer_stream, buffer_size, MSG_WAITALL) <= 0) {
    //     log_error(logger_cpu, "Error al recibir el contenido del buffer");
    //     free(buffer_stream);
    //     return;
    // }

    // // Ahora sí, deserializá los datos
    // if (buffer_size < sizeof(uint32_t) * 3) {
    //     log_error(logger_cpu, "El tamaño del buffer es insuficiente para deserializar los datos de memoria");
    //     free(buffer_stream);
    //     return;
    // }
    // void* stream = buffer_stream;
    //  memcpy(&(mmu->tamanio_pagina), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    //  memcpy(&(mmu->cantidad_niveles), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    //  memcpy(&(mmu->cantidad_entradas_tabla), stream, sizeof(uint32_t));
    // free(buffer_stream);	 
}

void pedir_instruccion_a_memoria(t_peticion_instruccion* infoPCB){
	
    log_info(logger_cpu, "Iniciando la peticion de instruccion a memoria");

    t_paquete* paquete = crear_paquete_instruccion();
    agregar_a_paquete(paquete, &(infoPCB->pid), sizeof(uint8_t));
    agregar_a_paquete(paquete, &(infoPCB->pc), sizeof(uint16_t));

    if(paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        return;
    }
    //Envía la peticion serializada a MEMORIA
    enviar_paquete(paquete, fd_conexion_memoria);
	log_debug(logger_cpu, "Petición envíada, aguardo respuesta");
    eliminar_paquete(paquete); 
}	
	
void manejar_respuesta_de_instruccion(t_paquete* paquete){

	//Deserializa la instrucción recibida
    int desplazamiento = 0;
	char* respuesta_instruccion = leer_string_desde_buffer(paquete->buffer, &desplazamiento);
	log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta_instruccion);

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	instruccion = decode(respuesta_instruccion);
	if (!instruccion) {
        log_error(logger_cpu, "Error al decodificar la instrucción");
        // No olvides liberar respuesta->instruccion antes de salir
        free_instruccion(instruccion);
        free(respuesta_instruccion);
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
	free(respuesta_instruccion);
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
	//log_info(logger_cpu, "[CONEXION] Conectando a memoria en ip:%s, puerto:%s", ip_memoria, puerto_memoria);
	fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	
	if(fd_conexion_memoria == -1){
		log_error(logger_cpu, "Error al iniciar conexion de MEMORIA");
		abort();
	}
	
	//log_info(logger_cpu, "[CONEXION] Conectado a memoria con FD: %d", fd_conexion_memoria);
	pedir_datos_a_memoria("Te saludo desde el modulo [[CPU]]", fd_conexion_memoria);
}

void _handshake_kernel_con_cpu_id(int fd_conexion, char* cpu_id) {

	uint8_t handshake = atoi(cpu_id);
	uint8_t result;

	send(fd_conexion, &handshake, sizeof(uint8_t), 0);
	recv(fd_conexion, &result, sizeof(uint8_t), MSG_WAITALL);

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

void enviar_proceso_desalojado(int socket_servidor, uint8_t pid, uint16_t pc) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(int) + sizeof(int);
	buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &pc, sizeof(uint16_t)); offset += sizeof(uint16_t);
    
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



void enviar_read_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, uint32_t tamanio){
    //Envia el read a memoria
    t_paquete* paquete_read = crear_paquete_con_codigo(READ_MEMORIA);
    agregar_a_paquete(paquete_read, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete_read, &direccion_fisica_final, sizeof(uint32_t));
    agregar_a_paquete(paquete_read, &tamanio, sizeof(uint32_t));
    enviar_paquete(paquete_read, fd_conexion_memoria);
    log_debug(logger_cpu, "Enviando READ a Memoria: PID=%d, Direccion Fisica Final=%d, Tamanio=%d", pid, direccion_fisica_final, tamanio);
    eliminar_paquete(paquete_read);
}

void enviar_write_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, char* datos){
    // Mandamos la ejecución con la dirección física final
    t_paquete* paquete_write = crear_paquete_con_codigo(WRITE_MEMORIA);
    agregar_a_paquete(paquete_write, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete_write, &direccion_fisica_final, sizeof(uint32_t));
    agregar_a_paquete(paquete_write, datos, strlen(datos) + 1);
    enviar_paquete(paquete_write, fd_conexion_memoria);
    log_debug(logger_cpu, "Enviando WRITE a Memoria: PID=%d, Direccion Fisica Final=%d, Contenido: %s", pid, direccion_fisica_final, datos);
    eliminar_paquete(paquete_write);
}

char* obtener_pagina_de_memoria(uint8_t pid, uint32_t direccion_fisica) {
    // Enviar solicitud a memoria para obtener la página correspondiente
    enviar_read_a_memoria(pid, direccion_fisica, mmu->tamanio_pagina);

    // Esperar respuesta de memoria
    t_paquete* paquete_respuesta = crear_paquete_con_codigo(PAQUETE);
    paquete_respuesta->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
    recibir_buffer_en_paquete(fd_conexion_memoria,paquete_respuesta);
    if (paquete_respuesta == NULL) {
        log_error(logger_cpu, "Error al recibir respuesta de memoria");
        return NULL;
    }

    // Verificar que la respuesta sea válida
    if (paquete_respuesta->codigo_operacion != READ_MEMORIA) {
        log_error(logger_cpu, "Error al obtener página de memoria, código de operación: %d", paquete_respuesta->codigo_operacion);
        eliminar_paquete(paquete_respuesta);
        return NULL;
    }

    // Deserializar contenido de la página
    char* pagina = deserializar_read_o_write_de_memoria(paquete_respuesta);
    eliminar_paquete(paquete_respuesta);
    return pagina;
}
char* recibir_read_o_write_de_memoria() {
    t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);

    // Recibir el código de operación desde la conexión
    paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);

    // Validar tipo de operación recibida
    if (paquete->codigo_operacion != READ_MEMORIA && paquete->codigo_operacion != WRITE_MEMORIA) {
        log_error(logger_cpu, "Error: Código de operación inesperado: %d", paquete->codigo_operacion);
        eliminar_paquete(paquete);
        return NULL;
    }

    // Recibir el buffer
    recibir_buffer_en_paquete(fd_conexion_memoria, paquete);
    log_debug(logger_cpu, "Tamaño del buffer recibido: %d", paquete->buffer->size);

    // Deserializar el contenido
    char* contenido = malloc(paquete->buffer->size + 1);  
    memcpy(contenido, paquete->buffer->stream, paquete->buffer->size);
    contenido[paquete->buffer->size] = '\0'; 

    log_debug(logger_cpu, "Contenido recibido: %s", contenido);

    // Log según el tipo de operación
    if (paquete->codigo_operacion == READ_MEMORIA) {
        log_info(logger_cpu, "READ_MEMORIA recibido. Contenido: %s", contenido);
    } else if (paquete->codigo_operacion == WRITE_MEMORIA) {
        log_info(logger_cpu, "WRITE_MEMORIA recibido. Contenido: %s", contenido);
    }

    eliminar_paquete(paquete);
    return contenido;
}

char* deserializar_read_o_write_de_memoria(t_paquete* paquete){
    char* contenido = malloc(paquete->buffer->size);
    memcpy(contenido, paquete->buffer->stream, paquete->buffer->size);
    return contenido;
}

//FUNCIONES DE DIRECCIONES
//Traduce una direccion logica de W/R a sus componentes utiles para calcular la direccion fisica
uint32_t * calcular_entradas_por_nivel(int nro_pagina, int cantidad_niveles, int cantidad_entradas_tabla) {
    uint32_t *entradas_por_nivel = malloc(sizeof(uint32_t) * cantidad_niveles);
    if (entradas_por_nivel == NULL) {
        log_error(logger_cpu, "Error al reservar memoria para entradas por nivel");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cantidad_niveles; i++) {
        int divisor = (int)pow(cantidad_entradas_tabla, cantidad_niveles - (i + 1));
        entradas_por_nivel[i] = (nro_pagina / divisor) % cantidad_entradas_tabla;
    }
    return entradas_por_nivel;
}

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
    resultado.entrada_nivel = calcular_entradas_por_nivel(resultado.nro_pagina, CANT_NIVELES, CANT_ENTRADAS_TABLA);
	//Devuelve nro de pagina, desplazamiento y entradas de cada nivel
	log_debug(logger_cpu, "Dirección lógica %d traducida a dirección física: Página %d, Desplazamiento %d", direccion_logica, resultado.nro_pagina, resultado.desplazamiento);
    return resultado;
}

// Calcula la direccion fisica real para una procesos con su pid y marco + desplazamiento
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica){
	uint32_t direccion_fisica_final = (marco * mmu->tamanio_pagina) + pre_direccion_fisica.desplazamiento;
	log_debug(logger_cpu, "Dirección física final calculada: %d", direccion_fisica_final);
	return direccion_fisica_final;
}

// Solicita a memoria el marco correspondiente a una pagina y devuelve la direccion fisica
int32_t solicitar_marco_a_memoria(t_pre_direccion_fisica pre_direccion_fisica, uint8_t pid) {

  

    log_debug(logger_cpu, "Solicitando marco a memoria para PID: %d, Página: %d", pid, pre_direccion_fisica.nro_pagina);
    t_paquete* paquete = crear_paquete_con_codigo(OBTENER_MARCO_CORRESPONDIENTE);
    agregar_a_paquete(paquete, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete, &pre_direccion_fisica.nro_pagina, sizeof(uint32_t));
    for (int i = 0; i < mmu->cantidad_niveles; i++)
        agregar_a_paquete(paquete, &pre_direccion_fisica.entrada_nivel[i], sizeof(uint32_t));

    enviar_paquete(paquete, fd_conexion_memoria);
    log_debug(logger_cpu, "[SEND] Solicitud de marco enviada");

    paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
    if (paquete->codigo_operacion != OBTENER_MARCO_CORRESPONDIENTE) {
        log_error(logger_cpu, "Código inesperado: %d", paquete->codigo_operacion);
        eliminar_paquete(paquete);


        return -1;
    }

    int32_t marco = recibir_marco_solicitado(paquete);
    eliminar_paquete(paquete);


    

    return marco;
}

int32_t recibir_marco_solicitado(t_paquete* paquete){
    int32_t marco;
    recibir_buffer_en_paquete(fd_conexion_memoria, paquete);
    if (paquete && paquete->buffer && paquete->buffer->stream) {
        memcpy(&marco, paquete->buffer->stream + sizeof(uint32_t), sizeof(int32_t));
        return marco;
    }
    else{
        log_error(logger_cpu, "Error al recibir el marco solicitado");
        return -1;//VALOR DE ERROR PARA INDICAR MARCO INVALIDO
    }
}
//FUNCIONES DE TLB

tlb_t* inicializar_tlb(uint32_t maximas_entradas_tlb) {
	tlb_t* tlb = malloc(sizeof(tlb_t));
	tlb->cantidad_entradas = maximas_entradas_tlb;
	tlb->entradas = malloc(sizeof(entradas_tlb_t) * tlb->cantidad_entradas);
	for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
        tlb->entradas[i].bit_en_uso = 0;
        tlb->entradas[i].nro_pagina = UINT32_MAX;
        tlb->entradas[i].marco_asociado = UINT32_MAX;
        tlb->entradas[i].instante_referencia = 0;
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
	uint32_t marco_correspondiente = solicitar_marco_a_memoria(pre_direccion_fisica, pcb_actual->pid);
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

t_algoritmo_cache algoritmo_cache_from_string(char* str) {
    if (strcmp(str, "CLOCK") == 0)
        return CLOCK;
    if (strcmp(str, "CLOCK-M") == 0)
        return CLOCK_M;
    // Valor por defecto o error
    return CLOCK;
}

//Inicializo cache
t_memoria_cache* inicializar_cache(char* algoritmo, uint32_t cant_paginas, uint32_t tam_pagina, uint32_t retardo) {
    t_memoria_cache* cache = malloc(sizeof(t_memoria_cache));
    cache->algoritmo_reemplazo = algoritmo_cache_from_string(algoritmo);
    cache->puntero_reemplazo = 0;
    cache->retardo = retardo;
    cache->cantidad_paginas = cant_paginas;
    cache->paginas = malloc(sizeof(t_pagina_de_cache) * cant_paginas);

    for (uint32_t i = 0; i < cant_paginas; i++) {
        cache->paginas[i].nro_pagina = -1; // -1 significa que está libre
        cache->paginas[i].contenido = malloc(tam_pagina);
        cache->paginas[i].bit_uso = false;
        cache->paginas[i].bit_modificado = false;
    }
    log_info(logger_cpu, "CACHE inicializada, Algoritmo: %s | Cant. Paginas: %d", algoritmo, cant_paginas);
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

bool cache_esta_activada() {
    // Verifica si la cache está activada
    if (memoria_cache->cantidad_paginas == 0) {
        log_error(logger_cpu, "La cache no está habilitada");
        return false;
    }
    return true;
}

int buscar_espacio_libre_en_cache(t_memoria_cache* cache) {
    // Busca una página libre en la cache
    for (int i = 0; i < cache->cantidad_paginas; i++) {
        if (cache->paginas[i].nro_pagina == -1) {
            log_debug(logger_cpu,"Espacio libre en cache encontrado en índice: %d", i);
            return i; // devuelve el índice de la primera página libre
        }
    }
    log_debug(logger_cpu,"No hay espacio libre en cache");
    return -1; // no hay espacio libre
}

int buscar_pagina_en_cache(t_memoria_cache* cache, int nro_pagina_buscado) {
    for (int i = 0; i < cache->cantidad_paginas; i++) {
        if (cache->paginas[i].nro_pagina == nro_pagina_buscado) {
            cache->paginas[i].bit_uso = true;
            for(uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
					log_trace(logger_cpu, "Bits de uso previo a limpiar: %d",tlb->entradas[i].bit_en_uso);

				}
            log_info(logger_cpu, "PID: %d - Cache Hit - Pagina: %d", pcb_actual->pid, nro_pagina_buscado);
            return i; // devuelve el índice de esa página dentro del array
        }
    }

    log_info(logger_cpu, "PID: %d - Cache Miss - Pagina: %d", pcb_actual->pid, nro_pagina_buscado);
    return -1; // no se encontró
}

char* leer_de_cache(int indice, uint32_t desplazamiento, uint32_t tamanio_a_leer) {
     // Validaciones básicas
    if (indice < 0 || indice >= memoria_cache->cantidad_paginas){
        log_error(logger_cpu, "Índice de página fuera de rango: %d", indice);
        return NULL;
    }
    if (desplazamiento + tamanio_a_leer > mmu->tamanio_pagina) {
        // No se puede leer más allá del final de la página
        log_error(logger_cpu, "Desplazamiento y tamaño a leer exceden el tamaño de la página: %d + %d > %d", 
                  desplazamiento, tamanio_a_leer, mmu->tamanio_pagina);
        return NULL;
    }

    // Marcar bit de uso
    memoria_cache->paginas[indice].bit_uso = true;

    // Copiar los datos a un nuevo buffer y devolver
    char* datos_leidos = malloc(tamanio_a_leer);
    memcpy(datos_leidos, memoria_cache->paginas[indice].contenido + desplazamiento, tamanio_a_leer);

    return datos_leidos;
}

void escribir_en_cache(int indice, uint32_t desplazamiento, char* datos_a_escribir){
    // Validaciones básicas
    uint32_t tamanio_a_escribir = strlen(datos_a_escribir) + 1; // +1 para incluir el terminador nulo
    if (indice < 0 || indice >= memoria_cache->cantidad_paginas){
        log_error(logger_cpu, "Índice de página fuera de rango: %d", indice);
        return;
    }

    if (desplazamiento + tamanio_a_escribir > mmu->tamanio_pagina) {
        // No se puede escribir más allá del final de la página
        log_error(logger_cpu, "Desplazamiento y tamaño a escribir exceden el tamaño de la página: %d + %d > %d", desplazamiento, tamanio_a_escribir, mmu->tamanio_pagina);
        return;
    }

    // Escribir datos
    memcpy(memoria_cache->paginas[indice].contenido + desplazamiento, datos_a_escribir, tamanio_a_escribir);

    // Marcar uso y modificación
    memoria_cache->paginas[indice].bit_uso = true;
    memoria_cache->paginas[indice].bit_modificado = true;
    return;
}

void agregar_pagina_a_cache(uint32_t nro_pagina, int indice_libre, char* contenido) {

    // Agregar la página a la cache
    memoria_cache->paginas[indice_libre].nro_pagina = nro_pagina;
    memcpy(memoria_cache->paginas[indice_libre].contenido, contenido, mmu->tamanio_pagina);
    //LOG OBLIGATORIO
    log_info(logger_cpu, "PID: %d - Cache Add - Pagina: %d", pcb_actual->pid, nro_pagina);
}

int reemplazo_clock(t_memoria_cache* cache){
    while (1) {
        t_pagina_de_cache* pagina = &cache->paginas[cache->puntero_reemplazo];
        if (!pagina->bit_uso) {
            // Si está modificada, escribir en memoria antes de reemplazar
            if (pagina->bit_modificado) {
                // Debés calcular la dirección física de la página reemplazada
                t_pre_direccion_fisica pre_dir;
                pre_dir.nro_pagina = pagina->nro_pagina;
                pre_dir.desplazamiento = 0;
                pre_dir.entrada_nivel = calcular_entradas_por_nivel(pagina->nro_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);
                uint32_t marco = solicitar_marco_a_memoria(pre_dir, pcb_actual->pid);
                uint32_t direccion_fisica = calcular_direccion_fisica_final(marco, pre_dir);
                enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido);
                log_info(logger_cpu, "PID: %d - Memory Update - Página: %d - Frame: %d", pcb_actual->pid, pagina->nro_pagina, marco);
                free(pre_dir.entrada_nivel);
                pagina->bit_modificado = false;
            }
            int victima = cache->puntero_reemplazo;
            cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
            return victima;
        }
        pagina->bit_uso = false;
        cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
    }
}

int reemplazo_clock_m(t_memoria_cache* cache, uint32_t tam_pagina) {
    // Primera vuelta: buscar uso=0 y modificado=0
    for (int vueltas = 0; vueltas < 2; vueltas++) {
        for (int i = 0; i < cache->cantidad_paginas; i++) {
            t_pagina_de_cache* pagina = &cache->paginas[cache->puntero_reemplazo];
            if (!pagina->bit_uso && !pagina->bit_modificado) {
                int victima = cache->puntero_reemplazo;
                cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
                return victima;
            }
            cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
        }
        // Segunda vuelta: buscar uso=0 y modificado=1
        for (int i = 0; i < cache->cantidad_paginas; i++) {
            t_pagina_de_cache* pagina = &cache->paginas[cache->puntero_reemplazo];
            if (!pagina->bit_uso && pagina->bit_modificado) {
                // Escribir en memoria antes de reemplazar
                t_pre_direccion_fisica pre_dir;
                pre_dir.nro_pagina = pagina->nro_pagina;
                pre_dir.desplazamiento = 0;
                pre_dir.entrada_nivel = calcular_entradas_por_nivel(pagina->nro_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);
                uint32_t marco = solicitar_marco_a_memoria(pre_dir, pcb_actual->pid);
                uint32_t direccion_fisica = calcular_direccion_fisica_final(marco, pre_dir);
                enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido);
                log_info(logger_cpu, "PID: %d - Memory Update - Página: %d - Frame: %d", pcb_actual->pid, pagina->nro_pagina, marco);
                free(pre_dir.entrada_nivel);
                pagina->bit_modificado = false;
                int victima = cache->puntero_reemplazo;
                cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
                return victima;
            }
            pagina->bit_uso = false;
            cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
        }
    }
    // Si no encontró, reemplaza la actual
    int victima = cache->puntero_reemplazo;
    cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
    return victima;
}


int manejar_cache_miss(t_pre_direccion_fisica pre_direccion_fisica) {
    //Verificaciones básicas

    //Busco espacio libre para la nueva pagina
    int marco_cache = buscar_espacio_libre_en_cache(memoria_cache);
    if (marco_cache == -1) {
        log_warning(logger_cpu, "iniciando algoritmo de reemplazo");
        //ACA APLICA EL ALGORITMO DE REEMPLAZO DE CACHE
        if (memoria_cache->algoritmo_reemplazo == CLOCK) {      
            
                marco_cache = reemplazo_clock(memoria_cache);
        } else if (memoria_cache->algoritmo_reemplazo == CLOCK_M) {
                marco_cache = reemplazo_clock_m(memoria_cache, mmu->tamanio_pagina);

        }
    }

    //Verifico TLB
    int marco = esta_en_tlb(pre_direccion_fisica.nro_pagina);
    
    //Si la pagina NO está en TLB, es necesario acceder a la tabla de paginas
    if (marco == -1) {
        log_warning(logger_cpu, "Página %d NO encontrada en TLB,es necesario acceder a la tabla de páginas", pre_direccion_fisica.nro_pagina);
        // Accede a la tabla de páginas para obtener el marco y la agrega a la tlb
            marco = tlb_miss(pre_direccion_fisica);

     }
     //Si la pagina ya esta en la TLB, no necesito acceder a la tabla de paginas para obtener el marco
     else{
        log_debug(logger_cpu, "Página %d encontrada en TLB, no es necesario acceder a la tabla de páginas", pre_direccion_fisica.nro_pagina);
    }
    //Una vez que sabemos el marco de la página, podemos calcular la direccion física
    uint32_t direccion_fisica_final = pre_direccion_fisica.nro_pagina * mmu->tamanio_pagina;
    //Con la dirección física, buscamos la página en memoria y la cargamos en cache
    char* pagina = obtener_pagina_de_memoria(pcb_actual->pid, direccion_fisica_final);
    if (pagina == NULL) {
        log_error(logger_cpu, "Error al obtener la página de memoria para PID: %d, Página: %d", pcb_actual->pid, pre_direccion_fisica.nro_pagina);
        return -1; // Error al obtener la página
    }
    //agrego la pag a cache
    agregar_pagina_a_cache(pre_direccion_fisica.nro_pagina, marco_cache, pagina);
    free(pagina);
    log_debug(logger_cpu, "Se agrego la página %d del proceso  PID: %d a CACHE", pre_direccion_fisica.nro_pagina, pcb_actual->pid);
    return marco_cache;
}
void actualizar_memoria_principal_completa() {
    char* respuesta;
    log_warning(logger_cpu, "Estoy dentro de actualizar_memoria_principal_completa");

    // 🛑 Desactivar receptor para control total de recv
    pthread_mutex_lock(&mutex_conexion_memoria);
    receptor_habilitado = false;
    pthread_mutex_unlock(&mutex_conexion_memoria);

    for (uint32_t i = 0; i < memoria_cache->cantidad_paginas; i++) {
        log_warning(logger_cpu, "Estoy dentro del for de actualizar_memoria_principal_completa");

        t_pagina_de_cache* pagina = &memoria_cache->paginas[i];
        if (pagina->nro_pagina != -1 && pagina->bit_modificado) {
            // Calcular dirección física
            t_pre_direccion_fisica pre_dir;
            pre_dir.nro_pagina = pagina->nro_pagina;
            pre_dir.desplazamiento = 0;
            pre_dir.entrada_nivel = calcular_entradas_por_nivel(pagina->nro_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);

            uint32_t marco = solicitar_marco_a_memoria(pre_dir, pcb_actual->pid);
            uint32_t direccion_fisica = calcular_direccion_fisica_final(marco, pre_dir);

            // Escribir en memoria principal
            enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido);
            respuesta = recibir_read_o_write_de_memoria();

            if (respuesta && strcmp(respuesta, "WRITE completado con éxito") == 0) {
                log_info(logger_cpu, "PID: %d - Memory Update - Página: %d - Frame: %d", pcb_actual->pid, pagina->nro_pagina, marco);
            } else if (respuesta) {
                log_error(logger_cpu, "Error al actualizar la memoria principal para PID: %d, Página: %d", pcb_actual->pid, pagina->nro_pagina);
                log_error(logger_cpu, "Respuesta inesperada de memoria: %s", respuesta);
            } else {
                log_error(logger_cpu, "Error: respuesta NULL al actualizar la memoria principal para PID: %d, Página: %d", pcb_actual->pid, pagina->nro_pagina);
            }

            free(respuesta);
            free(pre_dir.entrada_nivel);
            pagina->bit_modificado = false;
        }

        // Limpiar la entrada de la caché
        pagina->nro_pagina = -1;
        pagina->bit_uso = false;
        pagina->bit_modificado = false;
        memset(pagina->contenido, 0, mmu->tamanio_pagina);
    }

    memoria_cache->puntero_reemplazo = 0;

    // ✅ Reactivar receptor al final
    pthread_mutex_lock(&mutex_conexion_memoria);
    receptor_habilitado = true;
    pthread_cond_signal(&condicion_reactivacion_recepcion_memoria);
    pthread_mutex_unlock(&mutex_conexion_memoria);
}

    

