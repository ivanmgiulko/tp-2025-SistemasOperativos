#include "conexion-kernel-cpu.h"

void manejar_conexion_kernel_interrupt() {
    while (1) {
        int socket_interrupt = esperar_cliente(fd_server_kernel_interrupt, logger_kernel);
        if (socket_interrupt == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en interrupt");
            continue;
        }
        log_debug(logger_kernel, "Nueva conexión en interrupt: socket %d", socket_interrupt);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_interrupt;
        pthread_create(&hilo_cliente_interrupt, NULL, (void*)manejar_cliente_interrupt, (void*)&socket_interrupt);
        pthread_detach(hilo_cliente_interrupt);
    }
}

void manejar_conexion_kernel_dispatch() {
    while (1) {
        socket_dispatch = esperar_cliente(fd_server_kernel_dispatch, logger_kernel);
        if (socket_dispatch == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en dispatch");
            continue;
        }
        log_debug(logger_kernel, "Nueva conexión en dispatch: socket %d", socket_dispatch);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_dispatch;
        pthread_create(&hilo_cliente_dispatch, NULL, (void*)manejar_cliente_dispatch, (void*)&socket_dispatch);
        pthread_detach(hilo_cliente_dispatch);
    }
}


int manejar_cliente_interrupt(void* socket_cliente_ptr){
	int socket_interrupt = *(int*)socket_cliente_ptr;
    while (1) {
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_interrupt);
        switch (paquete->codigo_operacion) {
        case MENSAJE:
            recibir_mensaje(socket_interrupt, logger_kernel);
            break;
        case INSTRUCCION:
            break;
        case SYSCALL_EXIT:
            log_trace(logger_kernel, "Recibi la syscall EXIT desde CPU");
            recv(socket_interrupt, &(paquete->buffer->size), sizeof(uint32_t), 0);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(socket_interrupt, paquete->buffer->stream, paquete->buffer->size, 0);
            
            t_pcb* proceso_a_finalizar = pop_cola_mutex(estado_exec);

            char* ip_memoria = configuracion_kernel->IP_MEMORIA;
            char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
            int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            enviar_proceso_a_finalizar_Memoria(*proceso_a_finalizar, fd_conexion_memoria);

            manejar_conexion_kernel_memoria(fd_conexion_memoria);

            break;

		case -1:
			log_error(logger_kernel, "El cliente [CPU - Interrupt] se desconectó.");
			return EXIT_FAILURE;
        default:
            log_warning(logger_kernel, "Operación desconocida en interrupt.");
            break;
        }
    }
    close(socket_interrupt);
    log_info(logger_kernel, "Conexión cerrada en interrupt: socket %d", socket_interrupt);
	return EXIT_SUCCESS;
}
void* manejar_cliente_dispatch(void* socket_cliente_ptr) {
	int socket_dispatch = *(int*)socket_cliente_ptr;
	while (1) {
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_dispatch);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_dispatch, logger_kernel);
			break;

        case SYSCALL_IO:
            
            recv(socket_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(socket_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
            t_param_io* pruebaIO = deserializar_syscall_io(paquete->buffer);
            return pruebaIO;
            break;  

        case SYSCALL_INIT_PROC:

            recv(socket_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(socket_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
            
            // Recibo prooceso desde CPU deserializado
            t_pcb* proceso_prueba_syscall = proceso_syscall_prueba(paquete->buffer);
            // Observacion: este proceso no tiene iniciada las metricas, habria que ver como usar la funcion "iniciarPCB"
            
            // Pasamos procesos que llegan desde CPU (SYSCALL INIT_PROC) a NEW
            pasar_pcb_a_new(proceso_prueba_syscall);

            break;

        // case SYSCALL_EXIT:

        //     recv(socket_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
		// 	paquete->buffer->stream = malloc(paquete->buffer->size);
		// 	recv(socket_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
            
        //     t_pcb* proceso_a_finalizar = pop_cola_mutex(estado_exec);

        //     char* ip_memoria = configuracion_kernel->IP_MEMORIA;
        //     char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
        //     int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

        //     enviar_proceso_a_finalizar_Memoria(*proceso_a_finalizar, fd_conexion_memoria);

        //     manejar_conexion_kernel_memoria(fd_conexion_memoria);

        //     break;

		case INSTRUCCION:
			break;
		case -1:
			log_error(logger_kernel, "el cliente [CPU - Dispatch] se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_dispatch);
	return EXIT_SUCCESS;
}

void enviar_proc_cpu(t_peticion_instruccion pcbInfo, int socket_cliente) { 
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(int) * 2;
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pcbInfo.pid, sizeof(int)); offset += sizeof(int);
    memcpy(buffer->stream + offset, &pcbInfo.pc, sizeof(int)); offset += sizeof(int);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = INFO_PROC_EXEC;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_param_io* deserializar_syscall_io(t_buffer* buffer) { 
    t_param_io* pruebaIO = malloc(sizeof(t_param_io));
    void* stream = buffer->stream;

    memcpy(&(pruebaIO->tiempo), stream, sizeof(int64_t)); stream += sizeof(int64_t);
    memcpy(&(pruebaIO->dispositivo_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    pruebaIO->dispositivo = malloc(pruebaIO->dispositivo_length);
    memcpy(pruebaIO->dispositivo , stream, pruebaIO->dispositivo_length);

    return pruebaIO;
}

t_pcb* proceso_syscall_prueba(t_buffer* buffer) { 
    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    int tamanioProceso;

    void* stream = buffer->stream;

    memcpy(&(tamanioProceso), stream, sizeof(int)); stream += sizeof(int);
    memcpy(&(path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    pathArchivoPseudocodigo = malloc(path_length);
    memcpy(pathArchivoPseudocodigo, stream, path_length);

    t_pcb* proceso_syscall_prueba = iniciarPCB(pathArchivoPseudocodigo, tamanioProceso, asignar_pid());

    return proceso_syscall_prueba;
}