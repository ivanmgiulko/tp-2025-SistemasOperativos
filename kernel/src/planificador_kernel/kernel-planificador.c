#include "kernel-planificador.h"

void decir_algoritmo(){
    printf("%s", configuracion_kernel->ALGORITMO_INGRESO_A_READY);
}

t_estado* estado_new;
t_estado* estado_ready;
t_estado* estado_susp_ready;
t_estado* estado_exec;
t_estado* estado_blocked;
t_estado* estado_susp_blocked;
t_estado* estado_exit;

t_contador* pid_contador;

sem_t sem_cantidad_pcbs_en_new;
sem_t sem_cantidad_pcbs_en_ready;
sem_t sem_cantidad_pcbs_en_blocked;
sem_t bin_proceso_bloqueado;
sem_t sem_cantidad_pcbs_en_susp_ready;

sem_t sem_hay_espacio_en_memoria;

void crear_proceso_cero(char* path, int tamanio){
    
    inicializar_pid();
  	t_pcb* proceso_ejemplo = iniciarPCB(path ,tamanio, asignar_pid());

    pasar_pcb_a_new(proceso_ejemplo);
	log_info(logger_kernel,"## %d Se crea el proceso - Estado: NEW", proceso_ejemplo->pid);
    
}

void inicializar_pid(){
   pid_contador = inicializar_contador();
}

int asignar_pid(){
    pthread_mutex_lock(&pid_contador->mutex);
    int valor_pid = pid_contador->valor++;
    pthread_mutex_unlock(&pid_contador->mutex);
    return valor_pid;
}

void inicializar_estructuras()
{
    logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); 

	configuracion_kernel = crear_config_kernel("./kernel.config", logger_kernel);

    sem_init(&sem_cantidad_pcbs_en_new, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_ready, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_blocked, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_susp_ready, 0, 0);

    sem_init(&bin_proceso_bloqueado, 0, 1);

    estado_new = inicializar_estado();
    estado_ready = inicializar_estado();
    estado_susp_ready = inicializar_estado();
    estado_exec = inicializar_estado();
    estado_blocked = inicializar_estado();
    estado_susp_blocked = inicializar_estado();
    estado_exit = inicializar_estado();
}


p_algoritmos devolver_algoritmo_planificacion(){
    return EXIT_FAILURE;
}

void iniciar_planificacion_largo_plazo(){

    _iniciar_cuando_apreta_enter();

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    pthread_t hilo_planificador_mediano_plazo;
 	pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_mediano_plazo, NULL);
	pthread_detach(hilo_planificador_mediano_plazo);

    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;

    t_pcb* proceso_ejemplo1 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
    pasar_pcb_a_susp_ready(proceso_ejemplo1);

    t_pcb* proceso_ejemplo2 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
    pasar_pcb_a_susp_ready(proceso_ejemplo2);        

    t_pcb* proceso_ejemplo3 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 400, asignar_pid());
    pasar_pcb_a_new(proceso_ejemplo3);

    t_pcb* proceso_ejemplo4 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 400, asignar_pid());
    pasar_pcb_a_new(proceso_ejemplo4);

    while(1){

        bool _cola_susp_ready_esta_vacia = _verificar_cola_susp_ready_esta_vacia();
        
        bool _cola_new_estaba_vacia = _verificar_cola_new_estaba_vacia();  // :v

        if(_cola_susp_ready_esta_vacia) { 

            _enviar_desde_new_a_ready(_cola_new_estaba_vacia, algortimo_ingreso_ready);

        } else { 

            _enviar_desde_susp_ready_a_ready(_cola_new_estaba_vacia, algortimo_ingreso_ready);

        }
    }
}

void iniciar_planificador_mediano_plazo() {

    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_blocked);
        sem_wait(&bin_proceso_bloqueado);

        t_pcb* _proceso_bloquedo = peek_cola_mutex(estado_blocked);

        t_io* _io_que_usa_pcb_bloqueado = buscar_io_en_lista(lista_de_io, _proceso_bloquedo->pid);

        if(_io_que_usa_pcb_bloqueado->enabled == true) {
            _io_que_usa_pcb_bloqueado->enabled = false;
            enviar_proceso_a_io_para_bloqueo(_proceso_bloquedo->pid, _io_que_usa_pcb_bloqueado->tiempo_ultimo_bloqueo, _io_que_usa_pcb_bloqueado->socket);
        } else { 
            // El proceso sigue bloqueado
            
        }

       
        
        // Cuando finaliza un proceso de usar la IO, puedo usar el ?mismo? semaforo para que otro la use

        // enviar_proceso_a_io_para_bloqueo(_proceso_bloquedo->pid, _syscall_io_recibida.tiempo, socket_io);

    }
}

// CONSUMIDOR
void iniciar_planificador_corto_plazo(){
    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_ready);
        
        if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "FIFO") == 0){
            
            t_pcb* pcb_en_ready = pop_cola_mutex(estado_ready);   

            pasar_pcb_ready_a_exec(pcb_en_ready);
            
            t_pcb* pcb_en_exec = peek_cola_mutex(estado_exec);

            t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); // Hacerle el free
            infoProceso->pc = pcb_en_exec->pc;
            infoProceso->pid = pcb_en_exec->pid;

            enviar_proc_cpu(*infoProceso, socket_dispatch);
        }
    }
}

bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new) 
{ 
    char* ip_memoria = configuracion_kernel->IP_MEMORIA;
    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    enviarProceso_A_Memoria(*pcb_en_new, fd_conexion_memoria);

    return manejar_conexion_kernel_memoria(fd_conexion_memoria);
}

