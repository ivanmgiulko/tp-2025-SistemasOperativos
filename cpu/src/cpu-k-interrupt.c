#include "cpu-k-interrupt.h"
// logger

int manejar_conexion_kernel_interrupt(){
    
	while (1) {
		int cod_op = recibir_operacion(fd_conexion_kernel_interrupt);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_conexion_kernel_interrupt, logger_cpu);
			break;
        
		case PROCESO_DESALOJAR:

			log_warning(logger_cpu, "NIGGA HAY QUE DESALOJAR EL PROCESO");

			enviar_proceso_desalojado(fd_conexion_kernel_interrupt, 0, 4);

			break;
			
		case -1:
			log_error(logger_cpu, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(fd_conexion_kernel_interrupt);
	return EXIT_SUCCESS;
}

void enviar_proceso_desalojado(int socket_servidor, int pid, int pc) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(int) + sizeof(int);
	buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(buffer->stream + offset, &pid, sizeof(int)); offset += sizeof(int);
    memcpy(buffer->stream + offset, &pc, sizeof(int)); offset += sizeof(int);
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO_DESALOJADO;
    paquete->buffer = buffer;
    void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(uint32_t));
    offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));   offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    send(socket_servidor, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    free(a_enviar);
    eliminar_paquete(paquete);

}