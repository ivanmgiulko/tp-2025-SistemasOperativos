#include <utils/hello.h>
#include "cpu-header.h"
int main(int argc, char* argv[]) {
    char* ip_kernel;
	char* puerto_kernel_interrupt;
	char* valor_prueba = "LLEGUE AL SERVER DESDE INTERRUPT!!!";
    t_log* logger_cpu = iniciar_logger();
	t_config* config_cpu = iniciar_config();
    

	/* ---------------- LOGGING ---------------- */

	logger_cpu = log_create("cpu-int.log", "log", true, LOG_LEVEL_TRACE);
    // log_info(logger_cpu, "Primer log");
	
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config_cpu = config_create("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/cpu/cpu.config");

	puerto_kernel_interrupt = config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
	ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
	log_info(logger_cpu, ip_kernel);
	log_info(logger_cpu, puerto_kernel_interrupt);
	
    // Creamos una conexión hacia el servidor, en este caso el KERNEL
	log_info(logger_cpu, "Intentando conectarse al PUERTO INTERRUPT del KERNEL");
	int conexion = crear_conexion(ip_kernel, puerto_kernel_interrupt);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor_prueba, conexion);

    config_destroy(config_cpu);
	log_destroy(logger_cpu);


    return 0;
}
