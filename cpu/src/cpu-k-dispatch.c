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
				pthread_mutex_lock(&mutex_cpu);
				pcb_actual->pid = infoPCB->pid;
				pcb_actual->pc = infoPCB->pc;
				pthread_mutex_unlock(&mutex_cpu);
				sem_post(&sem_cpu);
				log_trace(logger_cpu, "#cpu-k-dispatch.c PID: %d | PC: %d", infoPCB->pid, infoPCB->pc);
				
				pedir_instruccion_a_memoria(infoPCB);
				
				free(infoPCB);
				
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

void enviar_syscall_init_proc_kernel(t_param_init_proc prueba_init_proc, int socket_cliente){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	uint32_t archivoLength = strlen(prueba_init_proc.archivo);
    buffer->size = sizeof(int) + sizeof(uint32_t) + (archivoLength);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &prueba_init_proc.tamanio, sizeof(int)); offset += sizeof(int);
    memcpy(buffer->stream + offset, &archivoLength, sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(buffer->stream + offset, prueba_init_proc.archivo, archivoLength);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SYSCALL_INIT_PROC;
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

void enviar_syscall_exit(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = SYSCALL_EXIT;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}