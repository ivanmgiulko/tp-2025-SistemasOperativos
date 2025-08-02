

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

    // log_debug(logger_cpu, "Datos de memoria recibidos: tamanio_pagina=%d, cantidad_niveles=%d, cantidad_entradas_tabla=%d",
    //       mmu->tamanio_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);
}

void pedir_instruccion_a_memoria(t_peticion_instruccion* infoPCB){
	
    log_info(logger_cpu, "## PID: <%d> - FETCH - Program Counter: <%d>", infoPCB->pid, infoPCB->pc);

    t_paquete* paquete = crear_paquete_instruccion();
    agregar_a_paquete(paquete, &(infoPCB->pid), sizeof(uint8_t));
    agregar_a_paquete(paquete, &(infoPCB->pc), sizeof(uint16_t));

    if(paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        return;
    }
    //Envía la peticion serializada a MEMORIA
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete); 
}	
	
void manejar_respuesta_de_instruccion(){
    while(1){
        sem_wait(&sem_instruccion);

        t_instruccion* instruccion = decode(respuesta_instruccion);
        if (!instruccion) {
            free_instruccion(instruccion);

            return; 
        }
        
        ejecutar_instruccion(instruccion);
        
        free_instruccion(instruccion);
        free(respuesta_instruccion);	
    }
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

	
    uint32_t resultado_handshake;
    uint32_t t_modulo = 1;
    send(fd_conexion_memoria, &t_modulo, sizeof(uint32_t), 0);

    recv(fd_conexion_memoria, &resultado_handshake, sizeof(uint32_t), MSG_WAITALL);

    if(resultado_handshake == 1){
        pedir_datos_a_memoria("Te saludo desde el modulo [[CPU]]", fd_conexion_memoria);
    }
	
}

void _handshake_kernel_con_cpu_id(int fd_conexion, char* cpu_id) {

	uint8_t handshake = atoi(cpu_id);
	uint8_t result;

	send(fd_conexion, &handshake, sizeof(uint8_t), 0);
	recv(fd_conexion, &result, sizeof(uint8_t), MSG_WAITALL);

	if (result == 0) {
    //	log_debug(logger_cpu, "Handshake con [KERNEL] exitoso!");
	} else {
		log_error(logger_cpu, "Handshake con [KERNEL] fallido!");
		abort();
	}
}

void check_interrupt(){
     while (1) {
      //  log_trace(logger_cpu, "AY");
        sem_wait(&sem_cpu);
       // log_trace(logger_cpu, "DIOS MIO");

        if(flag_interrupt){
            log_info(logger_cpu, "## Llega interrupción al puerto Interrupt");
            flag_interrupt = false; 
            enviar_proceso_desalojado(fd_conexion_kernel_interrupt, pcb_actual->pid, pcb_actual->pc);
         //   log_trace(logger_cpu, "ANASHE");
            if(cache_esta_activada()){
           
           //     log_error(logger_cpu, "SE ENTRA A cache_esta_activada");
                actualizar_memoria_principal_completa();
            }
            if(tlb_esta_activada())
                limpiar_tlb();
					
        
        } else {
            pedir_instruccion_a_memoria(pcb_actual);
            //log_trace(logger_cpu, "CLARENCE");

        }

    }
}

void enviar_proceso_desalojado(int socket_servidor, uint8_t pid, uint16_t pc) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(int32_t) + sizeof(int32_t);
	buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &pc, sizeof(uint16_t)); offset += sizeof(uint16_t);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_DESALOJADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int32_t) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int32_t));   offset += sizeof(int32_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_servidor, a_enviar, buffer->size + sizeof(int32_t) + sizeof(uint32_t), 0);

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
    return mmu;
}

void destruir_mmu(mmu_t* mmu) {
    free(mmu);  // no hay punteros internos, solo liberar la struct
}



