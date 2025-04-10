#include"memoria-header.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

t_log* logger_memoria;

int iniciar_servidor(char* puerto)
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
	log_trace(logger_memoria, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{ 
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger_memoria, "Se conecto un cliente!");

	return socket_cliente;
}

// CREACION DE HILO PARA LOS CLIENTES
void manejar_hilos(int server_fd){

    while(1){
        int socket_cliente = esperar_cliente(server_fd);
        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, (void*)manejar_conexion, (void*)socket_cliente);
        pthread_detach(hilo_cliente);
    }

}

// HANDLER DE CONEXIONES (POR AHORA COPY PASTE DEL SERVER DE KERNEL)
int manejar_conexion(int socket_cliente){
    while (1) {
        int cod_op = recibir_operacion(socket_cliente);
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_cliente);
            break;
        case -1:
            log_error(logger_memoria, "el cliente se desconecto. Terminando servidor");
            return EXIT_FAILURE;
        default:
            log_warning(logger_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
            }
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

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger_memoria, "Me llego el mensaje %s:", buffer);
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