#include <utils/client.h>
#include <utils/server.h>

int main(int argc, char* argv[]) {
	
	
	t_config* config_kernel = iniciar_config("./kernel.config");
	t_log* logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); // Crea el logger del Kernel

	
	char* puerto_kernel_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
	char* ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
	char* puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
	
	// INICIO SERVER KERNEL
	int server_cpu_interrupt_fd = iniciar_servidor(puerto_kernel_interrupt, logger_kernel);
	int server_cpu_dispatch_fd = iniciar_servidor(puerto_kernel_dispatch, logger_kernel);
	
	// VALIDAR QUE EL VALOR DE LOS FILE DESCRIPTORS SON CORRECTOS
	if(server_cpu_interrupt_fd == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }

	if(server_cpu_dispatch_fd == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }

	
	log_info(logger_kernel, "Servidor INTERRUPT listo para recibir al cliente");
	log_info(logger_kernel, "Servidor DISPATCH listo para recibir al cliente");
	
	// CONEXION DE KERNEL A MEMORIA Y MENSAJE DE PRUEBA

	int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje(config_get_string_value(config_kernel, "VALOR_PRUEBA"), conexion_memoria);
	
	// ESPERA DE CONEXION
	
	int cliente_cpu_dispatch_fd = esperar_cliente(server_cpu_dispatch_fd, logger_kernel);
	int cliente_cpu_interrupt_fd = esperar_cliente(server_cpu_interrupt_fd,  logger_kernel);
//	int cliente_fd = esperar_cliente(server_fd);

	// ARREGLAR ESTO PARA MANEJAR MENSAJES DE AMBOS CLIENTES
	manejar_conexion(server_cpu_dispatch_fd, cliente_cpu_dispatch_fd, logger_kernel);
	manejar_conexion(server_cpu_interrupt_fd, cliente_cpu_interrupt_fd, logger_kernel);
//	t_list* lista;
	
/*
	#define PUERTO_KERNEL "40073"
	char* puerto_io = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");

	int server_io_fd = iniciar_servidor(PUERTO_KERNEL, logger_servidor);

	if(server_io_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de IO");
        abort();
    }
	
	log_info(logger_servidor, "Servidor KERNEL listo para recibir al IO");
	int cliente_io_fd = esperar_cliente(server_io_fd, logger_servidor);
	manejar_conexion(server_io_fd, cliente_io_fd, logger_servidor);
	*/
}