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

t_pcb* crear_proceso_cero(char* path, int tamanio){
    inicializar_pid();
    int nuevo_pid = asignar_pid();
  
    printf("Creando proceso cero con path %s, tamanio %d y pid %d\n", path, tamanio, nuevo_pid);

  	t_pcb* proceso_ejemplo = iniciarPCB(path,tamanio, asignar_pid());
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

    bool cola_vacia = queue_is_empty(estado_new->cola);
    if(cola_vacia){
        // Creo conexion a memo y le envio el tamanio del proceso
        char* ip_memoria = configuracion_kernel->IP_MEMORIA;
	    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
	    fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

        // enviar_tamanioProceso(tam_proceso, fd_conexion_memoria);
        enviarProceso_A_Memoria(*pcb, fd_conexion_memoria);

        // Recibo respuesta por parte de memo si es que hay memoria
        if(manejar_conexion_kernel_memoria(fd_conexion_memoria) == true) { 
            encolar_pcb(estado_ready, pcb);
            log_info(logger_kernel, "%d Pasa del estado NEW al estado READY", pcb->pid);
            sem_post(&sem_cantidad_pcbs_en_new); // Le avisa al planificador cuando hay un proceso en NEW, asi evitamos la espera activa
        } else { 
            log_trace(logger_kernel, "El proceso %d sigue en READY porque no hay espacio en memo", pcb->pid);
            // El proceso sigue en la cola de New
        }

    } else {
        if(configuracion_kernel->ALGORITMO_INGRESO_A_READY == FIFO) { 
            
        }
        // La cola no estaba vacia cuando llego el proceso
        // En base al algoritmo elegimos el siguiente proceso
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

// CONSUMIDOR
void pasar_de_new_a_ready(){

    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en NEW
        sem_wait(&sem_cantidad_pcbs_en_new);
    }
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
