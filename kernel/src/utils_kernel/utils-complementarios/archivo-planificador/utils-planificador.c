#include "utils-planificador.h"

#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-memoria/modulo-memoria.h>

void _enviar_desde_new_a_ready() 
{ 
    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;

    bool _cola_new_estaba_vacia = _verificar_cola_new_estaba_vacia();  // :v

    if(_cola_new_estaba_vacia) {

    _enviar_proceso_new_a_cola_ready();

    } else {

        if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
            
            _enviar_proceso_new_a_cola_ready();
            
        } else if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                
            list_sort(estado_new->cola, _tiene_menos_tamanio);

            _enviar_proceso_new_a_cola_ready();
        } else {

            log_error(logger_kernel, "El algoritmo de ingreso a ready no es soportado por el Sistema Operativo...");
            exit(1);
        }
    }
}

void _enviar_desde_susp_ready_a_ready() 
{
    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;

    bool _cola_new_estaba_vacia = _verificar_cola_new_estaba_vacia();  // :v

    if(_cola_new_estaba_vacia) {
           
        _enviar_proceso_susp_ready_a_cola_ready();

    } else {

         if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
                
            _enviar_proceso_susp_ready_a_cola_ready();
            
        }else if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                
            list_sort(estado_susp_ready->cola, _tiene_menos_tamanio);

            _enviar_proceso_susp_ready_a_cola_ready();
        } else {

            log_error(logger_kernel, "El algoritmo de ingreso a ready no es soportado por el Sistema Operativo...");
            exit(1);
        }
    }
}

void _iniciar_cuando_apreta_enter() 
{ 
    char *leido;
	bool lineaVacia = false;
	do {
		leido = readline("> ");
		if(strcmp(leido, "") == 0){
			lineaVacia = !lineaVacia;
            // Se rompe el While e inicia el planificador a largo plazo
		} else {
			free(leido);
		}
	} while(!lineaVacia);
    free(leido);

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    pthread_t hilo_planificador_mediano_plazo;
 	pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_mediano_plazo, NULL);
	pthread_detach(hilo_planificador_mediano_plazo);

}

void crear_proceso_cero(char* path, int tamanio)
{
    inicializar_pid();

    uint64_t estimacion_inical = atoi(configuracion_kernel->ESTIMACION_INICIAL);

  	t_pcb* proceso_cero = iniciarPCB(path ,tamanio, asignar_pid(), estimacion_inical);
    pasar_pcb_a_new(proceso_cero);
	log_info(logger_kernel,"%d Se crea el proceso - Estado: NEW", proceso_cero->pid);
}

void inicializar_pid(){
   pid_contador = inicializar_contador();
}

uint8_t asignar_pid(){
    pthread_mutex_lock(&pid_contador->mutex);
    uint8_t valor_pid = pid_contador->valor++;
    pthread_mutex_unlock(&pid_contador->mutex);
    return valor_pid;
}

void inicializar_estructuras(char* path_relativo, char* path_config)
{
    logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); 

    char* path_relativo_config = string_duplicate(path_relativo);
    string_append(&path_relativo_config, path_config);

	configuracion_kernel = crear_config_kernel(path_relativo_config, logger_kernel);

    // INICIAMOS LISTA DE CPU E IOs
    lista_cpus = malloc(sizeof(t_lista_cpus));
    lista_cpus->lista_cpus = list_create();
    pthread_mutex_init(&lista_cpus->mutex_lista, NULL);

    lista_de_io = malloc(sizeof(t_lista_io));
    lista_de_io->lista_ios = list_create();
	pthread_mutex_init(&lista_de_io->mutex_lista, NULL);
   
    // INICIAMOS SEMAFOROS
    sem_init(&sem_cantidad_pcbs_en_new, 0, 0);
    sem_init(&sem_puede_replanificar_srt, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_ready, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_blocked, 0, 0);
 
    sem_init(&bin_susp_blocked, 0, 0);
    sem_init(&bin_eliminar_procesos_en_interfaces, 0, 0);
    sem_init(&bin_proceso_eliminar, 0, 1);
    sem_init(&bin_cpu_disponible, 0, 0);

    // INICIAMOS LOS ESTADOS DE LOS PROCESOS
    estado_new          = inicializar_estado();
    estado_ready        = inicializar_estado();
    estado_susp_ready   = inicializar_estado();
    estado_exec         = inicializar_estado();
    estado_blocked      = inicializar_estado();
    estado_blocked_aux  = inicializar_estado();
    estado_susp_blocked = inicializar_estado();
    estado_exit         = inicializar_estado();
}

