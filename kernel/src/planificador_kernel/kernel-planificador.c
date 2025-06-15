#include "kernel-planificador.h"
#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/utils-complementarios/archivo-planificador/utils-planificador.h>

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

t_lista_cpus* lista_cpus = NULL;

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
        // Si devuelve null?
        if(_io_que_usa_pcb_bloqueado->enabled) {
            
            alternar_estado_io(_io_que_usa_pcb_bloqueado);

            enviar_proceso_a_io_para_bloqueo(_proceso_bloqueado->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);
        } else { 

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

                }else if(temporal_gettime(tiempo_esperando) >= tiempo_maximo_espera && !proceso_suspendido){
                    pasar_pcb_blocked_a_suspblocked(_proceso_bloqueado);
                    // avisar a memo para que aumente el tamanio
                    proceso_suspendido = !proceso_suspendido;
                }
                
            } while(!flag);

        }
        tiempo_esperando->elapsed_ms = 0;
        temporal_stop(tiempo_esperando);
    }
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