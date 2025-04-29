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

pthread_mutex_t mutex_pid;
sem_t sem_cantidad_pcbs_en_new;

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

    log_info(logger_kernel, "Inicia el planificador a largoplazo");

    // 1) Verificar si cola esta vacia
    //      2.1) Cola vacia = preguntar espacio a memoria
    //          3.1) Memoria con espacio = pasar de new a ready
    //          3.2) Memoria sin espacio = queda proceso en new
    //      2.2) Cola con procesos = se pasa de new a ready (o no) segun algoritmo de planificacion 


    bool cola_vacia = queue_is_empty(estado_new->cola);
    if(cola_vacia){
        char* ip_memoria = configuracion_kernel->IP_MEMORIA;
	    char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
	    fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
        char* tam_proceso = string_itoa(sizeof(pcb));
        enviar_tamanioProceso(tam_proceso, fd_conexion_memoria);

    //     if("hay espacio en memoria"){
    //         encolar_pcb(&estado_new, pcb);
    //         sem_post(&sem_cantidad_pcbs_en_new);
    //     } else {

    //     }
    // } else {
    //     encolar_pcb(&estado_new, pcb);
    //     sem_post(&sem_cantidad_pcbs_en_new);
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
