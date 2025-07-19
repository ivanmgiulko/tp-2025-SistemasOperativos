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
sem_t bin_eliminar_procesos_en_interfaces;
sem_t bin_proceso_eliminar;
sem_t bin_cpu_disponible;

sem_t sem_hay_espacio_en_memoria;

t_lista_cpus* lista_cpus = NULL;

void iniciar_planificacion_largo_plazo(){

    _iniciar_cuando_apreta_enter();

    t_pcb* proceso_1 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt", 500, asignar_pid(), 50000);
    log_info(logger_kernel, "%d Se crea el proceso - Estado: NEW", proceso_1->pid);
    pasar_pcb_a_new(proceso_1);
    
    t_pcb* proceso_2 = iniciarPCB("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES2.txt", 500, asignar_pid(), 50);
    log_info(logger_kernel, "%d Se crea el proceso - Estado: NEW", proceso_2->pid);
    pasar_pcb_a_new(proceso_2);
    
    char* algortimo_ingreso_ready = configuracion_kernel->ALGORITMO_INGRESO_A_READY;
    while(1){
    
        sem_wait(&sem_cantidad_pcbs_en_new);

        bool _cola_susp_ready_esta_vacia = _verificar_cola_susp_ready_esta_vacia();
    
        bool _cola_new_estaba_vacia = _verificar_cola_new_estaba_vacia();  // :v

        _cola_susp_ready_esta_vacia ?  _enviar_desde_new_a_ready(_cola_new_estaba_vacia, algortimo_ingreso_ready) :  _enviar_desde_susp_ready_a_ready(_cola_new_estaba_vacia, algortimo_ingreso_ready);

    }

    pthread_exit(NULL);
}

void iniciar_planificador_mediano_plazo() {
    while(1){
        sem_wait(&sem_cantidad_pcbs_en_blocked);

        pthread_mutex_lock(&(estado_blocked->mutex));
        uint8_t cantidad_procesos_bloqueados = list_size(estado_blocked->cola);
        pthread_mutex_unlock(&(estado_blocked->mutex));

        t_pcb* _proceso_bloqueado = peek_cola_mutex(estado_blocked, cantidad_procesos_bloqueados - 1);
        
        pthread_t hilo_temporizador;
        pthread_create(&hilo_temporizador, NULL, (void*)iniciar_temporizador_suspblocked, (void*)_proceso_bloqueado);
        pthread_detach(hilo_temporizador);
    } 
    
    pthread_exit(NULL);
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

    pthread_exit(NULL);
}

