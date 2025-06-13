
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
    int* offset = malloc(sizeof(int)); 
    int pid, pc;
    while (1) {
        
        t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_interrupt);

        *offset = 0;
        
        switch (paquete->codigo_operacion){
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_kernel);
                break;
            
            case SYSCALL_IO:
            
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);                        

                pc = _deserializar_pc(offset, paquete); 
                
                t_syscall_io _syscall_io_recibida = _deserializar_syscall_io(offset, paquete);
                
                log_info(logger_kernel, "## %d - Solicitó syscall: IO", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_io* interfaz_disponible = funcion_syscall_IO(_syscall_io_recibida.dispositivo);
                
                t_pcb* _proceso_a_bloquear = _sacar_pcb_de_exec(pid);
                _proceso_a_bloquear->pc = pc;

                if(interfaz_disponible != NULL) { 
                    
                    log_info(logger_kernel, "## %d - Bloqueado por IO: %s", pid, _syscall_io_recibida.dispositivo);    
                    
                    t_info_proceso_en_io* _info_proceso_bloqueado = malloc(sizeof(t_info_proceso_en_io));
                    _info_proceso_bloqueado->pid    = _proceso_a_bloquear->pid;
                    _info_proceso_bloqueado->tiempo = _syscall_io_recibida.tiempo;

                    encolar_pcb_en_interfaz(interfaz_disponible, _info_proceso_bloqueado);
    
                    pasar_de_exec_a_blocked(_proceso_a_bloquear); 

                    sem_post(&sem_cantidad_pcbs_en_blocked);
                    // SEMAFORO PARA PLANIFICADOR LARGO PLAZO PARA QUE SALGA DE ESPERA ACTIVA

                } else {

                    pasar_de_exec_a_exit(_proceso_a_bloquear);

                }

                eliminar_paquete(paquete);
                break;
            case SYSCALL_INIT_PROC:
                // de la syscall INIT_PORC recibo PID, Archivo, tamanioProceso
                recibir_paquete(socket_interrupt, paquete);

                pid = _deserializar_pid(offset, paquete);    

                log_info(logger_kernel, "## %d - Solicitó syscall: INIT_PROC", pid);

                char* archivo = deserializar_archivo_instrucciones(offset, paquete);

                int tamanio_proceso = deserializar_tamanio_proceso(offset, paquete);

                t_pcb* nuevo_proceso = iniciarPCB(archivo, tamanio_proceso, asignar_pid());

                pasar_pcb_a_new(nuevo_proceso);

                eliminar_paquete(paquete);
                break;
            case SYSCALL_DUMP_MEMORY:
                
                recibir_paquete(socket_interrupt, paquete);
                
                pid = _deserializar_pid(offset, paquete); 

                log_info(logger_kernel, "## %d - Solicitó syscall: DUMP_MEMORY", pid);

                pc = _deserializar_pc(offset, paquete); 

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_pcb* _proceso_a_dumpear = _sacar_pcb_de_exec(pid);
                _proceso_a_dumpear->pc = pc;

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

                log_info(logger_kernel, "## %d - Solicitó syscall: EXIT", pid);

                liberar_cpu_de_proceso(pid); // Libero a la cpu para que mande otro proceso

                t_pcb* _proceso_a_finalizar = _sacar_pcb_de_exec(pid);
                _proceso_a_finalizar->pc = pc;

                pasar_de_exec_a_exit(_proceso_a_finalizar);
                
                eliminar_paquete(paquete);
                break;

            case -1:
                log_error(logger_kernel, "El cliente [CPU - Interrupt] se desconectó.");
                return EXIT_FAILURE;
            default:
                log_warning(logger_kernel, "Operación desconocida en interrupt.");
                break;
            }
        }
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
		paquete->codigo_operacion = recibir_operacion(socket_dispatch);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_dispatch, logger_kernel);
			break;

		case INSTRUCCION:
			break;
		case -1:
			log_error(logger_kernel, "el cliente [CPU - Dispatch] se desconecto.");
            flag = true;
			break;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_dispatch);
	return EXIT_SUCCESS;
}