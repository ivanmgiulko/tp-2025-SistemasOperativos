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

    pasar_pcb_a_new(proceso_ejemplo);
	log_info(logger_kernel,"## %d Se crea el proceso - Estado: NEW", proceso_ejemplo->pid);
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

}


p_algoritmos devolver_algoritmo_planificacion(){
    return EXIT_FAILURE;
}

void iniciar_planificador_largo_plazo(){
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
	
    iniciar_planificacion_largo_plazo();
}

void iniciar_planificacion_largo_plazo(){

    log_debug(logger_kernel, "Inicia el planificador a largoplazo");

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    pthread_t hilo_planificador_mediano_plazo;
 	pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_mediano_plazo, NULL);
	pthread_detach(hilo_planificador_mediano_plazo);

    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;


        t_pcb* proceso_ejemplo1 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
        pasar_pcb_a_new(proceso_ejemplo1);

        t_pcb* proceso_ejemplo2 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
        pasar_pcb_a_new(proceso_ejemplo2);

        t_pcb* proceso_ejemplo3 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
        pasar_pcb_a_new(proceso_ejemplo3);

        t_pcb* proceso_ejemplo4 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 4000, asignar_pid());
        pasar_pcb_a_new(proceso_ejemplo4);


    while(1){

        // El proceso llega y no hay ningun proceso en la primer cola

        bool cola_new_estaba_vacia = verificar_cola_new_estaba_vacia();  // :v

        if(cola_new_estaba_vacia) {
           
            _enviar_proceso_new_a_cola_ready();

        } else {

            if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
                
               _enviar_proceso_new_a_cola_ready();
            
            }

            if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                
                list_sort(estado_new->cola, _tiene_menos_tamanio);

                _enviar_proceso_new_a_cola_ready();
            }
        }
    }
    
}

void iniciar_planificador_mediano_plazo() {

}

// CONSUMIDOR
void iniciar_planificador_corto_plazo(){
    log_debug(logger_kernel, "Comienza hilo de corto plazo");
    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_ready);
        
        if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "FIFO") == 0){
            
            pasar_pcb_ready_a_exec(estado_ready->cola, 0);
            
            t_pcb* pcb_en_exec = pop_cola_mutex(estado_exec);

            t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); // Hacerle el free
            infoProceso->pc = pcb_en_exec->pc;
            infoProceso->pid = pcb_en_exec->pid;
            enviar_proc_cpu(*infoProceso, socket_dispatch);

            
        //  Ver "conexion-kernel-cpu ya que ahora estamos simulando que recibe una IO desde CPU"
            // t_param_io* io_recibida_cpu = (t_param_io*) manejar_cliente_dispatch(&socket_dispatch);
            // bool interfaz_disponible = funcion_syscall_IO(io_recibida_cpu->dispositivo, io_recibida_cpu->tiempo);
            // if(interfaz_disponible == true) { // La interfaz existe -> no contemplo casos de si ya esta siendo usada la IO
            //     log_info(logger_kernel, "## %d - Bloqueado por IO: %s", pcbEnReady->pid, io_recibida_cpu->dispositivo);    
            //     // sacar de exec y mandar a blocked
            //     enviar_proceso_a_io(pcbEnReady->pid, io_recibida_cpu->tiempo, socket_io);
            // } else {
            //     log_debug(logger_kernel, "LA INTERFAZ MOUSE NOOOOO ESTA DISPONIBLE!");
            // }
        }
    }
}

bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new) { 
    
    char* ip_memoria = configuracion_kernel->IP_MEMORIA;
    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    enviarProceso_A_Memoria(*pcb_en_new, fd_conexion_memoria);
            
    return manejar_conexion_kernel_memoria(fd_conexion_memoria);
}

void encolar_pcb(t_estado* estado, t_pcb* pcb) {

    pthread_mutex_lock(&(estado->mutex));
    list_add(estado->cola, pcb);
    pthread_mutex_unlock(&(estado->mutex));

}

bool verificar_cola_new_estaba_vacia() {
    pthread_mutex_lock(&(estado_new->mutex));
    bool cola_vacia = list_size(estado_new->cola) - 1 == 0;
    pthread_mutex_unlock(&(estado_new->mutex));

    return cola_vacia;
}

void  pasar_pcb_a_new(t_pcb* pcb) {
    // Encolar el PCB en la cola de NEW
    encolar_pcb(estado_new, pcb);
    log_info(logger_kernel, "## %d Pasa al estado NEW", pcb->pid);
    sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
}

void pasar_pcb_new_a_ready(t_pcb* pcb) { // Por ahora al pedo, despues cuando pasemos a lista lo utilizamos para el "find"
    // Popear el pcb pasado por parametro de forma atomica de NEW
    t_pcb* pcb_en_new = pop_cola_mutex(estado_new);
    log_info(logger_kernel, "## %d Pasa del estado NEW al estado READY", pcb_en_new->pid);
    // Pushear el pcb pasado por parametro de forma atomica en READY 
    encolar_pcb(estado_ready,pcb_en_new);
    // POSTEAR EL SEAMOFOR QUE TE AVISA QUE TENES PROCESOS EN READY (?)
    sem_post(&sem_cantidad_pcbs_en_ready); 
}

void pasar_pcb_ready_a_exec(t_pcb* pcb) { // Por ahora al pedo, despues cuando pasemos a lista lo utilizamos para el "find"
    // Popear el pcb pasado por parametro de forma atomica de NEW
    t_pcb* pcb_en_ready = pop_cola_mutex(estado_ready);
    log_info(logger_kernel, "## %d Pasa del estado READY al estado EXEC", pcb_en_ready->pid);
    // Pushear el pcb pasado por parametro de forma atomica en READY 
    encolar_pcb(estado_exec, pcb_en_ready);
    
}

t_estado* inicializar_estado() {
    // REVISAR FREE PARA MALLOC
    t_estado* estado = malloc(sizeof(t_estado));
    estado->cola = list_create();
    pthread_mutex_init(&(estado->mutex), NULL);
    return estado;
}

t_pcb* pop_cola_mutex(t_estado* cola_mutex) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = list_remove(cola_mutex->cola, 0);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* push_cola_mutex(t_estado* cola_mutex, t_pcb* pcb) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    list_add(cola_mutex->cola, pcb);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* peek_cola_mutex(t_estado* cola_mutex) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    t_pcb* pcb = list_get(cola_mutex->cola, 0);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}

t_pcb* peek_pcb_en_new() {
    sem_wait(&sem_cantidad_pcbs_en_new);
    t_pcb* pcb = peek_cola_mutex(estado_new);
    return pcb;
}

bool _tiene_menos_tamanio(void* a, void* b) { 
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->tamanioMemoria <= proceso_b->tamanioMemoria;
} 

void _enviar_proceso_new_a_cola_ready() {
    t_pcb* pcb_en_new = peek_pcb_en_new();

    bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_new);
            
    if(hay_espacio_en_memoria) { 
        pasar_pcb_new_a_ready(pcb_en_new);
    } else { 
        log_trace(logger_kernel, "El proceso %d sigue en NEW porque no hay espacio en memo", pcb_en_new->pid);
        // El proceso sigue en la cola de New
        sem_wait(&sem_hay_espacio_en_memoria); // Espera el semaforo desde kernel-memoria
        log_debug(logger_kernel, "Se libero memoria, pruebo de nuevo");
        sem_post(&sem_cantidad_pcbs_en_new);   // comienzo de nuevo el while para que ponga al proceso en Ready
    }
}