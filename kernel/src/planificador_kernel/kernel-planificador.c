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

t_contador* pid;

sem_t sem_cantidad_pcbs_en_new;
sem_t sem_cantidad_pcbs_en_ready;
sem_t sem_hay_espacio_en_memoria;

t_pcb* crear_proceso_cero(char* path, int tamanio){
    
    inicializar_pid();
  	t_pcb* proceso_ejemplo = iniciarPCB(path ,tamanio, asignar_pid());
    printf("Creando proceso cero con path %s, tamanio %d y pid %d\n", path, tamanio, proceso_ejemplo->pid);
    
    pasar_pcb_a_new(proceso_ejemplo);
	log_obligatorio(logger_kernel, proceso_ejemplo->pid, " Se crea el proceso - Estado: NEW");
    return proceso_ejemplo;
}

void inicializar_pid(){
   pid = inicializar_contador();
}

int asignar_pid(){
    pthread_mutex_lock(&pid->mutex);
    int valor_pid = pid->valor++;
    pthread_mutex_unlock(&pid->mutex);
    return valor_pid;
}

void inicializar_estructuras(){
    estado_new = inicializar_estado();
    estado_ready = inicializar_estado();
    estado_susp_ready = inicializar_estado();
    estado_exec = inicializar_estado();
    estado_blocked = inicializar_estado();
    estado_susp_blocked = inicializar_estado();
    estado_exit = inicializar_estado();

    sem_post(&sem_hay_espacio_en_memoria);
}


p_algoritmos devolver_algoritmo_planificacion(){
    
}

void iniciar_planificador_largoPlazo(){
	char *leido;
	bool lineaVacia = false;
	do{
		leido = readline("> ");
		if(strcmp(leido, "") == 0){
			lineaVacia = !lineaVacia;
            // Se rompe el While e inicia el planificador a largo plazo
		}else{
			free(leido);
		}
	}while(!lineaVacia);
	
    iniciar_planificacion_largoPlazo();
}

void iniciar_planificacion_largoPlazo(){

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_cortoPlazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    // pthread_t hilo_planificador_mediano_plazo;
 	// pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_medianoPlazo, NULL);
	// pthread_detach(hilo_planificador_mediano_plazo);

    log_trace(logger_kernel, "Inicia el planificador a largoplazo");
    // Habria que poner un while(1) para que esto siempre este ejecutandose?

    while(1){

        t_pcb* pcb_en_new = peek_pcb_en_new();
        
        bool cola_new_estaba_vacia = verificar_cola_new_estaba_vacia();  // :v

        char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;

        log_error(logger_kernel, "TENGO UN PROCESO");
        
        if(cola_new_estaba_vacia) {
           
            bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_new, fd_conexion_memoria);
            
            if(hay_espacio_en_memoria) { 
                pasar_pcb_new_a_ready(pcb_en_new);
                log_info(logger_kernel, "%d Pasa del estado NEW al estado READY", pcb_en_new->pid);
                sem_post(&sem_cantidad_pcbs_en_ready); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
                
                // Como podemos testear esta shit:
                /*
                . El 2do proceso que llega no tiene espacio
                . esperamos a que finalice el primero
                . finaliza el primero
                . hacemos que el 2do pase a Ready
                . dios se apiade de nosotros
                */

            } else { 
                log_trace(logger_kernel, "El proceso %d sigue en NEW porque no hay espacio en memo", pcb_en_new->pid);
                // El proceso sigue en la cola de New
            }

        } else {

            if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
                
                // sem_wait(&sem_hay_espacio_en_memoria);  // Le avisa al planificador cuando no hay espacio en memo, asi evitamos la espera activa
                
                // hacemos peek al primero proceso
                // vemos si hay espacio en memoria
                // pasamos a ready
            }

            if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                // ordenamos la cola (futura lista)
                // hacemos peek al primero proceso
                // vemos si hay espacio en memoria
                // pasamos a ready
            }
        }
    }
    
}

