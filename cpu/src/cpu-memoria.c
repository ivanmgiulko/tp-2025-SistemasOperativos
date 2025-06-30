#include "cpu-memoria.h"
int manejar_conexion_memoria(){
    log_trace(logger_cpu, "[DEBUG] Hilo de recepción de memoria iniciado");
    while (1) {
        t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);
        //log_debug(logger_cpu, "Esperando código de operación de Memoria...");
        paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
        log_warning(logger_cpu, "[RECV] Recibido código de operación: %s (nro: %d)", convertir_cod_op_a_string(paquete->codigo_operacion), paquete->codigo_operacion);
        recibir_buffer_en_paquete(fd_conexion_memoria, paquete);
        log_debug(logger_cpu, "Tamaño del buffer recibido: %d", paquete->buffer->size);
        //Verifico que el contenido no sea nulo
        if(paquete->buffer->stream == NULL){
            log_error(logger_cpu, "ERROR al recibir paquete desde memoria (contenido NULL)");
            continue;
        }
        switch (paquete->codigo_operacion) {

            case MENSAJE:
            
                //recibir_mensaje(fd_conexion_memoria, logger_cpu);
                loggear_mensaje_desde_buffer(paquete->buffer, logger_cpu);
                break;

            case DATOS_DE_MEMORIA:
                log_info(logger_cpu, "Recibí los datos de memoria");
                recibir_datos_de_memoria(paquete, mmu);
                break;

            case INSTRUCCION: {
				log_info(logger_cpu, "[INSTRUCCION_THREAD] Recibí la instrucción solicitada a MEMORIA");
    			manejar_respuesta_de_instruccion(paquete);
                break;
            }

            case WRITE_MEMORIA: {

				char* mensaje_confirmacion_write = deserializar_read_o_write_de_memoria(paquete);
				log_debug(logger_cpu, "[WRITE_THREAD] Recibido contenido: tam: %d ,contenido: %s", paquete->buffer->size, mensaje_confirmacion_write);
				log_info(logger_cpu, "PID: <%d> - Accion: <ESCRIBIR> - Direccion Fisica: <%d> - Valor : <%s>", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, ultima_escritura);

				pcb_actual->pc++;
				sem_wait(&sem_memoria);
				sem_post(&sem_cpu);
                break;
            }

            case READ_MEMORIA: {
				ultima_lectura = deserializar_read_o_write_de_memoria(paquete);
				log_debug(logger_cpu, "[READ_THREAD] Recibido contenido: %.*s", paquete->buffer->size, ultima_lectura);
				log_info(logger_cpu, "PID: <%d> - Accion: <LEER> - Direccion Fisica: <%d> - Valor : <%s>", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, ultima_lectura);

				pcb_actual->pc++;
				sem_wait(&sem_memoria);
				sem_post(&sem_cpu);
                break;
            }

            case FIN_PID:
                log_info(logger_cpu, "Recibi el fin de PID");
                sem_post(&sem_cpu_kernel);
                break;
            case -1:
                log_error(logger_cpu, "el cliente se desconecto.");
                eliminar_paquete(paquete);
                return EXIT_FAILURE;
            default:
                log_warning(logger_cpu, "Operacion desconocida #%d#. No quieras meter la pata", paquete->codigo_operacion);
                break;
            }
            //Libero punteros ocupados por paquete
            eliminar_paquete(paquete);
        }

	close(fd_conexion_memoria);
	return EXIT_SUCCESS;
}