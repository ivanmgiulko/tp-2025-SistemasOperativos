#include "conexion-kernel-cpu.h"

void manejar_conexion_kernel_interrupt() {
    while (1) {
        int socket_interrupt = esperar_cliente(fd_server_kernel_interrupt, logger_kernel);
        if (socket_interrupt == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en interrupt");
            continue;
        }
        log_info(logger_kernel, "Nueva conexión en interrupt: socket %d", socket_interrupt);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_interrupt;
        pthread_create(&hilo_cliente_interrupt, NULL, (void*)manejar_cliente_interrupt, (void*)&socket_interrupt);
        pthread_detach(hilo_cliente_interrupt);
    }
}

void manejar_conexion_kernel_dispatch() {
    while (1) {
        int socket_dispatch = esperar_cliente(fd_server_kernel_dispatch, logger_kernel);
        if (socket_dispatch == -1) {
            log_error(logger_kernel, "Error al aceptar cliente en dispatch");
            continue;
        }
        log_info(logger_kernel, "Nueva conexión en dispatch: socket %d", socket_dispatch);

        // Crear un hilo para manejar la conexión del cliente
        pthread_t hilo_cliente_dispatch;
        pthread_create(&hilo_cliente_dispatch, NULL, (void*)manejar_cliente_dispatch, (void*)&socket_dispatch);
        pthread_detach(hilo_cliente_dispatch);
    }
}


int manejar_cliente_interrupt(void* socket_cliente_ptr){
	int socket_interrupt = *(int*)socket_cliente_ptr;
    while (1) {
        int cod_op = recibir_operacion(socket_interrupt);
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_interrupt, logger_kernel);
            break;
        case INSTRUCCION:
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
int manejar_cliente_dispatch(void* socket_cliente_ptr){
	int socket_dispatch = *(int*)socket_cliente_ptr;
	while (1) {
		int cod_op = recibir_operacion(socket_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_dispatch, logger_kernel);
			break;
		case INSTRUCCION:
			break;
		case -1:
			log_error(logger_kernel, "el cliente [CPU - Dispatch] se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_dispatch);
	return EXIT_SUCCESS;
}