bool _verificar_cola_new_estaba_vacia() 
{
    pthread_mutex_lock(&(estado_new->mutex));
    bool cola_vacia = list_size(estado_new->cola) - 1 == 0;
    pthread_mutex_unlock(&(estado_new->mutex));
    return cola_vacia;
}

bool _verificar_cola_susp_ready_esta_vacia() { 
    pthread_mutex_lock(&(estado_susp_ready->mutex));
    bool cola_vacia = list_size(estado_susp_ready->cola) == 0;
    pthread_mutex_unlock(&(estado_susp_ready->mutex));
    return cola_vacia;
}

void pasar_pcb_a_new(t_pcb* pcb) 
{
    encolar_pcb_en_estado(estado_new, pcb);
    pcb->estadoProceso = NEW;
    log_info(logger_kernel, "## %d Pasa al estado NEW", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_a_susp_ready(t_pcb* pcb) 
{
    encolar_pcb_en_estado(estado_susp_ready, pcb);
    pcb->estadoProceso = SUSP_READY;
    log_info(logger_kernel, "## %d Pasa al estado SUSP-READY", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_susp_ready); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_new_a_ready(t_pcb* pcb)
{ 
    encolar_pcb_en_estado(estado_ready, pcb);
    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado NEW al estado READY", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_ready); 
}

void pasar_pcb_susp_ready_a_ready(t_pcb* pcb) { 
    encolar_pcb_en_estado(estado_ready, pcb);
    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado SUSP-READY al estado READY", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_ready); 
}

void pasar_pcb_blocked_a_ready(t_pcb* pcb) { 
    encolar_pcb_en_estado(estado_ready, pcb);
    pcb->estadoProceso = READY;
    log_info(logger_kernel, "## %d Pasa del estado BLOCKED al estado READY", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_ready); 
}

void pasar_pcb_ready_a_exec(t_pcb* pcb) 
{ 
    encolar_pcb_en_estado(estado_exec, pcb);
    pcb->estadoProceso = EXEC;
    log_info(logger_kernel, "## %d Pasa del estado READY al estado EXEC", pcb->pid);
}

void pasar_de_exec_a_blocked(t_pcb* pcb){
    encolar_pcb_en_estado(estado_blocked, pcb);
    pcb->estadoProceso = BLOCKED;
    log_info(logger_kernel, "## %d Pasa del estado EXEC al estado BLOCKED", pcb->pid);
}

t_estado* inicializar_estado() 
{
    // REVISAR FREE PARA MALLOC
    t_estado* estado = malloc(sizeof(t_estado));
    estado->cola = list_create();
    pthread_mutex_init(&(estado->mutex), NULL);
    return estado;
}

t_pcb* pop_cola_mutex(t_estado* cola_mutex) 
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = list_remove(cola_mutex->cola, 0);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

void encolar_pcb_en_estado(t_estado* estado, t_pcb* pcb) 
{
    pthread_mutex_lock(&(estado->mutex));
    list_add(estado->cola, pcb);
    pthread_mutex_unlock(&(estado->mutex));
}

t_pcb* peek_cola_mutex(t_estado* cola_mutex) 
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = list_get(cola_mutex->cola, 0);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* peek_pcb_en_new() 
{
    sem_wait(&sem_cantidad_pcbs_en_new);
    t_pcb* pcb = peek_cola_mutex(estado_new);
    return pcb;
}

t_pcb* peek_pcb_en_susp_ready() 
{
    sem_wait(&sem_cantidad_pcbs_en_susp_ready);
    t_pcb* pcb = peek_cola_mutex(estado_susp_ready);
    return pcb;
}

bool _tiene_menos_tamanio(void* a, void* b) 
{ 
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->tamanioMemoria <= proceso_b->tamanioMemoria;
} 

void _enviar_proceso_new_a_cola_ready() {
    t_pcb* pcb_en_new = peek_pcb_en_new();

    bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_new);
            
    if(hay_espacio_en_memoria) { 
        pcb_en_new = pop_cola_mutex(estado_new);
        pasar_pcb_new_a_ready(pcb_en_new);
    } else { 
        log_trace(logger_kernel, "El proceso %d sigue en NEW porque no hay espacio en memo", pcb_en_new->pid);
        // El proceso sigue en la cola de New
        sem_wait(&sem_hay_espacio_en_memoria); // Espera el semaforo desde kernel-memoria
        sem_post(&sem_cantidad_pcbs_en_new);   // comienzo de nuevo el while para que ponga al proceso en Ready
    }
}

void _enviar_proceso_susp_ready_a_cola_ready() 
{
    t_pcb* pcb_en_susp_ready = peek_pcb_en_susp_ready();

    bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_susp_ready);
            
    if(hay_espacio_en_memoria) { 
        pcb_en_susp_ready = pop_cola_mutex(estado_susp_ready);
        pasar_pcb_susp_ready_a_ready(pcb_en_susp_ready);
    } else { 
        log_trace(logger_kernel, "El proceso %d sigue en SUSP-NEW porque no hay espacio en memo", pcb_en_susp_ready->pid);
        // El proceso sigue en la cola de New
        sem_wait(&sem_hay_espacio_en_memoria);        // Espera el semaforo desde kernel-memoria
        sem_post(&sem_cantidad_pcbs_en_susp_ready);   // comienzo de nuevo el while para que ponga al proceso en Ready
    }
}