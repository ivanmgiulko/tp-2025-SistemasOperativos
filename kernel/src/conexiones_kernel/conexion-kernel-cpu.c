
#include "conexion-kernel-cpu.h"

void manejar_conexion_kernel_interrupt() {
    while (1) {
        int socket_interrupt = esperar_cliente(fd_server_kernel_interrupt, logger_kernel);
        if (socket_interrupt == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en interrupt");
            continue;
        }
        log_debug(logger_kernel, "Nueva conexión en interrupt: socket %d", socket_interrupt);

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
    int offset;
    int pid;
    int tamanio_pid;
    while (1) {
        offset = 0;
        pid = 0;
        tamanio_pid = 0;
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_interrupt);
        switch (paquete->codigo_operacion){
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_kernel);
                break;
            case INSTRUCCION:
                break;

            case SYSCALL_IO:
                recibir_paquete(socket_interrupt, paquete);

                // Leer PID     
                memcpy(&tamanio_pid, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);
                memcpy(&pid, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                // Leer dispositivo
                int len_dispositivo=0;
                memcpy(&len_dispositivo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                char* dispositivo = malloc(len_dispositivo);
                memcpy(dispositivo, paquete->buffer->stream + offset, len_dispositivo); offset += len_dispositivo;

                // Leer tiempo
                int tamanio_tiempo;
                memcpy(&tamanio_tiempo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                int tiempo;
                memcpy(&tiempo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                log_info(logger_kernel, "## %d - Solicitó syscall: IO", pid);

                // log_trace(logger_kernel, "SYSCALL_IO recibida: PID=%d, dispositivo=%s, tiempo=%d", pid, dispositivo, tiempo);


                // Ver "conexion-kernel-cpu ya que ahora estamos simulando que recibe una IO desde CPU"
                // t_param_io* pruebaIO = deserializar_syscall_io(paquete->buffer);
                // bool interfaz_disponible = funcion_syscall_IO(io_recibida_cpu->dispositivo);
                // if(interfaz_disponible == true) { // La interfaz existe -> no contemplo casos de si ya esta siendo usada la IO
                //     log_info(logger_kernel, "## %d - Bloqueado por IO: %s", pcb_en_ready->pid, io_recibida_cpu->dispositivo);    
                //     // sacar de exec y mandar a blocked
                //     enviar_proceso_a_io_para_bloqueo(pcb_en_ready->pid, io_recibida_cpu->tiempo, socket_io);
                // } else {
                //     log_debug(logger_kernel, "LA INTERFAZ MOUSE NOOOOO ESTA DISPONIBLE!");
                // }

                free(dispositivo);

                eliminar_paquete(paquete);
                 
                break;
            case SYSCALL_INIT_PROC:

                recibir_paquete(socket_interrupt, paquete);

                offset = 0;

                // Leer tamaño PID
                memcpy(&tamanio_pid, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                // Leer PID
                memcpy(&pid, paquete->buffer->stream + offset, tamanio_pid); offset += tamanio_pid;

                // Leer tamaño archivo
                int len_archivo=0;
                memcpy(&len_archivo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                // Leer archivo
                char* archivo = malloc(len_archivo);
                memcpy(archivo, paquete->buffer->stream + offset, len_archivo); offset += len_archivo;

                // Leer tamaño_tamanio
                int tamanio_tamanio;
                memcpy(&tamanio_tamanio, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

                // Leer tamanio
                int tamanio;
                memcpy(&tamanio, paquete->buffer->stream + offset, tamanio_tamanio); offset += tamanio_tamanio;

                // Recibo prooceso desde CPU deserializado
                // t_pcb* proceso_prueba_syscall = proceso_syscall_prueba(paquete->buffer);
                // // Observacion: este proceso no tiene iniciada las metricas, habria que ver como usar la funcion "iniciarPCB"
                
                // // Pasamos procesos que llegan desde CPU (SYSCALL INIT_PROC) a NEW
                // pasar_pcb_a_new(proceso_prueba_syscall);
                //log_trace(logger_kernel, "SYSCALL_INIT_PROC recibida: PID=%d, archivo=%s, tamanio=%d", pid, archivo, tamanio);
                log_info(logger_kernel, "## %d - Solicitó syscall: INIT_PROC", pid);
            
                eliminar_paquete(paquete);
                free(archivo);
            
                break;
            case SYSCALL_DUMP_MEMORY:
                log_trace(logger_kernel, "Recibi la syscall DUMP_MEMORY desde CPU");
                
                recibir_paquete(socket_interrupt, paquete);
                
                
                // t_pcb* proceso_a_finalizar = pop_cola_mutex(estado_exec);
                
                // char* ip_memoria = configuracion_kernel->IP_MEMORIA;
                // char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
                // int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
                
                // enviar_proceso_a_finalizar_Memoria(*proceso_a_finalizar, fd_conexion_memoria);
                
                // manejar_conexion_kernel_memoria(fd_conexion_memoria);
                eliminar_paquete(paquete);
                break;

            case SYSCALL_EXIT:
                
                recibir_paquete(socket_interrupt, paquete);
                
                t_pcb* proceso_a_finalizar = pop_cola_mutex(estado_exec);

                log_info(logger_kernel, "## %d - Solicitó syscall: EXIT", proceso_a_finalizar->pid);

                char* ip_memoria = configuracion_kernel->IP_MEMORIA;
                char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
                int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

                enviar_proceso_a_finalizar_Memoria(*proceso_a_finalizar, fd_conexion_memoria);

                manejar_conexion_kernel_memoria(fd_conexion_memoria);
                
                eliminar_paquete(paquete);
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
