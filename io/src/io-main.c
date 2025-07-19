#include "io-main.h"

int main(int argc, char* argv[]) {
	if(argc <= CANT_MINIMA_ARGUMENTOS){
		fprintf(stderr, "Falta nombre de identificacion de dispositivo IO \n");
		return EXIT_FAILURE;
	}

	char* io_nombre = argv[1];
	char* ip_kernel;
	char* puerto_kernel;

	char* path_relativo = string_duplicate("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/prueba/");
	string_append(&path_relativo, argv[2]);

	t_config* config_io = iniciar_config(path_relativo);
    
	/* ---------------- LOGGING ---------------- */
	logger_io = log_create("io.log", "log", true, LOG_LEVEL_TRACE);
    // log_info(logger_io, "Primer log");
	
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");
	log_info(logger_io, "Nombre de dispositivo IO: %s", io_nombre);
	log_info(logger_io, "IP Kernel: %s", ip_kernel);
	log_info(logger_io, "Puerto Kernel: %s", puerto_kernel);
	
    // Creamos una conexión hacia el servidor, en este caso el KERNEL
	conexion_kernel_fd = crear_conexion(ip_kernel, puerto_kernel);
	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_nombre_interfaz(io_nombre, conexion_kernel_fd);
	
	pthread_t hilo_cliente_io_akernel;
    pthread_create(&hilo_cliente_io_akernel, NULL, (void*)manejar_conexion_io, (void*) conexion_kernel_fd);
    pthread_join(hilo_cliente_io_akernel, NULL);
	
    config_destroy(config_io);
	log_destroy(logger_io);


    return 0;
}

