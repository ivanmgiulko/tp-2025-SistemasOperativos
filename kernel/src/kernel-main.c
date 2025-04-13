#include <utils/client.h>
#include <utils/server.h>
#include "kernel-header.h"

int main(int argc, char* argv[]) {
	
	
	t_config* config_kernel = iniciar_config("./kernel.config");
	logger_servidor = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); // Crea el logger del Kernel

	
	/*
	// INICIO SERVER KERNEL PARA RECIBIR A CPU
	char* puerto_kernel_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
	int server_cpu_interrupt_fd = iniciar_servidor(puerto_kernel_interrupt, logger_servidor);
	int server_cpu_dispatch_fd = iniciar_servidor(puerto_kernel_dispatch, logger_servidor);
	
	// VALIDAR QUE EL VALOR DE LOS FILE DESCRIPTORS SON CORRECTOS
	if(server_cpu_interrupt_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }

	if(server_cpu_dispatch_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }

	
	log_info(logger_servidor, "Servidor INTERRUPT listo para recibir al cliente");
	log_info(logger_servidor, "Servidor DISPATCH listo para recibir al cliente");
	
	// ESPERA DE CONEXION
	int cliente_cpu_dispatch_fd = esperar_cliente(server_cpu_dispatch_fd);
	int cliente_cpu_interrupt_fd = esperar_cliente(server_cpu_interrupt_fd);
//	int cliente_fd = esperar_cliente(server_fd);

	// ARREGLAR ESTO PARA MANEJAR MENSAJES DE AMBOS CLIENTES
	manejar_conexion(server_cpu_dispatch_fd, cliente_cpu_dispatch_fd);
	manejar_conexion(server_cpu_interrupt_fd, cliente_cpu_interrupt_fd);
//	t_list* lista;
	*/

	// CONEXION DE KERNEL A MEMORIA Y MENSAJE DE PRUEBA
	char* ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
	char* puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
	log_info(logger_servidor, ip_memoria);
	int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("ALOHA DESDE KERNEL!", conexion_memoria);
	
	/*
	char* puerto_io = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");

	int server_io_fd = iniciar_servidor(puerto_io);

	if(server_io_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de IO");
        abort();
    }
	
	log_info(logger_servidor, "Servidor KERNEL listo para recibir al IO");
	int cliente_io_fd = esperar_cliente(server_io_fd);
	manejar_conexion(server_io_fd, cliente_io_fd);
	*/
}