void enviar_read_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, uint32_t tamanio, op_code codigo) {
    //Envia el read a memoria
    t_paquete* paquete_read = crear_paquete_con_codigo(codigo);
    agregar_a_paquete(paquete_read, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete_read, &direccion_fisica_final, sizeof(uint32_t));
    agregar_a_paquete(paquete_read, &tamanio, sizeof(uint32_t));
    enviar_paquete(paquete_read, fd_conexion_memoria);
//    log_debug(logger_cpu, "Enviando READ a Memoria: PID=%d, Direccion Fisica Final=%d, Tamanio=%d", pid, direccion_fisica_final, tamanio);
    eliminar_paquete(paquete_read);
}

void enviar_write_a_memoria(uint8_t pid, uint32_t direccion_fisica_final, char* datos, uint32_t tamanio, op_code codigo) {
    t_paquete* paquete_write = crear_paquete_con_codigo(codigo);
    agregar_a_paquete(paquete_write, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete_write, &direccion_fisica_final, sizeof(uint32_t));
    agregar_a_paquete(paquete_write, datos, tamanio); 
    enviar_paquete(paquete_write, fd_conexion_memoria);

//    log_debug(logger_cpu, "Enviando WRITE a Memoria: PID=%d, Direccion Fisica Final=%d", pid, direccion_fisica_final);
    eliminar_paquete(paquete_write);
}

char* recibir_read_o_write_de_memoria() {
    t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);

    // Recibir el código de operación desde la conexión
    paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);

    // Validar tipo de operación recibida
    if (paquete->codigo_operacion != READ_MEMORIA && paquete->codigo_operacion != WRITE_MEMORIA) {
//        log_error(logger_cpu, "Error: Código de operación inesperado: %d", paquete->codigo_operacion);
        eliminar_paquete(paquete);
        return NULL;
    }

    // Recibir el buffer
    recibir_buffer_en_paquete(fd_conexion_memoria, paquete);

    // Deserializar el contenido
     int desplazamiento = 0;
    uint32_t tamanio;
    memcpy(&tamanio, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    char* contenido = malloc(tamanio);
    memcpy(contenido, paquete->buffer->stream + desplazamiento, tamanio);


  //  log_debug(logger_cpu, "Contenido recibido: %s", contenido);

    // Log según el tipo de operación
    if (paquete->codigo_operacion == READ_MEMORIA) {
   //     log_debug(logger_cpu, "READ_MEMORIA recibido. Contenido: %s", contenido);
    } else if (paquete->codigo_operacion == WRITE_MEMORIA) {
    //    log_debug(logger_cpu, "WRITE_MEMORIA recibido. Contenido: %s", contenido);
    }

    eliminar_paquete(paquete);
    return contenido;
}

char* deserializar_read_o_write_de_memoria(t_paquete* paquete){
    uint32_t tamanio = 0;
    uint32_t desplazamiento = 0;
    memcpy(&tamanio, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Reservá solo lo necesario (+1 para '\0')
    char* contenido = malloc(tamanio + 1);
    memcpy(contenido, paquete->buffer->stream + desplazamiento, tamanio);
    contenido[tamanio] = '\0'; 

   // log_debug(logger_cpu, "Contenido deserializado: %s", contenido);

    return contenido;
}

char* recibir_string_de_memoria(){
    t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);
    paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
    recibir_buffer_en_paquete(fd_conexion_memoria, paquete);
    if(paquete->buffer->stream == NULL){
    log_error(logger_cpu, "ERROR al recibir paquete desde memoria (contenido NULL)");
    eliminar_paquete(paquete);
    return NULL;
    }
    char* mensaje = deserializar_read_o_write_de_memoria(paquete);
  //  log_trace(logger_cpu, "Mensaje recibido de memoria: %s", mensaje);
    eliminar_paquete(paquete);
    return mensaje;
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
	// log_debug(logger_cpu,"Tamanio Pagina = %d", TAMANIO_PAGINA);
	// log_debug(logger_cpu,"Cantdad niveles = %d", CANT_NIVELES);
	// log_debug(logger_cpu,"entradas tabla = %d", CANT_ENTRADAS_TABLA);
	// log_debug(logger_cpu, "direccion logica = %d", direccion_logica);
	
    resultado.nro_pagina = direccion_logica / TAMANIO_PAGINA;
    resultado.desplazamiento = direccion_logica % TAMANIO_PAGINA;

	// Reservar memoria dinámica para el array de niveles
    resultado.entrada_nivel = calcular_entradas_por_nivel(resultado.nro_pagina, CANT_NIVELES, CANT_ENTRADAS_TABLA);
	//Devuelve nro de pagina, desplazamiento y entradas de cada nivel
	//log_debug(logger_cpu, "Dirección lógica %d traducida a dirección física: Página %d, Desplazamiento %d", direccion_logica, resultado.nro_pagina, resultado.desplazamiento);
    return resultado;
}

