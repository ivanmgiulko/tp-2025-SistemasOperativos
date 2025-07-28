#include "cpu-memoria.h"
int manejar_conexion_memoria(){
    log_trace(logger_cpu, "[DEBUG] Hilo de recepción de memoria iniciado");

    while (1) {
        pthread_mutex_lock(&mutex_conexion_memoria);
        while (!receptor_habilitado) {
            log_debug(logger_cpu, "[RECEPTOR] En pausa, esperando señal...");
            pthread_cond_wait(&condicion_reactivacion_recepcion_memoria, &mutex_conexion_memoria);
        }
        pthread_mutex_unlock(&mutex_conexion_memoria);

        // Solo si está habilitado recibe
        t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);

        // 🛑 Esto solo debería ejecutarse si receptor_habilitado sigue siendo true
        paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
 //       log_warning(logger_cpu, "[RECV] Recibido código de operación: %s (nro: %d)", 
  //                  convertir_cod_op_a_string(paquete->codigo_operacion), 
    //                paquete->codigo_operacion);
        recibir_buffer_en_paquete(fd_conexion_memoria, paquete);

        if(paquete->buffer->stream == NULL){
        //    log_error(logger_cpu, "ERROR al recibir paquete desde memoria (contenido NULL)");
            eliminar_paquete(paquete);
            continue;
        }

        switch (paquete->codigo_operacion) {
            case MENSAJE:
                loggear_mensaje_desde_buffer(paquete->buffer, logger_cpu);
                break;

            case DATOS_DE_MEMORIA:
          //      log_debug(logger_cpu, "Recibí los datos de memoria");
                recibir_datos_de_memoria(paquete, mmu);
                memoria_cache = inicializar_cache(algoritmo_reemplazo_cache, entradas_cache, mmu->tamanio_pagina, retardo_cache);
                break;

            case INSTRUCCION:
                manejar_respuesta_de_instruccion(paquete);
                break;

            case WRITE_MEMORIA: {
                char* mensaje = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - ESCRIBIR en <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, mensaje);
                pcb_actual->pc++;
                sem_wait(&sem_memoria);
                sem_post(&sem_cpu);
                break;
            }

            case READ_MEMORIA: {
                ultima_lectura = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - LEER <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, ultima_lectura);
                pcb_actual->pc++;
                sem_wait(&sem_memoria);
                sem_post(&sem_cpu);
                break;
            }

            case FIN_PID:
                sem_post(&sem_cpu_kernel);
                break;

            case -1:
                log_error(logger_cpu, "El cliente se desconectó");
                eliminar_paquete(paquete);
                return EXIT_FAILURE;

            default:
                log_warning(logger_cpu, "Operacion desconocida #%d#", paquete->codigo_operacion);
                break;
        }

        eliminar_paquete(paquete);
    }

    close(fd_conexion_memoria);
    return EXIT_SUCCESS;
}