bool _chequear_interfaz_disponible(t_io* interfaz) {
    bool interfaz_disponible;
    pthread_mutex_lock(&lista_de_io->mutex_lista);
    interfaz_disponible = interfaz->enabled;
    pthread_mutex_unlock(&lista_de_io->mutex_lista);
    return interfaz_disponible;
}

bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new) 
{ 
    int fd_conexion_memoria = crear_conexion(configuracion_kernel->IP_MEMORIA, configuracion_kernel->PUERTO_MEMORIA);

    uint32_t resultado_handshake;
    uint32_t t_modulo = 0;
    send(fd_conexion_memoria, &t_modulo, sizeof(uint32_t), 0);
    recv(fd_conexion_memoria, &resultado_handshake, sizeof(uint32_t), MSG_WAITALL);

    if(resultado_handshake == 1){
        enviar_proceso_a_memoria(*pcb_en_new, fd_conexion_memoria, PROCESO_MEMORIA);
    }

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

    if(list_is_empty(cola_mutex->cola)) {
        pthread_mutex_unlock(&(cola_mutex->mutex));
        return NULL;
    }

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

t_pcb* peek_cola_mutex(t_estado* cola_mutex, uint8_t indice) 
{
    t_pcb* pcb = NULL;
    pthread_mutex_lock(&(cola_mutex->mutex));
    if(list_size(cola_mutex->cola) > indice) {
        pcb = list_get(cola_mutex->cola, indice);
    }
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return pcb;
}


bool _tiene_menos_tamanio(void* a, void* b) 
{ 
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->tamanioMemoria <= proceso_b->tamanioMemoria;
} 

void _enviar_proceso_new_a_cola_ready() {
    
    t_pcb* pcb_en_new = peek_cola_mutex(estado_new, 0);

    if(pcb_en_new != NULL) {
        bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_new);
            
        if(hay_espacio_en_memoria) { 
            pcb_en_new = pop_cola_mutex(estado_new);
            pasar_pcb_new_a_ready(pcb_en_new);
        } else { 
            // El proceso sigue en la cola de New
            sem_wait(&sem_hay_espacio_en_memoria); // Espera el semaforo desde kernel-memoria
        }
    }
}

void _enviar_proceso_susp_ready_a_cola_ready() 
{
    t_pcb* pcb_en_susp_ready = peek_cola_mutex(estado_susp_ready, 0);

    if(pcb_en_susp_ready != NULL) {
        bool hay_espacio_en_memoria = preguntar_a_memoria_espacio(pcb_en_susp_ready);
            
        if(hay_espacio_en_memoria) { 
            pcb_en_susp_ready = pop_cola_mutex(estado_susp_ready);
            pasar_pcb_susp_ready_a_ready(pcb_en_susp_ready);
        } else { 
            // El proceso sigue en la cola de New
            sem_wait(&sem_hay_espacio_en_memoria); // Espera el semaforo desde kernel-memoria
        }
    }
}

bool _menor_estimacion(void* a, void* b) {
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->estimacion_parcial <= proceso_b->estimacion_parcial;
}

void* _mayor_estimacion(void* a, void* b) 
{
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->estimacion_parcial > proceso_b->estimacion_parcial ? proceso_a : proceso_b;
}

void planificar_con_fifo() 
{
    t_cpu_conectada* cpu_libre = NULL;

    cpu_libre = _buscar_cpu_libre();
    
    sem_wait(&sem_cantidad_pcbs_en_ready);

    t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   
    
    enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);
     
}

