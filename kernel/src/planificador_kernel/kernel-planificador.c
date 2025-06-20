#include "kernel-gestor.h"

#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/utils-complementarios/archivo-planificador/utils-planificador.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>
#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>

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

t_lista_cpus* lista_cpus = NULL;

void iniciar_planificacion_largo_plazo(){

    _iniciar_cuando_apreta_enter();

    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)iniciar_planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    pthread_t hilo_planificador_mediano_plazo;
 	pthread_create(&hilo_planificador_mediano_plazo, NULL, (void*)iniciar_planificador_mediano_plazo, NULL);
	pthread_detach(hilo_planificador_mediano_plazo);

    t_pcb* proceso_1 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 500, asignar_pid(), 50000);
    log_info(logger_kernel, "%d Se crea el proceso - Estado: NEW", proceso_1->pid);
    pasar_pcb_a_new(proceso_1);
    

    t_pcb* proceso_2 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES2.txt", 500, asignar_pid(), 50);
    log_info(logger_kernel, "%d Se crea el proceso - Estado: NEW", proceso_2->pid);
    pasar_pcb_a_new(proceso_2);
    

    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;
    while(1){
        if(!list_is_empty(estado_new->cola) || !list_is_empty(estado_susp_ready->cola)){
            
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
}

void iniciar_planificador_mediano_plazo() {

    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_blocked);

        t_pcb* _proceso_bloqueado = pop_cola_mutex(estado_blocked);
        encolar_pcb_en_estado(estado_blocked_aux, _proceso_bloqueado);

        t_io* _io_que_usa_pcb_bloqueado = buscar_io_en_lista(lista_de_io->lista_ios, _proceso_bloqueado->pid);
        t_info_proceso_en_io* _proceso_que_usa_io = buscar_proceso_en_io(_io_que_usa_pcb_bloqueado->procesos, _proceso_bloqueado->pid);
        
        if(_io_que_usa_pcb_bloqueado->enabled) {
            
            alternar_estado_io(_io_que_usa_pcb_bloqueado);

            enviar_proceso_a_io_para_bloqueo(_proceso_bloqueado->pid, _proceso_que_usa_io->tiempo, _io_que_usa_pcb_bloqueado->socket);

        } else { 

            // LA interfaz no esta disponible -> sigue en BLOCKED
            pthread_t hilo_administrar_proceso_bloqueado;
 	        pthread_create(&hilo_administrar_proceso_bloqueado, NULL, (void*)administrar_proceso_bloqueado, (void*)_proceso_bloqueado);
	        pthread_detach(hilo_administrar_proceso_bloqueado);

        }
    }
}

// CONSUMIDOR
void iniciar_planificador_corto_plazo(){
    while(1){
        // Semaforo para que se pueda loopear el while hasta que haya algun proceso en READY
        sem_wait(&sem_cantidad_pcbs_en_ready);

        if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "FIFO") == 0) {
            
            planificar_con_fifo();
            
        } else if(strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "SJF") == 0) {

            planificar_con_sjf();

        } else if (strcmp(configuracion_kernel->ALGORITMO_CORTO_PLAZO, "SRT") == 0) {

            planificar_con_srt();

        } else {

            log_error(logger_kernel, "El algoritmo de corto plazo no es soportado por el Sistema Operativo...");
            exit(1);
        }
    }
}

