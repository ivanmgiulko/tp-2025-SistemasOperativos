#include "conexion-kernel-io.h"

void manejar_conexion_kernel_io() {
    while (1) {
        socket_io = esperar_cliente(fd_server_io, logger_kernel);
        if (fd_server_io == -1) {
            log_error(logger_kernel, "Error al aceptar cliente de IO");
            continue;
        }

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_io;
        pthread_create(&hilo_cliente_io, NULL, (void*)manejar_cliente_io, (void*)&socket_io);
        pthread_detach(hilo_cliente_io);
    }
}

int manejar_cliente_io(void* socket_cliente_ptr){
    int socket_cliente_io = *(int*)socket_cliente_ptr;
	size_t tamanio_interfaz;
	int32_t resultado_handshake_exitoso = 1;
	if(recv(socket_cliente_io, &tamanio_interfaz, sizeof(size_t), 0) != sizeof(size_t)){
		log_error(logger_kernel, "Error al recibir el tamanio de la interfaz");
		return EXIT_FAILURE;
	}

	void* stream = malloc(tamanio_interfaz);
	if(recv(socket_cliente_io, stream, tamanio_interfaz, 0) != tamanio_interfaz){
		log_error(logger_kernel, "Error al recibir el nombre de la interfaz");
		return EXIT_FAILURE;
	}

	// Enviar respuesta al cliente
	send(socket_cliente_io, &resultado_handshake_exitoso, sizeof(int32_t), 0);

	char* nombre_io = malloc(tamanio_interfaz + 1);
	memcpy(nombre_io, stream, tamanio_interfaz);
	nombre_io[tamanio_interfaz] = '\0';  // Asegurarse de que termine en \0

	// Si la lista de interfaces ya fue iniciada, no se crea de nuevo
	if(lista_de_io == NULL){
		inicializar_lista_io();
	}

	inicializar_io(nombre_io, socket_cliente_io);

	free(stream);

	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_cliente_io);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente_io, logger_kernel);
			break;
		case PROCESO_DESBLOQUEADO:
			recibir_paquete(socket_cliente_io, paquete);

			log_warning(logger_kernel, "Recibo el proceso bloqueado para finalizar");

			uint8_t pid_desbloqueado = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "## %d finalizó IO y pasa a READY", pid_desbloqueado);
			
			pthread_mutex_lock(&lista_de_io->mutex_lista);
			t_io* _io_que_usa_pcb_bloqueado    = buscar_io_en_lista(lista_de_io->lista_ios, pid_desbloqueado);
			_io_que_usa_pcb_bloqueado->enabled = true;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			list_remove(_io_que_usa_pcb_bloqueado->procesos, 0);
			
			t_pcb* _proceso_desbloqueado = _sacar_pcb_de_blocked_auxiliar(pid_desbloqueado);
			
			pasar_pcb_blocked_a_ready(_proceso_desbloqueado);

			break;

		case PROCESO_SUSPENDIDO_DESBLOQUEADO:
			recibir_paquete(socket_cliente_io, paquete);

			log_warning(logger_kernel, "Recibo el proceso bloqueado y suspendido para finalizar");

			uint8_t pid_desbloqueado_susp = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "## %d finalizó IO y pasa a READY", pid_desbloqueado_susp);
			
			pthread_mutex_lock(&lista_de_io->mutex_lista);
			t_io* _io_que_usa_pcb_bloqueado_susp    = buscar_io_en_lista(lista_de_io->lista_ios, pid_desbloqueado_susp);
			_io_que_usa_pcb_bloqueado_susp->enabled = true;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			list_remove(_io_que_usa_pcb_bloqueado_susp->procesos, 0);
			
			t_pcb* _proceso_desbloqueado_suspendido = _sacar_pcb_de_blocked_auxiliar(pid_desbloqueado_susp);
			
			pasar_pcb_suspblocked_a_suspready(_proceso_desbloqueado_suspendido);

			break;
	
		case -1:
			log_error(logger_kernel, "el cliente [IO] se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_cliente_io);
	return EXIT_SUCCESS;
}

void enviar_proceso_a_io_para_bloqueo(uint8_t pid, int64_t tiempo, int socket_cliente) { 
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(int64_t);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &tiempo, sizeof(int64_t)); offset += sizeof(int64_t);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_BLOQUEADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void enviar_proceso_suspendido_a_io_para_bloqueo(uint8_t pid, int64_t tiempo, int socket_cliente) { 
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint8_t) + sizeof(int64_t);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(uint8_t)); offset += sizeof(uint8_t);
    memcpy(buffer->stream + offset, &tiempo, sizeof(int64_t)); offset += sizeof(int64_t);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_SUSPENDIDO_BLOQUEADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

uint8_t _recibir_proceso_bloqueado(t_buffer* buffer) { 
	uint8_t pid;
    
    memcpy(&pid, buffer->stream, sizeof(uint8_t)); buffer->stream += sizeof(uint8_t);
    
    return pid;
}

t_lista_io* lista_de_io = NULL;

void inicializar_lista_io() {
	lista_de_io = malloc(sizeof(t_lista_io));
    lista_de_io->lista_ios = list_create();
	pthread_mutex_init(&lista_de_io->mutex_lista, NULL);
    log_info(logger_kernel, "Lista de IO inicializada");
}

void inicializar_io(char* nombre_io, int socket_io) {
    
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->socket = socket_io;
	io->enabled = true;
    list_add(lista_de_io->lista_ios, io);

    log_debug(logger_kernel, "IO inicializado: %s", io->nombre);
    
 }

t_io* buscar_io(t_list* lista_de_io, char* nombre_io) {
    bool _es_el_io(void* elemento) {
        t_io* io = (t_io*) elemento;
        return string_contains(io->nombre, nombre_io);
    }

    return list_find(lista_de_io, _es_el_io);
}

t_io* funcion_syscall_IO(char* nombreInterfaz) { 
    return buscar_io(lista_de_io->lista_ios, nombreInterfaz);
}

void encolar_pcb_en_interfaz(t_io* interfaz, t_info_proceso_en_io* pcb) {
    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    list_add(interfaz->procesos, pcb);
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
}

t_pcb* _sacar_pcb_de_blocked_auxiliar(uint8_t pid) 
{ 
    pthread_mutex_lock(&(estado_blocked_aux->mutex));
    t_pcb* _proceso_a_desbloquear = NULL;
    for (int i = 0; i < list_size(estado_blocked_aux->cola); i++) {
        t_pcb* pcb = list_get(estado_blocked_aux->cola, i);
        if (pcb->pid == pid) {
            _proceso_a_desbloquear = list_remove(estado_blocked_aux->cola, i); 
            break; 
        }
    }

    pthread_mutex_unlock(&(estado_blocked_aux->mutex));
    return _proceso_a_desbloquear;
}