// CONSUMIDOR
void iniciar_planificador_cortoPlazo(){
    log_trace(logger_kernel, "Comienza hilo de corto plazo");
    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_ready);
        if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "FIFO") == 0){
            
            t_pcb* pcbEnReady = queue_pop(estado_ready->cola);
            log_info(logger_kernel, "%d Pasa del estado READY al estado EXEC", pcbEnReady->pid);
            t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); // Hacerle el free
            infoProceso->pc = pcbEnReady->pc;
            infoProceso->pid = pcbEnReady->pid;
            enviar_proc_cpu(*infoProceso, socket_dispatch);

            // Ver "conexion-kernel-cpu ya que ahora estamos simulando que recibe una IO desde CPU"
            t_param_io* io_recibida_cpu = (t_param_io*) manejar_cliente_dispatch(&socket_dispatch);
            bool interfaz_disponible = funcion_syscall_IO(io_recibida_cpu->dispositivo, io_recibida_cpu->tiempo);
            if(interfaz_disponible == true) { // La interfaz existe -> no contemplo casos de si ya esta siendo usada la IO
                log_info(logger_kernel, "%d - Bloqueado por IO: %s", pcbEnReady->pid, io_recibida_cpu->dispositivo);    
                // sacar de ready y mandar a blocked
                enviar_proceso_a_io(pcbEnReady->pid, io_recibida_cpu->tiempo, socket_io);
            } else {
                log_debug(logger_kernel, "LA INTERFAZ MOUSE NOOOOO ESTA DISPONIBLE!");
            }
        }
    }
}

void encolar_pcb(t_estado* estado, t_pcb* pcb) {

    pthread_mutex_lock(&(estado->mutex));
    queue_push(estado->cola, pcb);
    pthread_mutex_unlock(&(estado->mutex));

    log_debug(logger_kernel, "PCB con PID %d encolado en el estado.", pcb->pid);
}

bool verificar_cola_new_estaba_vacia() {
    pthread_mutex_lock(&(estado_new->mutex));
    bool cola_vacia =  queue_size(estado_new->cola) - 1 == 0;
    pthread_mutex_unlock(&(estado_new->mutex));

    return cola_vacia;
}

void pasar_pcb_a_new(t_pcb* pcb) {
    // Encolar el PCB en la cola de NEW
    encolar_pcb(estado_new, pcb);
    log_info(logger_kernel, "%d Pasa al estado NEW", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_new_a_ready(t_pcb* pcb) { // Por ahora al pedo, despues cuando pasemos a lista lo utilizamos para el "find"
    
    // Popear el pcb pasado por parametro de forma atomica de NEW
    t_pcb* pcb_en_new = pop_cola_mutex(estado_new);
    // Pushear el pcb pasado por parametro de forma atomica en READY 
    encolar_pcb(estado_ready,pcb_en_new);
    // POSTEAR EL SEAMOFOR QUE TE AVISA QUE TENES PROCESOS EN READY (?)
    //  sem_post(&sem_cantidad_pcbs_en_ready); 
}

t_estado* inicializar_estado() {
    // REVISAR FREE PARA MALLOC
    t_estado* estado = malloc(sizeof(t_estado));
    estado->cola = queue_create();
    pthread_mutex_init(&(estado->mutex), NULL);
    return estado;
}

t_pcb* pop_cola_mutex(t_estado* cola_mutex) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = queue_pop(cola_mutex->cola);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* push_cola_mutex(t_estado* cola_mutex, t_pcb* pcb) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    queue_push(cola_mutex->cola, pcb);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* peek_cola_mutex(t_estado* cola_mutex) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = queue_peek(cola_mutex->cola);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* peek_pcb_en_new() {
    sem_wait(&sem_cantidad_pcbs_en_new);
    t_pcb* pcb = peek_cola_mutex(estado_new);
    return pcb;
}

bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new, int fd_conexion_memoria) { 
    fd_conexion_memoria = crear_conexion(configuracion_kernel->IP_MEMORIA, configuracion_kernel->PUERTO_MEMORIA);
            
    // enviar_tamanioProceso(tam_proceso, fd_conexion_memoria);
    enviarProceso_A_Memoria(*pcb_en_new, fd_conexion_memoria);
            
    // Recibo respuesta por parte de memo si es que hay memoria
    return  manejar_conexion_kernel_memoria(fd_conexion_memoria);
}