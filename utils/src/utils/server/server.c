#include"server.h"
#include <utils/serializacion/serializacion.h>
t_list* lista_interfaces; // Lista de interfaces usada por el Kernel
t_log* logger_servidor; // logger de Kernel o de Memoria

int iniciar_servidor(char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(server_info->ai_family,
                        server_info->ai_socktype,
                        server_info->ai_protocol);

	// Asociamos el socket a un puerto
	setsockopt(socket_servidor , SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(server_info);
	log_trace(logger_servidor, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{ 
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger_servidor, "Se conecto un cliente!");

	return socket_cliente;
}

void manejar_hilos(int server_fd){

    while(1){
        int socket_cliente = esperar_cliente(server_fd);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion, (void*)socket_cliente);
        pthread_detach(hilo_cliente);
    }

}

int manejar_conexion(int socket_cliente){
	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente);
			break;
        case INTERFAZ:
			char* nombreInterfaz = recibir_nommbreInterfaz(socket_cliente);
			log_info(logger_servidor, "Recibi la interfaz desde IO: %s", nombreInterfaz);
			list_add(lista_interfaces, nombreInterfaz);
			
			break;
		case INSTRUCCION:
			break;
			/*
		case PAQUETE:
			lista = recibir_paquete(socket_cliente);
			log_info(logger_servidor, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
            */
		case -1:
			log_error(logger_servidor, "el cliente se desconecto.");
			return EXIT_FAILURE;
		default:
			log_warning(logger_servidor, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

	close(socket_cliente);
	return EXIT_SUCCESS;
}


void iterator(char* value) {
	log_info(logger_servidor,"%s", value);
}
