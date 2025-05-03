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

t_pcb* crear_proceso_cero(char* path, int tamanio){
    inicializar_pid();
    
  	t_pcb* proceso_ejemplo = iniciarPCB(path,tamanio, asignar_pid());
    printf("Creando proceso cero con path %s, tamanio %d y pid %d\n", path, tamanio, proceso_ejemplo->pid);
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
}


p_algoritmos devolver_algoritmo_planificacion(){
    
}

void iniciar_planificador_largoPlazo(t_pcb* pcb_inicial){
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
	
    iniciar_planificacion_largoPlazo(pcb_inicial);
}

void iniciar_planificacion_largoPlazo(t_pcb* pcb){

    inicializar_estructuras();

    log_trace(logger_kernel, "Inicia el planificador a largoplazo");
    // Habria que poner un while(1) para que esto siempre este ejecutandose?
    sem_post(&sem_cantidad_pcbs_en_new);
    while(1){
        sem_wait(&sem_cantidad_pcbs_en_new);

        bool cola_vacia = queue_is_empty(estado_new->cola);
        encolar_pcb(estado_new, pcb);
        if(cola_vacia){
            // Creo conexion a memo y le envio el tamanio del proceso
            char* ip_memoria = configuracion_kernel->IP_MEMORIA;
	        char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
	        fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            t_pcb* pcbDeNew = queue_pop(estado_new->cola);

            // enviar_tamanioProceso(tam_proceso, fd_conexion_memoria);
            enviarProceso_A_Memoria(*pcbDeNew, fd_conexion_memoria);

            // Recibo respuesta por parte de memo si es que hay memoria
            if(manejar_conexion_kernel_memoria(fd_conexion_memoria) == true) { 
                encolar_pcb(estado_ready, pcbDeNew);
                log_info(logger_kernel, "%d Pasa del estado NEW al estado READY", pcb->pid);
                sem_post(&sem_cantidad_pcbs_en_ready); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
            } else { 
                encolar_pcb(estado_new, pcbDeNew);
                log_trace(logger_kernel, "El proceso %d sigue en READY porque no hay espacio en memo", pcb->pid);
                // El proceso sigue en la cola de New
            }
        } else {
            if(configuracion_kernel->ALGORITMO_INGRESO_A_READY == FIFO) { 
                // La cola no estaba vacia cuando llego el proceso
                // En base al algoritmo elegimos el siguiente proceso
            }

            if(configuracion_kernel->ALGORITMO_INGRESO_A_READY == PMCP) {

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
        
    if (configuracion_kernel->ALGORITMO_CORTO_PLAZO == FIFO){
        t_pcb* pcbEnReady = queue_pop(estado_ready->cola);
        log_info(logger_kernel, "%d Pasa del estado READY al estado EXEC", pcbEnReady->pid);
        t_peticion_instruccion* infoProceso;
        infoProceso->pc = pcbEnReady->pc;
        infoProceso->pid = pcbEnReady->pid;
        enviarProcReady_A_CPU_Dispatch(*infoProceso, fd_conexion_memoria);


    }


        // switch (configuracion_kernel->ALGORITMO_CORTO_PLAZO) {
        // case FIFO:
        //     t_pcb* pcbEnReady = queue_pop(estado_ready->cola);
        //     log_info(logger_kernel, "%d Pasa del estado READY al estado EXEC", pcbEnReady->pid);
        //     t_peticion_instruccion* infoProceso;
        //     infoProceso->pc = pcbEnReady->pc;
        //     infoProceso->pid = pcbEnReady->pid;

        //     break;
        // case SJF:
        //     break;
        // case SJF_SIN_DESALOJO:
        //     break;
        // default:
        // log_warning(logger_kernel, "El algortimo de planificacion corto plazo no es admitido");
        //     break;
        // }
    }
}

// PRODUCTOR
void pasar_a_new(t_pcb* pcb){
    bool cola_vacia = queue_is_empty(estado_new->cola);
    if(cola_vacia){
            if("hay espacio en memoria"){
                encolar_pcb(&estado_new, pcb);
                sem_post(&sem_cantidad_pcbs_en_new);
            }else{

            }
    }else{
        encolar_pcb(&estado_new, pcb);
        sem_post(&sem_cantidad_pcbs_en_new);
    }

}

void encolar_pcb(t_estado* estado , t_pcb* pcb) {
    pthread_mutex_lock(&(estado->mutex));
    queue_push(estado->cola, pcb);
    pthread_mutex_unlock(&(estado->mutex));
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
