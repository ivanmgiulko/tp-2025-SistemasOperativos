#include <utils/hello.h>
#include "kernel-header.h"


int main(int argc, char* argv[]) {
	logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE);
    
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	int server_fd = iniciar_servidor();
	log_info(logger_kernel, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
			/*
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
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
	return EXIT_SUCCESS;
}
