#include <utils/hello.h>
#include "kernel-header.h"


int main(int argc, char* argv[]) {
	t_config* config_kernel = iniciar_config();
		char* puerto_kernel_interrupt;

	logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE);
    
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
config_kernel = config_create("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/kernel.config");

	puerto_kernel_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
//	int server_fd = iniciar_servidor();
	int server_cpu_interrupt_fd = iniciar_servidor(puerto_kernel_interrupt);
	log_info(logger_kernel, "Servidor listo para recibir al cliente");
	int cliente_cpu_interrupt_fd = esperar_cliente(server_cpu_interrupt_fd);
//	int cliente_fd = esperar_cliente(server_fd);

//	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_cpu_interrupt_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_cpu_interrupt_fd);
			break;
			/*
		case PAQUETE:
			lista = recibir_paquete(cliente_cpu_interrupt_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
			*/
		case -1:
			log_error(logger_kernel, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	close(cliente_cpu_interrupt_fd);
	close(server_cpu_interrupt_fd);
	return EXIT_SUCCESS;
}