// Calcula la direccion fisica real para una procesos con su pid y marco + desplazamiento
uint32_t calcular_direccion_fisica_final(uint32_t marco, t_pre_direccion_fisica pre_direccion_fisica){
	uint32_t direccion_fisica_final = (marco * mmu->tamanio_pagina) + pre_direccion_fisica.desplazamiento;
	//log_debug(logger_cpu, "Dirección física final calculada: %d", direccion_fisica_final);
	return direccion_fisica_final;
}

// Solicita a memoria el marco correspondiente a una pagina y devuelve la direccion fisica
int32_t solicitar_marco_a_memoria(t_pre_direccion_fisica pre_direccion_fisica, uint8_t pid) {

  

   // log_debug(logger_cpu, "Solicitando marco a memoria para PID: %d, Página: %d", pid, pre_direccion_fisica.nro_pagina);
    t_paquete* paquete = crear_paquete_con_codigo(OBTENER_MARCO_CORRESPONDIENTE);
    agregar_a_paquete(paquete, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete, &pre_direccion_fisica.nro_pagina, sizeof(uint32_t));
    for (int i = 0; i < mmu->cantidad_niveles; i++)
        agregar_a_paquete(paquete, &pre_direccion_fisica.entrada_nivel[i], sizeof(uint32_t));

    enviar_paquete(paquete, fd_conexion_memoria);
   // log_debug(logger_cpu, "[SEND] Solicitud de marco enviada");

    sem_wait(&sem_rta_marco);
    log_info(logger_cpu,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", 
             pid, pre_direccion_fisica.nro_pagina, marco_global);
    eliminar_paquete(paquete);
    return marco_global;
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
	return tlb;
}

bool tlb_esta_activada(){
    if(tlb->cantidad_entradas == 0){
        return false;
    }
    return true;
}

void destruir_tlb(tlb_t* tlb) {
    if (!tlb) return;

    free(tlb->entradas);  // liberar array de entradas
    free(tlb);            // liberar estructura completa
}

