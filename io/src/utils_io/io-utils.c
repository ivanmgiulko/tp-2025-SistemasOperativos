#include "io-utils.h"

void inicializar_estructuras(char* complemento_path) {

	char* path_relativo = string_duplicate("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/prueba/");
//	char* path_relativo = string_duplicate("/home/utnso/so-deploy/tp-2025-1c-FAMILIA-MATRIX/prueba/");

	string_append(&path_relativo, complemento_path);

	config_io = iniciar_config(path_relativo);

	free(path_relativo);
    
	logger_io = log_create("io.log", "log", true, LOG_LEVEL_TRACE);
	
}

void finalizar_estructuras() {
    config_destroy(config_io);
	log_destroy(logger_io);
}

void enviar_interfaz_a_kernel(char* nombre_io) {
    char* ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
	char* puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");

    // Creamos una conexión hacia el servidor, en este caso el KERNEL
	conexion_kernel_fd = crear_conexion(ip_kernel, puerto_kernel);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_nombre_interfaz(nombre_io, conexion_kernel_fd);
}