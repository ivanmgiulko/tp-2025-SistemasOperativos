#include "cpu-memoria.h"
int manejar_conexion_memoria(){

    while (1) {
        
        t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);

        paquete->codigo_operacion = recibir_cod_operacion(fd_conexion_memoria);
        
        recibir_buffer_en_paquete(fd_conexion_memoria, paquete);

        if(paquete->buffer->stream == NULL){
        //    log_error(logger_cpu, "ERROR al recibir paquete desde memoria (contenido NULL)");
            eliminar_paquete(paquete);
            continue;
        }
        log_warning(logger_cpu, "RECIBI CODIGO: %d", paquete->codigo_operacion );
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
                int desplazamiento = 0;
                respuesta_instruccion = leer_string_desde_buffer(paquete->buffer, &desplazamiento);
                sem_post(&sem_instruccion);
                break;

            case WRITE_MEMORIA: 
                char* mensaje = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - ESCRIBIR en <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, mensaje);
                pcb_actual->pc++;
                respuesta_memo = strdup(mensaje);
                log_trace(logger_cpu, "write normal");
                sem_post(&sem_cpu);
                break;
            

            case READ_MEMORIA: 
                ultima_lectura = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - LEER <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, ultima_lectura);
                log_trace(logger_cpu, "read normal");
                pcb_actual->pc++;
                sem_post(&sem_cpu);
                break;

            case WRITE_MEMORIA_CACHE: 
                char* mensaje_cache = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - ESCRIBIR en <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, mensaje);
                respuesta_memo = strdup(mensaje_cache);
                log_trace(logger_cpu, "WRITE CACHE");
                sem_post(&sem_respuesta_memo);
                break;
            

            case READ_MEMORIA_CACHE: 
                ultima_lectura = deserializar_read_o_write_de_memoria(paquete);
                log_info(logger_cpu, "PID: <%d> - LEER <%d> -> %s", pcb_actual->pid, mmu->ultima_direccion_fisica_calculada, ultima_lectura);
                log_trace(logger_cpu, "READ CACHE");

                sem_post(&sem_read);
                break;
            

            case OBTENER_MARCO_CORRESPONDIENTE:
                log_warning(logger_cpu, "EL NO HIZO NADA");
                if (paquete && paquete->buffer && paquete->buffer->stream) {
                    memcpy(&marco_global, paquete->buffer->stream + sizeof(uint32_t), sizeof(int32_t));
                    log_warning(logger_cpu, "recibir_marco_solicitado ==== %d", marco_global);
                }
                else{
                    log_error(logger_cpu, "Error al recibir el marco solicitado");
                }
                log_warning(logger_cpu, "EL NO HIZO NADA   2");
                sem_post(&sem_rta_marco);
                break;
            case FIN_PID:

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