void agregar_a_tlb(uint32_t nro_pagina_entrante, uint32_t marco_asociado_entrante, algoritmo_tlb_t algoritmo){
	if (tlb->cantidad_entradas == 0) {
   // log_warning(logger_cpu, "No se usa TLB: cantidad de entradas es 0.");
    return;
    }
 	for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
        if (tlb->entradas[i].bit_en_uso == 0) {
            tlb->entradas[i].nro_pagina = nro_pagina_entrante;
            tlb->entradas[i].marco_asociado = marco_asociado_entrante;
			tlb->entradas[i].bit_en_uso = 1; 
      //      log_debug(logger_cpu, "Agregada a TLB: Pagina <%d> - Marco <%d> en posicion <%d>", nro_pagina_entrante, marco_asociado_entrante, i);
            // log_warning(logger_cpu, "TLB: bit en uso <%d> - Marco <%d> en numero de pagina <%d>", tlb->entradas[i].bit_en_uso, tlb->entradas[i].marco_asociado, 
			// 			tlb->entradas[i].nro_pagina);

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
			log_info(logger_cpu, "PID <%d> - TLB HIT - Pagina: <%d>",pcb_actual->pid, nro_pagina);
			// TLB Hit: “PID: <PID> - TLB HIT - Pagina: <NUMERO_PAGINA>”
            tlb->entradas[i].instante_referencia = ++contador_accesos_tlb;
			return marco_asociado;
		}
	}
	log_info(logger_cpu, "PID: <%d> - TLB MISS - Pagina: <%d>", pcb_actual->pid,nro_pagina);
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
  //  log_debug(logger_cpu, "TLB llena. Reemplazo FIFO en posicion <%d>: Pagina <%d> - Marco <%d>", proxima_a_reemplazar, nro_pagina_entrante, marco_asociado_entrante);
    proxima_a_reemplazar = (proxima_a_reemplazar + 1) % tlb->cantidad_entradas;
}

 void agregar_a_tlb_lru(uint32_t nro_pagina_entrante, uint32_t marco_asociado_entrante) {
    int indice_lru = -1;
    uint64_t min_ref = UINT64_MAX;


    for (uint32_t i = 0; i < tlb->cantidad_entradas; i++) {
        if (tlb->entradas[i].instante_referencia < min_ref) {
            min_ref = tlb->entradas[i].instante_referencia;
            indice_lru = i;
        }
    }
   // log_debug(logger_cpu, "TLB llena. Reemplazo LRU en posicion <%d>: Pagina <%d> - Marco <%d>",
    //        indice_lru, nro_pagina_entrante, marco_asociado_entrante);


    // Insertar entrada
    tlb->entradas[indice_lru].nro_pagina = nro_pagina_entrante;
    tlb->entradas[indice_lru].marco_asociado = marco_asociado_entrante;
    tlb->entradas[indice_lru].bit_en_uso = 1;
    tlb->entradas[indice_lru].instante_referencia = ++contador_accesos_tlb;
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
    cache->paginas = malloc(sizeof(t_pagina_de_cache) * cache->cantidad_paginas);

    for (uint32_t i = 0; i < cache->cantidad_paginas; i++) {
        cache->paginas[i].nro_pagina = -1; // -1 significa que está libre
        cache->paginas[i].contenido = malloc(tam_pagina);
        cache->paginas[i].bit_uso = false;
        cache->paginas[i].bit_modificado = false;
    }
   // log_debug(logger_cpu, "CACHE inicializada, Algoritmo: %s | Cant. Paginas: %d", algoritmo, cache->cantidad_paginas);
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
   //         log_debug(logger_cpu,"Espacio libre en cache encontrado en índice: %d", i);
            return i; // devuelve el índice de la primera página libre
        }
    }
  //  log_debug(logger_cpu,"No hay espacio libre en cache");
    return -1; // no hay espacio libre
}

int buscar_pagina_en_cache(t_memoria_cache* cache, int nro_pagina_buscado) {
    for (int i = 0; i < cache->cantidad_paginas; i++) {
        if (cache->paginas[i].nro_pagina == nro_pagina_buscado) {
            cache->paginas[i].bit_uso = true;
            log_info(logger_cpu, "PID: <%d> - Cache Hit - Pagina: <%d>", pcb_actual->pid, nro_pagina_buscado);
            return i; // devuelve el índice de esa página dentro del array
        }
    }

    log_info(logger_cpu, "PID: <%d> - Cache Miss - Pagina: <%d>", pcb_actual->pid, nro_pagina_buscado);
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

   
    void* datos_leidos = malloc(tamanio_a_leer);
	memcpy(datos_leidos, memoria_cache->paginas[indice].contenido + desplazamiento, tamanio_a_leer);

	char* datos_leidos_como_string = calloc(tamanio_a_leer + 1, sizeof(char));
    memcpy(datos_leidos_como_string, datos_leidos, tamanio_a_leer);

    free(datos_leidos);
    return datos_leidos_como_string;
}

