

#include "./cpu-utils.h"

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
		enviar_proceso_desalojado(fd_conexion_kernel_interrupt, pcb_actual->pid, pcb_actual->pc);
		sem_post(&sem_cpu_kernel);
		flag_interrupt = false; 
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
void inicializar_mmu(mmu_t* mmu){
	mmu->tamanio_pagina = 0;
	mmu->cantidad_entradas_tabla = 0; 
	mmu->cantidad_niveles = 0; 

	mmu = malloc(sizeof(mmu_t));

	log_info(logger_cpu, "MMU inicializada");
			
}
