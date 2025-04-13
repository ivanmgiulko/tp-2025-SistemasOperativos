#include"server.h"

int iniciar_servidor(char* puerto, t_log* logger_servidor)
{
	int socket_servidor;

	struct addrinfo hints, *server_info, *p;

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

int esperar_cliente(int socket_servidor, t_log* logger_servidor)
{ 
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger_servidor, "Se conecto un cliente!");

	return socket_cliente;
}

void manejar_hilos(int server_fd, t_log* logger_servidor){

    while(1){
        int socket_cliente = esperar_cliente(server_fd, logger_servidor);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion, (void*)socket_cliente, (void*)logger_servidor);
        pthread_detach(hilo_cliente);
    }

}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger_servidor)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger_servidor, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

int manejar_conexion(int socket_server, int socket_cliente, t_log* logger_servidor){
	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente, logger_servidor);
			break;
            /*
		case PAQUETE:
			lista = recibir_paquete(socket_cliente);
			log_info(logger_servidor, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
            */
		case -1:
			// log_error(logger_servidor, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger_servidor, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	
	close(socket_server);
	close(socket_cliente);
	return EXIT_SUCCESS;
}

/*
void iterator(char* value) {
	log_info(logger_servidor,"%s", value);
}
*/