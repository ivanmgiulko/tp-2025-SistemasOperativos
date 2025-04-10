#include <utils/hello.h>
#include "cpu-header.h"
int main(int argc, char* argv[]) {
	char* valor_prueba_interrupt = "Conectado a KERNEL desde INTERRUPT";
	char* valor_prueba_dispatch = "Conectado a KERNEL desde DISPATCH";
    t_log* logger_cpu = iniciar_logger();
	t_config* config_cpu = iniciar_config("./cpu.config");
    
	char* puerto_kernel_interrupt = config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH");
    char* ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");

	/* ---------------- LOGGING ---------------- */

	logger_cpu = log_create("cpu-int.log", "log", true, LOG_LEVEL_TRACE);
    // log_info(logger_cpu, "Primer log");
	
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	log_info(logger_cpu, ip_kernel);
	log_info(logger_cpu, puerto_kernel_interrupt);
	
    // CREAMOS CONEXIONES DE DISPATCH E INTERRUPT HACIA EL KERNEL
	log_info(logger_cpu, "Intentando conectarse al PUERTO INTERRUPT del KERNEL");
	int conexion_kernel_interrupt_fd = crear_conexion(ip_kernel, puerto_kernel_interrupt);
	int conexion_kernel_dispatch_fd = crear_conexion(ip_kernel, puerto_kernel_dispatch);

	// VALIDAMOS LOS FILE DESCRIPTORS DE LAS CONEXIONES AL KERNEL
	if(conexion_kernel_interrupt_fd == -1){
			log_error(logger_cpu, "Error al iniciar conexion de interrupt");
			abort();
		}

	if(conexion_kernel_dispatch_fd == -1){
        log_error(logger_cpu, "Error al iniciar conexion de dispatch");
        abort();
    }

	// ENVIAMOS MENSAJES DE PRUEBA A AMBAS CONEXIONES DEL KERNEL
	enviar_mensaje(valor_prueba_interrupt, conexion_kernel_interrupt_fd);
	 enviar_mensaje(valor_prueba_dispatch, conexion_kernel_dispatch_fd);
	while(1){
	};
    config_destroy(config_cpu);
	log_destroy(logger_cpu);


    return 0;
}