void planificar_con_sjf()
{
    t_cpu_conectada* cpu_libre = NULL;

    cpu_libre = _buscar_cpu_libre();
    
    sem_wait(&sem_cantidad_pcbs_en_ready);

    pthread_mutex_lock(&estado_ready->mutex);
    list_sort(estado_ready->cola, _menor_estimacion);
    pthread_mutex_unlock(&estado_ready->mutex);

    t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   
    
    enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);
}

t_pcb* proceso_a_desalojar() 
{
    t_pcb* pcb = NULL;
    pthread_mutex_lock(&estado_exec->mutex);
    if(!list_is_empty(estado_exec->cola)) {
        pcb = list_get_maximum(estado_exec->cola, _mayor_estimacion);
    }
    pthread_mutex_unlock(&estado_exec->mutex);
    return pcb;
}


void planificar_con_srt()
{
    
    pthread_mutex_lock(&estado_ready->mutex);
    list_sort(estado_ready->cola, _menor_estimacion);
    pthread_mutex_unlock(&estado_ready->mutex);
    
    t_pcb* pcb_a_operar = peek_cola_mutex(estado_ready, 0);   
    if(pcb_a_operar != NULL){
        t_cpu_conectada* cpu_libre = NULL;
        cpu_libre = _buscar_cpu_libre();
    
        if(cpu_libre == NULL) {
            t_pcb* _proceso_con_mayor_estimacion = proceso_a_desalojar();
            
            if(_proceso_con_mayor_estimacion != NULL && pcb_a_operar->estimacion_parcial < _proceso_con_mayor_estimacion->estimacion_parcial) {
                
                t_cpu_conectada* cpu_de_proceso_a_desalojar = buscar_cpu_que_usa_proceso(lista_cpus->lista_cpus, _proceso_con_mayor_estimacion->pid);
                enviar_pid_a_desalojar(cpu_de_proceso_a_desalojar->socket_interrupt);    
            }
    
        }else{
            enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);
        } 
    }
}

void iniciar_temporizador_suspblocked(void* pcb){
    t_pcb* _proceso_bloqueado = (t_pcb*)pcb;
    int64_t tiempo_maximo_espera = strtoll(configuracion_kernel->TIEMPO_SUSPENSION, NULL, 10);

    uint8_t pid_aux = _proceso_bloqueado->pid;

    usleep(tiempo_maximo_espera * 1000);
    
    if(buscar_proceso_en_cola(estado_blocked, pid_aux) != NULL){
        
        pasar_pcb_blocked_a_suspblocked(_proceso_bloqueado);
        

        int fd_conexion_memoria = crear_conexion(configuracion_kernel->IP_MEMORIA, configuracion_kernel->PUERTO_MEMORIA);
    
        uint32_t resultado_handshake;
        uint32_t t_modulo = 0;
        send(fd_conexion_memoria, &t_modulo, sizeof(uint32_t), 0);
        recv(fd_conexion_memoria, &resultado_handshake, sizeof(uint32_t), MSG_WAITALL);

        if(resultado_handshake == 1){
            enviar_a_liberar_memoria(fd_conexion_memoria, *_proceso_bloqueado);
            manejar_conexion_kernel_memoria(fd_conexion_memoria);   
        }

    }
    pthread_exit(NULL);
}

void enviar_a_ejecutar_proceso(t_cpu_conectada* cpu, t_pcb* pcb) {
    
    _sacar_pcb_de_cola(pcb->pid, estado_ready);
    
    pasar_pcb_ready_a_exec(pcb);
    
    t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); 
    infoProceso->pc = pcb->pc;
    infoProceso->pid = pcb->pid;

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    cpu->pid_en_cpu = pcb->pid;
    pthread_mutex_unlock(&lista_cpus->mutex_lista);
    enviar_proc_cpu(*infoProceso, cpu->socket_dispatch);
    free(infoProceso);

}

void iniciar_swap_out() {

    while(1) {
    
        sem_wait(&bin_susp_blocked);
    
        _enviar_desde_susp_ready_a_ready();
    
    }
}