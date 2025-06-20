#include "utils-planificador.h"

#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-memoria/modulo-memoria.h>

void _enviar_desde_new_a_ready(bool _cola_new_estaba_vacia, char* algortimo_ingreso_ready) 
{ 
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

void _enviar_desde_susp_ready_a_ready(bool _cola_new_estaba_vacia, char* algortimo_ingreso_ready) 
{
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
}

void crear_proceso_cero(char* path, int tamanio)
{
    inicializar_pid();

    uint64_t estimacion_inical = atoi(configuracion_kernel->ESTIMACION_INICIAL);

  	t_pcb* proceso_cero = iniciarPCB(path ,tamanio, asignar_pid(), estimacion_inical);

    pasar_pcb_a_new(proceso_cero);
	log_info(logger_kernel,"## %d Se crea el proceso - Estado: NEW", proceso_cero->pid);
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

    lista_cpus = malloc(sizeof(t_lista_cpus));
    lista_cpus->lista_cpus = list_create();
    pthread_mutex_init(&lista_cpus->mutex_lista, NULL);

    // INICIAMOS SEMAFOROS
    sem_init(&sem_cantidad_pcbs_en_new, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_ready, 0, 0);
    sem_init(&sem_cantidad_pcbs_en_blocked, 0, 0);
    
    sem_init(&bin_proceso_eliminar, 0, 1);
    sem_init(&bin_cpu_disponible, 0, 0);
    sem_init(&bin_replanificar_srt, 0, 0);

    
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

bool _menor_estimacion(void* a, void* b) {
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->estimacion_actual <= proceso_b->estimacion_actual;
}

void* _mayor_estimacion(void* a, void* b) 
{
    t_pcb* proceso_a = (t_pcb*) a;
    t_pcb* proceso_b = (t_pcb*) b;
    return proceso_a->estimacion_actual > proceso_b->estimacion_actual ? proceso_a : proceso_b;
}

void planificar_con_fifo() 
{
    t_cpu_conectada* cpu_libre = malloc(sizeof(t_cpu_conectada));

    sem_wait(&bin_cpu_disponible); // Iniciado con la cant de CPUs
    cpu_libre = _buscar_cpu_libre();

    t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   

    enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);

}

void planificar_con_sjf()
{
    t_cpu_conectada* cpu_libre = malloc(sizeof(t_cpu_conectada));

    sem_wait(&bin_cpu_disponible); // Iniciado con la cant de CPUs
    cpu_libre = _buscar_cpu_libre();

    list_sort(estado_ready->cola, _menor_estimacion);
    t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   

    enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);
}

void planificar_con_srt()
{
    t_cpu_conectada* cpu_libre = malloc(sizeof(t_cpu_conectada));

    cpu_libre = _buscar_cpu_libre();

    list_sort(estado_ready->cola, _menor_estimacion);
    t_pcb* pcb_a_operar = pop_cola_mutex(estado_ready);   

    if(cpu_libre == NULL) {
        // buscamos proceso con mayor estimacion en la cola de exec
        t_pcb* _proceso_con_mayor_estimacion = list_get_maximum(estado_exec->cola, _mayor_estimacion);

        if(pcb_a_operar->estimacion_actual < _proceso_con_mayor_estimacion->estimacion_actual) {

            t_cpu_conectada* cpu_de_proceso_a_desalojar = buscar_cpu_que_usa_proceso(lista_cpus->lista_cpus, _proceso_con_mayor_estimacion->pid);
        
            enviar_pid_a_desalojar(cpu_de_proceso_a_desalojar->socket_interrupt);

            log_warning(logger_kernel, "EL PROCESO [%d] QUIERE ENTRAR Y TIENE MENOS ESTIMACION", pcb_a_operar->pid);

            // Ya se desalojo el CPU con el PCB con mayor estimacion para este entonces

            sem_wait(&bin_cpu_disponible); // Espera se desaloje el CPU que usaba el otro proceso que tenia mas estimacion

            log_warning(logger_kernel, "EL PROCESO [%d]", pcb_a_operar->pid);

            enviar_a_ejecutar_proceso(cpu_de_proceso_a_desalojar, pcb_a_operar);

        } else {
            // El proceso sigue en Ready, espera a que el otro finalice para ser enviado?
            // No, es devuelto a la cola de Ready y se replanifica de nuevo...
            log_warning(logger_kernel, "EL PROCESO [%d] QUIERE ENTRAR PERO TIENE MAYOR ESTIMACION", pcb_a_operar->pid);
            encolar_pcb_en_estado(estado_ready, pcb_a_operar);
            sem_post(&sem_cantidad_pcbs_en_ready);

        }

    } else {

        log_warning(logger_kernel, "EL PROCESO [%d] QUIERE ENTRAR Y LA CPU ESTA LIBREEEE", pcb_a_operar->pid);

        sem_wait(&bin_cpu_disponible);

        enviar_a_ejecutar_proceso(cpu_libre, pcb_a_operar);

    }
}

void administrar_proceso_bloqueado(void* pcb) 
{
    t_temporal* tiempo_esperando = temporal_create();
    temporal_stop(tiempo_esperando);
    tiempo_esperando->elapsed_ms = 0;
    
    t_pcb* _proceso_bloqueado = (t_pcb*)pcb;

    t_io* _io_que_usa_pcb_bloqueado = buscar_io_en_lista(lista_de_io->lista_ios, _proceso_bloqueado->pid);
    t_info_proceso_en_io* _proceso_que_usa_io = buscar_proceso_en_io(_io_que_usa_pcb_bloqueado->procesos, _proceso_bloqueado->pid);

    int64_t tiempo_maximo_espera = strtoll(configuracion_kernel->TIEMPO_SUSPENSION, NULL, 10);
    bool flag = false, proceso_suspendido = false;

    temporal_resume(tiempo_esperando);

    do { 
        if(_chequear_interfaz_disponible(_io_que_usa_pcb_bloqueado)) {

            alternar_estado_io(_io_que_usa_pcb_bloqueado);

            if(proceso_suspendido) {

                t_pcb* _proceso_suspendido = pop_cola_mutex(estado_susp_blocked);
                encolar_pcb_en_estado(estado_blocked_aux, _proceso_suspendido);

                enviar_proceso_suspendido_a_io_para_bloqueo(_proceso_suspendido->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
                
            } else {

                enviar_proceso_a_io_para_bloqueo(_proceso_bloqueado->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
            }
            flag = !flag;

        } else if(temporal_gettime(tiempo_esperando) >= tiempo_maximo_espera && !proceso_suspendido){
            pasar_pcb_blocked_a_suspblocked(_proceso_bloqueado);

            // char* ip_memoria = configuracion_kernel->IP_MEMORIA;
            // char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
            // int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            // enviar_a_liberar_memoria(fd_conexion_memoria, *_proceso_bloqueado);

            // manejar_conexion_kernel_memoria(fd_conexion_memoria);

            // avisar a memo para que aumente el tamanio
            proceso_suspendido = !proceso_suspendido;
        }
                
    } while(!flag);

    pthread_exit(NULL);

}

void enviar_a_ejecutar_proceso(t_cpu_conectada* cpu, t_pcb* pcb) {
    
    pasar_pcb_ready_a_exec(pcb);
            
    t_peticion_instruccion* infoProceso = malloc(sizeof(t_peticion_instruccion)); 
    infoProceso->pc = pcb->pc;
    infoProceso->pid = pcb->pid;

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    cpu->pid_en_cpu = pcb->pid;
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    enviar_proc_cpu(*infoProceso, cpu->socket_dispatch);

}