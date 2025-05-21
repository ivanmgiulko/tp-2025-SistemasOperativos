
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
    int offset, pid, pc, tamanio_pid;
    while (1) {
        
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_interrupt);

        offset = 0;
        tamanio_pid = 0;
        
        switch (paquete->codigo_operacion){
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_kernel);
                break;
            
            case SYSCALL_IO:
                // de la syscall IO recibo PID, DISPOSITIVO, TIEMPO
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete); offset += sizeof(int) * 2;

                pc = _deserializar_pid(offset, paquete); offset += sizeof(int) * 2;
                
                t_syscall_io _syscall_io_recibida = _deserializar_syscall_io(offset, paquete);
                
                log_info(logger_kernel, "## %d - Solicitó syscall: IO", pid);

                t_io* interfaz_disponible = funcion_syscall_IO(_syscall_io_recibida.dispositivo);
                
                if(interfaz_disponible != NULL) { 
                    
                    log_info(logger_kernel, "## %d - Bloqueado por IO: %s", pid, _syscall_io_recibida.dispositivo);    
                    
                    t_pcb* _proceso_a_bloquear = _sacar_pcb_de_exec(pid);

                    _proceso_a_bloquear->pc = pc;

                    t_info_proceso_en_io* _info_proceso_bloqueado = malloc(sizeof(t_info_proceso_en_io));
                    _info_proceso_bloqueado->pid = _proceso_a_bloquear->pid;
                    _info_proceso_bloqueado->tiempo = _syscall_io_recibida.tiempo;

                    encolar_pcb_en_interfaz(interfaz_disponible, _info_proceso_bloqueado);
    
                    encolar_pcb_en_estado(estado_blocked, _proceso_a_bloquear); 

                    sem_post(&sem_cantidad_pcbs_en_blocked);
                    // SEMAFORO PARA PLANIFICADOR LARGO PLAZO PARA QUE SALGA DE ESPERA ACTIVA

                } else {

                    char* ip_memoria = configuracion_kernel->IP_MEMORIA;
                    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
                    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

                    // Falta realizar prueba
                    t_pcb* _proceso_a_terminar = _sacar_pcb_de_exec(pid);
                    enviar_proceso_a_finalizar_Memoria(*_proceso_a_terminar, fd_conexion_memoria);

                    manejar_conexion_kernel_memoria(fd_conexion_memoria);
                }

                eliminar_paquete(paquete);
                 
                break;
            case SYSCALL_INIT_PROC:
                // de la syscall INIT_PORC recibo PID, Archivo, tamanioProceso
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
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete);

                log_info(logger_kernel, "## %d - Solicitó syscall: DUMP_MEMORY", pid);
                
                eliminar_paquete(paquete);
                break;

            case SYSCALL_EXIT:
                
                recibir_paquete(socket_interrupt, paquete);
                
                // Me genera duda hacerlo asi... en caso un proceso termine antes que otro, va a finalizar el que estaba primero.
                // habria que recibir el PID, buscarlo en la cola de mutex, sacarlo de ahi y finalizarlo
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

t_syscall_io _deserializar_syscall_io(int offset, t_paquete* paquete) 
{ 
    t_syscall_io _syscall_io_recibida;

    int len_dispositivo = 0;
    memcpy(&len_dispositivo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

    char* dispositivo = malloc(len_dispositivo);
    memcpy(dispositivo, paquete->buffer->stream + offset, len_dispositivo); offset += len_dispositivo;
    _syscall_io_recibida.dispositivo = dispositivo;

    // Leer tiempo
    int tamanio_tiempo;
    memcpy(&tamanio_tiempo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

    int tiempo;
    memcpy(&tiempo, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);
    _syscall_io_recibida.tiempo = tiempo;

    return _syscall_io_recibida;
}

int _deserializar_pid(int offset, t_paquete* paquete) 
{ 
    int pid = 0, tamanio_pid = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pid, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&pid, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

    return pid;
}

int _deserializar_pc(int offset, t_paquete* paquete) 
{ 
    int pc = 0, tamanio_pc = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pc, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&pc, paquete->buffer->stream + offset, sizeof(int)); offset += sizeof(int);

    return pc;
}

t_pcb* _sacar_pcb_de_exec(int pid) 
{ 
    pthread_mutex_lock(&(estado_exec->mutex));
    t_pcb* _proceso_a_bloquear = NULL;
    for (int i = 0; i < list_size(estado_exec->cola); i++) {
        t_pcb* pcb = list_get(estado_exec->cola, i);
        if (pcb->pid == pid) {
            _proceso_a_bloquear = list_remove(estado_exec->cola, i); // Eliminar el elemento
            break; // Salir del bucle una vez encontrado
        }
    }

    pthread_mutex_unlock(&(estado_exec->mutex));
    return _proceso_a_bloquear;
}