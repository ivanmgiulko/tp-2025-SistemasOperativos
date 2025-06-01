
#include "conexion-kernel-cpu.h"

void manejar_conexion_kernel_interrupt() {
    while (1) {
        int socket_interrupt = esperar_cliente(fd_server_kernel_interrupt, logger_kernel);
        if (socket_interrupt == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en interrupt");
            continue;
        }

        uint8_t id_cpu = _recibir_handshake_de_cpu(socket_interrupt, SOCKET_INTERRUPT);

        _agregar_socket_en_cpu(id_cpu, SOCKET_INTERRUPT, socket_interrupt);

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

        uint8_t id_cpu = _recibir_handshake_de_cpu(socket_dispatch, SOCKET_DISPATCH);

        _agregar_socket_en_cpu(id_cpu, SOCKET_DISPATCH, socket_dispatch);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_dispatch;
        pthread_create(&hilo_cliente_dispatch, NULL, (void*)manejar_cliente_dispatch, (void*)&socket_dispatch);
        pthread_detach(hilo_cliente_dispatch);
    }
}

int manejar_cliente_interrupt(void* socket_cliente_ptr){
	int socket_interrupt = *(int*)socket_cliente_ptr;
    int* offset = malloc(sizeof(int)); 
    int pid, pc;
    while (1) {
        
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_interrupt);

        *offset = 0;
        
        switch (paquete->codigo_operacion){
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_kernel);
                break;
            
            case SYSCALL_IO:
            
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);                        

                pc = _deserializar_pc(offset, paquete); 
                
                t_syscall_io _syscall_io_recibida = _deserializar_syscall_io(offset, paquete);
                
                log_info(logger_kernel, "## %d - Solicitó syscall: IO", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_io* interfaz_disponible = funcion_syscall_IO(_syscall_io_recibida.dispositivo);
                
                if(interfaz_disponible != NULL) { 
                    
                    log_info(logger_kernel, "## %d - Bloqueado por IO: %s", pid, _syscall_io_recibida.dispositivo);    
                    
                    t_pcb* _proceso_a_bloquear = _sacar_pcb_de_exec(pid);
                    _proceso_a_bloquear->pc = pc;

                    t_info_proceso_en_io* _info_proceso_bloqueado = malloc(sizeof(t_info_proceso_en_io));
                    _info_proceso_bloqueado->pid    = _proceso_a_bloquear->pid;
                    _info_proceso_bloqueado->tiempo = _syscall_io_recibida.tiempo;

                    encolar_pcb_en_interfaz(interfaz_disponible, _info_proceso_bloqueado);
    
                    encolar_pcb_en_estado(estado_blocked, _proceso_a_bloquear); 

                    sem_post(&sem_cantidad_pcbs_en_blocked);
                    // SEMAFORO PARA PLANIFICADOR LARGO PLAZO PARA QUE SALGA DE ESPERA ACTIVA

                } else {

                    _enviar_a_finalizar_proceso(pid, pc);
                }

                eliminar_paquete(paquete);
                 
                break;
            case SYSCALL_INIT_PROC:
                // de la syscall INIT_PORC recibo PID, Archivo, tamanioProceso
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);    

                log_info(logger_kernel, "## %d - Solicitó syscall: INIT_PROC", pid);

                char* archivo = deserializar_archivo_instrucciones(offset, paquete);

                int tamanio_proceso = deserializar_tamanio_proceso(offset, paquete);

                t_pcb* nuevo_proceso = iniciarPCB(archivo, tamanio_proceso, asignar_pid());

                pasar_pcb_a_new(nuevo_proceso);

                eliminar_paquete(paquete);
                break;
            case SYSCALL_DUMP_MEMORY:
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete); 

                log_info(logger_kernel, "## %d - Solicitó syscall: DUMP_MEMORY", pid);

                pc = _deserializar_pc(offset, paquete); 

                // Sacamos el proceso del estado exec y lo ponemos en blocked
                // Habria que mandarlo a Memoria y ahi que lo devuelva conexion-kernel-memorica.c
                // Hacemos ahi lo que diga la syscall, y de ahi se pasa a EXIT o vuelve a Ready

                eliminar_paquete(paquete);
                break;

            case SYSCALL_EXIT:
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete); 
                
                pc = _deserializar_pc(offset, paquete); 

                log_info(logger_kernel, "## %d - Solicitó syscall: EXIT", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                _enviar_a_finalizar_proceso(pid, pc);
                
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
    bool flag = false;
	while (!flag) {
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
            flag = true;
			break;
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
    eliminar_paquete(paquete);
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

t_syscall_io _deserializar_syscall_io(int* offset, t_paquete* paquete) 
{ 
    t_syscall_io _syscall_io_recibida;

    int len_dispositivo = 0;
    memcpy(&len_dispositivo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    char* dispositivo = malloc(len_dispositivo);
    memcpy(dispositivo, paquete->buffer->stream + *offset, len_dispositivo); *offset += len_dispositivo;
    _syscall_io_recibida.dispositivo = dispositivo;

    // Leer tiempo
    int tamanio_tiempo;
    memcpy(&tamanio_tiempo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    int tiempo;
    memcpy(&tiempo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    _syscall_io_recibida.tiempo = tiempo;

    return _syscall_io_recibida;
}

int _deserializar_pid(int* offset, t_paquete* paquete) 
{ 
    int pid = 0, tamanio_pid = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pid, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&pid, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    return pid;
}

int _deserializar_pc(int* offset, t_paquete* paquete) 
{ 
    int pc = 0, tamanio_pc = 0;
    // El PID es recibido como INT, pero deberia ser uint8_t    
    memcpy(&tamanio_pc, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&pc, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

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

void _enviar_a_finalizar_proceso(uint8_t pid, uint16_t pc)
{ 
    sem_wait(&bin_proceso_eliminar);
    char* ip_memoria = configuracion_kernel->IP_MEMORIA;
    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    // Falta realizar prueba
    t_pcb* _proceso_a_terminar = _sacar_pcb_de_exec(pid);

    _proceso_a_terminar->pc = pc;

    enviar_proceso_a_finalizar_Memoria(*_proceso_a_terminar, fd_conexion_memoria);

    manejar_conexion_kernel_memoria(fd_conexion_memoria);

}

uint8_t _recibir_handshake_de_cpu(int socket_cliente_cpu, int parte_cpu) {

    uint8_t handshake;  // Valor del CPU ID
    uint8_t resultOk = 0;
    uint8_t resultError = -1;

    recv(socket_cliente_cpu, &handshake, sizeof(uint8_t), MSG_WAITALL);
        
    if (handshake < 256 && handshake >= 0) {
        switch (parte_cpu) {
        case SOCKET_INTERRUPT:
            log_debug(logger_kernel, "Nueva conexión en Interrupt: socket %d", socket_cliente_cpu);
            break;
        
        case SOCKET_DISPATCH:
            log_debug(logger_kernel, "Nueva conexión en Dispatch: socket %d", socket_cliente_cpu);
            break;
        }
        send(socket_cliente_cpu, &resultOk, sizeof(uint8_t), 0);
    } else {
        send(socket_cliente_cpu, &resultError, sizeof(uint8_t), 0);
    }

    return handshake;
}

void _agregar_socket_en_cpu(uint8_t id_cpu, t_sockets_cpu tipo_socket, int valor_socket) {

    t_cpu_conectada* cpu_a_utilizar = _buscar_cpu_en_lista(id_cpu);

    if(cpu_a_utilizar == NULL) { 
        cpu_a_utilizar = _agregar_cpu_en_lista(id_cpu);
    }

    switch (tipo_socket) {
        case SOCKET_INTERRUPT:
            cpu_a_utilizar->socket_interrupt = valor_socket; // Asignar el socket de interrupt
            break;
        
        case SOCKET_DISPATCH:
            cpu_a_utilizar->socket_dispatch = valor_socket; // Asignar el socket de dispatch
            break;
    }
}

t_cpu_conectada* _agregar_cpu_en_lista(uint8_t id_cpu) 
{
    t_cpu_conectada* cpu_agregada = malloc(sizeof(t_cpu_conectada));

    cpu_agregada->id_cpu = id_cpu;
    cpu_agregada->socket_interrupt = -1; 
    cpu_agregada->socket_dispatch  = -1; 
    cpu_agregada->pid_en_cpu       = -1; 
    
    // PROTEGER CON MUTEX
    list_add(lista_cpus->lista_cpus, cpu_agregada);
    // PROTEGER CON MUTEX
    sem_post(&bin_cpu_disponible); // Posteo en base a los cpus disponibles -> 50000000 DE IQ

    return cpu_agregada;
}

t_cpu_conectada* _buscar_cpu_en_lista(uint8_t id_cpu)
{
    bool _cpu_tiene_id(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->id_cpu == id_cpu;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_tiene_id);
}

t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid)
{
    bool _cpu_tiene_pid(void* ptr) {
        t_cpu_conectada* cpu_con_proceso = (t_cpu_conectada*) ptr;
        return cpu_con_proceso->pid_en_cpu == pid;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_tiene_pid);
}

void liberar_cpu_de_proceso(uint8_t pid) 
{
    t_cpu_conectada* cpu_a_liberar = malloc(sizeof(t_cpu_conectada));
    
    pthread_mutex_lock(&lista_cpus->mutex_lista);
    cpu_a_liberar = _buscar_proceso_en_lista_cpu(pid);
    cpu_a_liberar->pid_en_cpu = -1;
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    log_debug(logger_kernel, "Se libero un CPU CARAJO");

    sem_post(&bin_cpu_disponible);
}

char* deserializar_archivo_instrucciones(int* offset, t_paquete* paquete) {
    int len_archivo=0;
    
    memcpy(&len_archivo, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

    char* archivo = malloc(len_archivo);
    memcpy(archivo, paquete->buffer->stream + *offset, len_archivo); *offset += len_archivo;

    return archivo;
}

int deserializar_tamanio_proceso(int* offset, t_paquete* paquete) {

    int tamanio_tamanio, tamanio;
    memcpy(&tamanio_tamanio, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);
    memcpy(&tamanio, paquete->buffer->stream + *offset, tamanio_tamanio); *offset += tamanio_tamanio;

    return tamanio;
}