void escribir_en_cache(int indice, uint32_t desplazamiento, char* datos_a_escribir){
    // Validaciones básicas
   // log_debug(logger_cpu, "Escribiendo en cache: Indice: %d, Desplazamiento: %d, Datos: %s", indice, desplazamiento, datos_a_escribir);
    uint32_t tamanio_a_escribir = strlen(datos_a_escribir); // +1 para incluir el terminador nulo
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
    log_info(logger_cpu, "PID: <%d> - Cache Add - Pagina: <%d>", pcb_actual->pid, nro_pagina);
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
                enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido, mmu->tamanio_pagina, WRITE_MEMORIA_CACHE);
                sem_wait(&sem_respuesta_memo);
                if(respuesta_memo == NULL) {
                    log_error(logger_cpu, "Error al recibir confirmación de escritura en memoria");
                    free(pre_dir.entrada_nivel);
                    return -1; // Error al recibir confirmación
                }
                log_info(logger_cpu, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pcb_actual->pid, pagina->nro_pagina, marco);
                free(pre_dir.entrada_nivel);
                free(respuesta_memo);
                respuesta_memo = NULL;
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
    // Intentar dos vueltas completas
    for (int ronda = 0; ronda < 2; ronda++) {
        for (int i = 0; i < cache->cantidad_paginas; i++) {
            t_pagina_de_cache* pagina = &cache->paginas[cache->puntero_reemplazo];

            if (!pagina->bit_uso && !pagina->bit_modificado) {
                // Víctima ideal
                int victima = cache->puntero_reemplazo;
                cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
                log_info(logger_cpu, "PID: <%d> - Reemplazo - Página: <%d>", pcb_actual->pid, pagina->nro_pagina);

                return victima;
            }

            cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
        }

        // Si no se encontró en la ronda anterior, ahora sí recorremos de nuevo para uso=0, modificado=1
        for (int i = 0; i < cache->cantidad_paginas; i++) {
            t_pagina_de_cache* pagina = &cache->paginas[cache->puntero_reemplazo];

            if (!pagina->bit_uso && pagina->bit_modificado) {
                // Escribir en memoria
                t_pre_direccion_fisica pre_dir;
                pre_dir.nro_pagina = pagina->nro_pagina;
                pre_dir.desplazamiento = 0;
                pre_dir.entrada_nivel = calcular_entradas_por_nivel(pagina->nro_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);
                uint32_t marco = solicitar_marco_a_memoria(pre_dir, pcb_actual->pid);
                uint32_t direccion_fisica = calcular_direccion_fisica_final(marco, pre_dir);
                enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido, mmu->tamanio_pagina, WRITE_MEMORIA_CACHE);
                sem_wait(&sem_respuesta_memo);

                if (respuesta_memo == NULL) {
                    log_error(logger_cpu, "Error al recibir confirmación de escritura en memoria");
                    free(pre_dir.entrada_nivel);
                    return -1;
                }

                log_info(logger_cpu, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pcb_actual->pid, pagina->nro_pagina, marco);
                free(pre_dir.entrada_nivel);
                free(respuesta_memo);
                respuesta_memo = NULL;
                pagina->bit_modificado = false;

                int victima = cache->puntero_reemplazo;
                cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
                return victima;
            }

            // IMPORTANTE: limpiar bit de uso en esta pasada
            pagina->bit_uso = false;

            cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
        }
    }

    // Última opción: reemplazar donde apunta, ya que se recorrieron 2 vueltas y no se encontró candidato ideal
    int victima = cache->puntero_reemplazo;
    cache->puntero_reemplazo = (cache->puntero_reemplazo + 1) % cache->cantidad_paginas;
    log_info(logger_cpu, "PID: <%d> - Reemplazo - Página: <%d>", pcb_actual->pid, victima);

    return victima;
}

