#include "server.h"
#include <utils/serializacion/serializacion.h>
#include <errno.h>
//  HACER MANEJAR CONEXION INDEPENDIENTE PARA CADA MODULO Y SUS CONEXIONES

int iniciar_servidor(char* puerto, t_log* logger)
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
	// log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{ 
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	if (socket_cliente == -1) {
        log_error(logger, "Error al aceptar cliente: %s", strerror(errno));
        return -1;
    }

	return socket_cliente;
}

void iterator(char* value, t_log* logger) {
	log_info(logger,"%s", value);
}
