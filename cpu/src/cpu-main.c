#include <utils/cliente/client.h>
#include <utils/server/server.h>
#include <utils/proceso/process.h>
int main(int argc, char* argv[]) {
	char* valor_prueba_interrupt = "Conectado a KERNEL desde INTERRUPT";
	char* valor_prueba_dispatch = "Conectado a KERNEL desde DISPATCH";
	char* valor_prueba_memoria = "Conectado a MEMORIA desde CPU";
    t_log* logger_cpu = iniciar_logger();
	t_config* config_cpu = iniciar_config("./cpu.config");
    
	char* puerto_kernel_interrupt = config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH");
    char* ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
	char* puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    char* ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");

	/* ---------------- LOGGING ---------------- */

	logger_cpu = log_create("cpu-int.log", "log", true, LOG_LEVEL_TRACE);
    // log_info(logger_cpu, "Primer log");
	
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	log_info(logger_cpu, ip_kernel);
	log_info(logger_cpu, puerto_kernel_interrupt);
	log_info(logger_cpu, puerto_kernel_dispatch);
	log_info(logger_cpu, ip_memoria);
	log_info(logger_cpu, puerto_memoria);
//	IP_MEMORIA=127.0.0.4
//	PUERTO_MEMORIA=40074
    // CREAMOS CONEXIONES DE DISPATCH E INTERRUPT HACIA EL KERNEL y asia memoria
	log_info(logger_cpu, "Intentando conectarse al PUERTO INTERRUPT del KERNEL");
	int conexion_kernel_interrupt_fd = crear_conexion(ip_kernel, puerto_kernel_interrupt);

	log_info(logger_cpu, "Intentando conectarse al PUERTO DISPATCH del KERNEL");
	int conexion_kernel_dispatch_fd = crear_conexion(ip_kernel, puerto_kernel_dispatch);

	log_info(logger_cpu, "Intentando conectarse a la MEMORIA");
	int conexion_memoria_fd = crear_conexion(ip_kernel, puerto_memoria);

	// VALIDAMOS LOS FILE DESCRIPTORS DE LAS CONEXIONES AL KERNEL
	if(conexion_kernel_interrupt_fd == -1){
			log_error(logger_cpu, "Error al iniciar conexion de interrupt");
			abort();
		}

	if(conexion_kernel_dispatch_fd == -1){
        log_error(logger_cpu, "Error al iniciar conexion de dispatch");
        abort();
    }

	if(conexion_memoria_fd == -1){
			log_error(logger_cpu, "Error al iniciar conexion de MEMORIA");
			abort();
		}
	// ENVIAMOS MENSAJES DE PRUEBA A AMBAS CONEXIONES DEL KERNEL
	enviar_mensaje(valor_prueba_interrupt, conexion_kernel_interrupt_fd);
	enviar_mensaje(valor_prueba_dispatch, conexion_kernel_dispatch_fd);
	enviar_mensaje(valor_prueba_memoria, conexion_memoria_fd);

//	while(1){
//	};
	pthread_t hilo_cliente_cpuInt_akernel;
    pthread_create(&hilo_cliente_cpuInt_akernel, NULL, (void*)manejar_conexion, (void*) conexion_kernel_interrupt_fd);
    pthread_detach(hilo_cliente_cpuInt_akernel);

	pthread_t hilo_cliente_cputDispatch_akernel;
    pthread_create(&hilo_cliente_cputDispatch_akernel, NULL, (void*)manejar_conexion, (void*) conexion_kernel_dispatch_fd);
    pthread_detach(hilo_cliente_cputDispatch_akernel);

	pthread_t hilo_cliente_cpu_amemoria;
    pthread_create(&hilo_cliente_cpu_amemoria, NULL, (void*)manejar_conexion, (void*) conexion_memoria_fd);
    pthread_join(hilo_cliente_cpu_amemoria, NULL);

    config_destroy(config_cpu);
	log_destroy(logger_cpu);


    return 0;
}
