#include "cpu-k-dispatch.h"

int manejar_conexion_kernel_dispatch(){

	while (1) {
		int cod_op = recibir_operacion(fd_conexion_kernel_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_conexion_kernel_dispatch, logger_cpu);
			break;
        case INTERFAZ:
			char* nombreInterfaz = recibir_nommbreInterfaz(fd_conexion_kernel_dispatch);
			log_info(logger_cpu, "Recibi la interfaz desde IO: %s", nombreInterfaz);
		//	list_add(lista_interfaces, nombreInterfaz);
			
			break;
		case INSTRUCCION:
			break;
			/*
		case PAQUETE:
			lista = recibir_paquete(fd_conexion_kernel_dispatch);
			log_info(logger_cpu, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
            */
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


