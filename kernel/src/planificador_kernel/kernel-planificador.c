#include "kernel-planificador.h"
#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>

t_estado* estado_new;
t_estado* estado_ready;
t_estado* estado_susp_ready;
t_estado* estado_exec;
t_estado* estado_blocked;
t_estado* estado_blocked_aux; // Ver en planificador mediano plazo
t_estado* estado_susp_blocked;
t_estado* estado_exit;

t_contador* pid_contador;

sem_t sem_cantidad_pcbs_en_new;
sem_t sem_cantidad_pcbs_en_ready;
sem_t sem_cantidad_pcbs_en_blocked;
sem_t sem_cantidad_pcbs_en_susp_ready;

sem_t bin_proceso_eliminar;
sem_t bin_cpu_disponible;

sem_t sem_hay_espacio_en_memoria;

t_temporal* tiempo_esperando;

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

t_lista_cpus* lista_cpus = NULL;

void inicializar_estructuras()
{
    logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); 

	configuracion_kernel = crear_config_kernel("./kernel.config", logger_kernel);

    lista_cpus = malloc(sizeof(t_lista_cpus));
    lista_cpus->lista_cpus = list_create();
    pthread_mutex_init(&lista_cpus->mutex_lista, NULL);

    // INICIAMOS SEMAFOROS
    sem_init(&sem_cantidad_pcbs_en_new, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_ready, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_blocked, 0, 0);
    
    sem_init(&bin_proceso_eliminar, 0, 1);
    sem_init(&bin_cpu_disponible, 0, 0);

    tiempo_esperando = temporal_create();
    tiempo_esperando->elapsed_ms = 0;
    temporal_stop(tiempo_esperando);

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

void iniciar_planificacion_largo_plazo(){

    _iniciar_cuando_apreta_enter();

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    pthread_t hilo_planificador_mediano_plazo;
 	pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_mediano_plazo, NULL);
	pthread_detach(hilo_planificador_mediano_plazo);

    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;
    while(1){
        
        sem_wait(&sem_cantidad_pcbs_en_new);

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

        t_pcb* _proceso_bloqueado = pop_cola_mutex(estado_blocked);
        encolar_pcb_en_estado(estado_blocked_aux, _proceso_bloqueado);

        t_io* _io_que_usa_pcb_bloqueado = buscar_io_en_lista(lista_de_io->lista_ios, _proceso_bloqueado->pid);

        t_info_proceso_en_io* _proceso_que_usa_io = buscar_proceso_en_io(_io_que_usa_pcb_bloqueado->procesos, _proceso_bloqueado->pid);

        if(_io_que_usa_pcb_bloqueado->enabled == true) {

            pthread_mutex_lock(&lista_de_io->mutex_lista);
            _io_que_usa_pcb_bloqueado->enabled = false;
            pthread_mutex_unlock(&lista_de_io->mutex_lista);

            enviar_proceso_a_io_para_bloqueo(_proceso_bloqueado->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
        } else { 

            int64_t tiempo_maximo_espera = strtoll(configuracion_kernel->TIEMPO_SUSPENSION, NULL, 10);
            bool flag = false, proceso_suspendido = false;

            do { 
                temporal_resume(tiempo_esperando);
                
                if(_chequear_interfaz_disponible(_io_que_usa_pcb_bloqueado) == true) {

                    pthread_mutex_lock(&lista_de_io->mutex_lista);
                    _io_que_usa_pcb_bloqueado->enabled = false;
                    pthread_mutex_unlock(&lista_de_io->mutex_lista);

                    if(proceso_suspendido == true) {
                        t_pcb* _proceso_suspendido = pop_cola_mutex(estado_susp_blocked);
                        encolar_pcb_en_estado(estado_blocked_aux, _proceso_suspendido);
                        enviar_proceso_suspendido_a_io_para_bloqueo(_proceso_suspendido->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
                    } else {
                        enviar_proceso_a_io_para_bloqueo(_proceso_bloqueado->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
                    }
                    flag = true;
                }

                if(temporal_gettime(tiempo_esperando) >= tiempo_maximo_espera && _chequear_interfaz_disponible(_io_que_usa_pcb_bloqueado) == false && proceso_suspendido == false){
                    pasar_pcb_blocked_a_suspblocked(_proceso_bloqueado);
                    // avisar a memo para que aumente el tamanio
                    proceso_suspendido = true;
                }
                

            } while(flag == false);

        }
        tiempo_esperando->elapsed_ms = 0;
        temporal_stop(tiempo_esperando);
    }
}

bool _chequear_interfaz_disponible(t_io* interfaz) {
    bool interfaz_disponible = false;
    pthread_mutex_lock(&lista_de_io->mutex_lista);
    interfaz_disponible = interfaz->enabled;
    pthread_mutex_unlock(&lista_de_io->mutex_lista);
    return interfaz_disponible;
}

// CONSUMIDOR
void iniciar_planificador_corto_plazo(){
    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_ready);
        
        if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "FIFO") == 0){
            
            t_cpu_conectada* cpu_libre = malloc(sizeof(t_cpu_conectada));
            
            sem_wait(&bin_cpu_disponible); // Iniciado con la cant de CPUs
            cpu_libre = _buscar_cpu_libre();

            t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   

            pasar_pcb_ready_a_exec(pcb_a_operar);
            
            t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); // Hacerle el free
            infoProceso->pc = pcb_a_operar->pc;
            infoProceso->pid = pcb_a_operar->pid;

            pthread_mutex_lock(&lista_cpus->mutex_lista);
            cpu_libre->pid_en_cpu = pcb_a_operar->pid;
            pthread_mutex_unlock(&lista_cpus->mutex_lista);

            enviar_proc_cpu(*infoProceso, cpu_libre->socket_dispatch);
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
    t_pcb* pcb = peek_cola_mutex(estado_new);
    return pcb;
}

t_pcb* peek_pcb_en_susp_ready() 
{
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
    }
}

