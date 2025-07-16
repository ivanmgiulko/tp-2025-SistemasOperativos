
#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-cpu/modulo-cpu.h>
#include <utils_kernel/manejar-conexiones/modulo-cpu/manejar-conexion-cpu.h>
#include <utils_kernel/manejar-conexiones/modulo-memoria/manejar-conexion-memoria.h>
#include <utils_kernel/utils-complementarios/conexion-con-cpu/utils-kernel-cpu.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>

int manejar_cliente_interrupt(void* socket_cliente_ptr){
	int socket_interrupt = *(int*)socket_cliente_ptr;
    uint8_t pid; 
    uint16_t pc;

    while (1) {
        
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(socket_interrupt);
        
        int* offset = malloc(sizeof(int)); 
        *offset = 0;
        
        switch (paquete->codigo_operacion){
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_kernel);
                free(paquete);
                free(offset);
                break;
            
            case SYSCALL_IO:
            
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);                        

                pc = _deserializar_pc(offset, paquete); // 6

                // Cantidad de rafagas = 5 | pc - 1
                
                t_datos_io _syscall_io_recibida = _deserializar_syscall_io(offset, paquete);
                
                free(offset);
                
                log_info(logger_kernel, "%d - Solicitó syscall: IO", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_pcb* _proceso_a_bloquear = _sacar_pcb_de_cola(pid, estado_exec);
                
                _proceso_a_bloquear->pc = pc;
                
                _proceso_a_bloquear->tiempo_rafaga = temporal_gettime(_proceso_a_bloquear->metricas_tiempo->tiempoEnExec) - _proceso_a_bloquear->estimacion_aux;
                
                _proceso_a_bloquear->estimacion_aux = temporal_gettime(_proceso_a_bloquear->metricas_tiempo->tiempoEnExec);
                
                if(lista_de_io_vacia()) {
                    
                    pasar_de_exec_a_exit(_proceso_a_bloquear);

                } else {
                    
                    pthread_mutex_lock(&(lista_de_io->mutex_lista));
                    t_io* interfaz_io_existente = buscar_io(lista_de_io->lista_ios, _syscall_io_recibida.dispositivo); 
                    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
                
                    if(interfaz_io_existente != NULL) { 
                    
                        log_info(logger_kernel, "%d - Bloqueado por IO: %s", pid, _syscall_io_recibida.dispositivo);    
                    
                        _proceso_a_bloquear->datos_io->dispositivo =_syscall_io_recibida.dispositivo;
                        _proceso_a_bloquear->datos_io->tiempo = _syscall_io_recibida.tiempo;

                        encolar_pcb_en_interfaz(interfaz_io_existente, &_proceso_a_bloquear->pid);

                        pasar_de_exec_a_blocked(_proceso_a_bloquear); 

                        sem_post(&sem_cantidad_pcbs_en_blocked);
                        // SEMAFORO PARA PLANIFICADOR LARGO PLAZO PARA QUE SALGA DE ESPERA ACTIVA

                        } else {

                        pasar_de_exec_a_exit(_proceso_a_bloquear);

                    }
                
                eliminar_paquete(paquete);
                
                }
                
                break;

            case SYSCALL_INIT_PROC:
                // de la syscall INIT_PORC recibo PID, Archivo, tamanioProceso
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);    

                log_info(logger_kernel, "%d - Solicitó syscall: INIT_PROC", pid);

                char* archivo = deserializar_archivo_instrucciones(offset, paquete);

                int tamanio_proceso = deserializar_tamanio_proceso(offset, paquete);

                free(offset);

                t_pcb* nuevo_proceso = iniciarPCB(archivo, tamanio_proceso, asignar_pid(), atoi(configuracion_kernel->ESTIMACION_INICIAL));

                log_info(logger_kernel, "%d Se crea el proceso - Estado: NEW", nuevo_proceso->pid);

                pthread_mutex_lock(&estado_new->mutex);
                pasar_pcb_a_new(nuevo_proceso);
                pthread_mutex_unlock(&estado_new->mutex);

                eliminar_paquete(paquete);
                break;

            case SYSCALL_DUMP_MEMORY:
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete); 

                log_info(logger_kernel, "%d - Solicitó syscall: DUMP_MEMORY", pid);

                pc = _deserializar_pc(offset, paquete); 
                
                free(offset);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_pcb* _proceso_a_dumpear = _sacar_pcb_de_cola(pid, estado_exec);
                _proceso_a_dumpear->pc = pc;

                _proceso_a_dumpear->tiempo_rafaga = temporal_gettime(_proceso_a_dumpear->metricas_tiempo->tiempoEnExec) - _proceso_a_dumpear->estimacion_aux;

                _proceso_a_dumpear->estimacion_aux = temporal_gettime(_proceso_a_dumpear->metricas_tiempo->tiempoEnExec);

                pasar_de_exec_a_blocked(_proceso_a_dumpear);

                char* ip_memoria = configuracion_kernel->IP_MEMORIA;
                char* puerto_memoria = configuracion_kernel->PUERTO_MEMORIA;
                int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

                enviar_proceso_a_dumpear_en_memoria(fd_conexion_memoria, *_proceso_a_dumpear);

                manejar_conexion_kernel_memoria(fd_conexion_memoria);
                
                eliminar_paquete(paquete);
                break;

            case SYSCALL_EXIT:
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete); 
                
                pc = _deserializar_pc(offset, paquete); 

                free(offset);

                log_info(logger_kernel, "%d - Solicitó syscall: EXIT", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_pcb* _proceso_a_finalizar = _sacar_pcb_de_cola(pid, estado_exec);
                _proceso_a_finalizar->pc = pc;

                pasar_de_exec_a_exit(_proceso_a_finalizar);
                
                eliminar_paquete(paquete);
                break;

            case PROCESO_DESALOJADO:

                recibir_paquete(socket_interrupt, paquete);
                
                memcpy(&pid, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

                memcpy(&pc, paquete->buffer->stream + *offset, sizeof(int)); *offset += sizeof(int);

                free(offset);

                log_info(logger_kernel, "%d - Desalojado por algoritmo SJF/SRT", pid);

                t_pcb* _proceso_desalojado = _sacar_pcb_de_cola(pid, estado_exec);
                _proceso_desalojado->pc = pc;

                liberar_cpu_de_proceso(pid);

                pasar_pcb_exec_a_ready(_proceso_desalojado);

                eliminar_paquete(paquete);
                
                break;

            case -1:
                log_error(logger_kernel, "El cliente [CPU - Interrupt] se desconectó.");
                free(offset);
                eliminar_paquete(paquete);
                return EXIT_FAILURE;
            default:
                free(offset);
                log_warning(logger_kernel, "Operación desconocida en interrupt.");
                eliminar_paquete(paquete);
                break;
            }
        }
    
    pthread_exit(NULL);
    close(socket_interrupt);
    log_info(logger_kernel, "Conexión cerrada en interrupt: socket %d", socket_interrupt);
	return EXIT_SUCCESS;
}

void* manejar_cliente_dispatch(void* socket_cliente_ptr) {
	int socket_dispatch = *(int*)socket_cliente_ptr;
    bool flag = false;
	while (!flag) {

        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_cod_operacion(socket_dispatch);

		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_dispatch, logger_kernel);
            free(paquete);
			break;
		case -1:
			log_error(logger_kernel, "el cliente [CPU - Dispatch] se desconecto.");
            eliminar_paquete(paquete);
            flag = true;
			break;
		default:
            eliminar_paquete(paquete);
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
    
    pthread_exit(NULL);
	close(socket_dispatch);
	return EXIT_SUCCESS;
}