int manejar_cache_miss(t_pre_direccion_fisica pre_direccion_fisica) {
    //Verificaciones básicas

    //Busco espacio libre para la nueva pagina
    int marco_cache = buscar_espacio_libre_en_cache(memoria_cache);
    if (marco_cache == -1) {
  //      log_warning(logger_cpu, "iniciando algoritmo de reemplazo");
        //ACA APLICA EL ALGORITMO DE REEMPLAZO DE CACHE
        if (memoria_cache->algoritmo_reemplazo == CLOCK) {      
            
                marco_cache = reemplazo_clock(memoria_cache);
        } else if (memoria_cache->algoritmo_reemplazo == CLOCK_M) {
                marco_cache = reemplazo_clock_m(memoria_cache, mmu->tamanio_pagina);

        }
    }
    int marco;
    //Verifico TLB
    if(tlb_esta_activada()){
        marco = esta_en_tlb(pre_direccion_fisica.nro_pagina);
        
        //Si la pagina NO está en TLB, es necesario acceder a la tabla de paginas
        if (marco == -1) {
        //    log_warning(logger_cpu, "Marco correspondiente a la Página %d NO encontrado en TLB, Es necesario acceder a la tabla de páginas", pre_direccion_fisica.nro_pagina);
            // Accede a la tabla de páginas para obtener el marco y la agrega a la tlb
                marco = tlb_miss(pre_direccion_fisica);

        }
        //Si la pagina ya esta en la TLB, no necesito acceder a la tabla de paginas para obtener el marco
        else{
     //       log_debug(logger_cpu, "Página %d encontrada en TLB, no es necesario acceder a la tabla de páginas", pre_direccion_fisica.nro_pagina);
        }
    }
    else {
     //   log_warning(logger_cpu, "TLB no está activada, se accederá a la tabla de páginas directamente");
        // Accede a la tabla de páginas para obtener el marco
        marco = solicitar_marco_a_memoria(pre_direccion_fisica, pcb_actual->pid);
    } 
    //Una vez que sabemos el marco de la página, podemos calcular la direccion física
    uint32_t direccion_fisica_final = pre_direccion_fisica.nro_pagina * mmu->tamanio_pagina;
    //Con la dirección física, buscamos la página en memoria y la cargamos en cache
    enviar_read_a_memoria(pcb_actual->pid, direccion_fisica_final, mmu->tamanio_pagina, READ_MEMORIA_CACHE);
    sem_wait(&sem_read);
    if (ultima_lectura == NULL) {
        log_error(logger_cpu, "Error al obtener la página de memoria para PID: %d, Página: %d", pcb_actual->pid, pre_direccion_fisica.nro_pagina);
        return -1; // Error al obtener la página
    }
    //agrego la pag a cache
    agregar_pagina_a_cache(pre_direccion_fisica.nro_pagina, marco_cache, ultima_lectura);
    free(ultima_lectura);
    ultima_lectura = NULL;
    return marco_cache;
}
void actualizar_memoria_principal_completa() {
    
    for (uint32_t i = 0; i < memoria_cache->cantidad_paginas; i++) {

        t_pagina_de_cache* pagina = &memoria_cache->paginas[i];
        if (pagina->nro_pagina != -1 && pagina->bit_modificado) {
            // Calcular dirección física
            t_pre_direccion_fisica pre_dir;
            pre_dir.nro_pagina = pagina->nro_pagina;
            pre_dir.desplazamiento = 0;
            pre_dir.entrada_nivel = calcular_entradas_por_nivel(pagina->nro_pagina, mmu->cantidad_niveles, mmu->cantidad_entradas_tabla);

            uint32_t marco = solicitar_marco_a_memoria(pre_dir, pcb_actual->pid);
            uint32_t direccion_fisica = calcular_direccion_fisica_final(marco, pre_dir);
       //     log_debug(logger_cpu, "Contenido de pagina antes de enviar a memoria: %s", pagina->contenido);
            // Escribir en memoria principal
            enviar_write_a_memoria(pcb_actual->pid, direccion_fisica, pagina->contenido, mmu->tamanio_pagina, WRITE_MEMORIA_CACHE);
            sem_wait(&sem_respuesta_memo);

            if (respuesta_memo && strcmp(respuesta_memo, "WRITE completado con éxito") == 0) {
                log_info(logger_cpu, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pcb_actual->pid, pagina->nro_pagina, marco);
            } else if (respuesta_memo) {
                log_error(logger_cpu, "Error al actualizar la memoria principal para PID: %d, Página: %d", pcb_actual->pid, pagina->nro_pagina);
                log_error(logger_cpu, "Respuesta inesperada de memoria: %s", respuesta_memo);
            } else {
                log_error(logger_cpu, "Error: respuesta NULL al actualizar la memoria principal para PID: %d, Página: %d", pcb_actual->pid, pagina->nro_pagina);
            }

            free(respuesta_memo);
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

  
}

    

