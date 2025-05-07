#include "cpu-k-dispatch.h"

int manejar_conexion_kernel_dispatch(){

	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(fd_conexion_kernel_dispatch);

		switch (paquete->codigo_operacion) {

			case MENSAJE:
				recibir_mensaje(fd_conexion_kernel_dispatch, logger_cpu);
				break;
				
			case INFO_PROC_EXEC:
				
				recv(fd_conexion_kernel_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				recv(fd_conexion_kernel_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
				t_peticion_instruccion* infoPCB = deserializar_info_pcb(paquete->buffer);
				sem_post(&sem_cpu);
				log_trace(logger_cpu, "PID: %d | PC: %d", infoPCB->pid, infoPCB->pc);
				pedir_instruccion_a_memoria(infoPCB);
				free(infoPCB);
				// 

				// El proceso debe realizar una IO ahora:
				t_param_io* pruebaIO = malloc(sizeof(t_param_io));
				pruebaIO->dispositivo = string_duplicate("MOUSE");
				pruebaIO->dispositivo_length = string_length(pruebaIO->dispositivo);
				pruebaIO->tiempo = 25000;
				enviar_io_kernel(*pruebaIO, fd_conexion_kernel_dispatch);
				
				break;

			case INSTRUCCION:
				break;
			
			case -1:
				log_error(logger_cpu, "el cliente se desconecto.");
				return EXIT_FAILURE;
			default:
				log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
	close(fd_conexion_kernel_dispatch);
	return EXIT_SUCCESS;
}

t_peticion_instruccion* deserializar_info_pcb(t_buffer* buffer) {
	t_peticion_instruccion* infoPCB = malloc(sizeof(t_peticion_instruccion));
    void* stream = buffer->stream;

    memcpy(&(infoPCB->pid), stream, sizeof(int)); stream += sizeof(int);
    memcpy(&(infoPCB->pc), stream, sizeof(int)); stream += sizeof(int);
    
    return infoPCB;

}

void enviar_io_kernel(t_param_io pruebaIO, int socket_cliente){
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(int64_t) + sizeof(uint32_t) + (pruebaIO.dispositivo_length);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pruebaIO.tiempo, sizeof(int64_t)); offset += sizeof(int64_t);
    memcpy(buffer->stream + offset, &pruebaIO.dispositivo_length, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, pruebaIO.dispositivo, pruebaIO.dispositivo_length);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SYSCALL_IO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

}