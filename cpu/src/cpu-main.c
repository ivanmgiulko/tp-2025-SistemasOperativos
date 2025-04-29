#include "cpu.h"
int main(int argc, char* argv[]){
	if(argc<2){
		fprintf(stderr, "Falta ID del CPU\n");
		return EXIT_FAILURE;
	}
	char* cpu_id = argv[1];
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
	char *directorioLogger = string_new(); // Gloria eterna al creador de las commons
	string_append(&directorioLogger,"cpu-");
	string_append(&directorioLogger, cpu_id);
		string_append(&directorioLogger,".log");

	logger_cpu = log_create(directorioLogger, "log", true, LOG_LEVEL_TRACE);
	log_info(logger_cpu, "%s", directorioLogger);
    // log_info(logger_cpu, "Primer log");
	free(directorioLogger);
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	log_info(logger_cpu, "ID CPU: %s", cpu_id);
	log_info(logger_cpu, "IP Kernel: %s", ip_kernel);
	log_info(logger_cpu, "Puerto Kernel Interrupt: %s",puerto_kernel_interrupt);
	log_info(logger_cpu, "Puerto Kernel Dispatch: %s", puerto_kernel_dispatch);
	log_info(logger_cpu, "IP Memoria: %s", ip_memoria);
	log_info(logger_cpu,"Puerto Memoria: %s", puerto_memoria);
//	IP_MEMORIA=127.0.0.4
//	PUERTO_MEMORIA=40074
    // CREAMOS CONEXIONES DE DISPATCH E INTERRUPT HACIA EL KERNEL y asia memoria
	log_info(logger_cpu, "Intentando conectarse al PUERTO INTERRUPT del KERNEL");
	fd_conexion_kernel_interrupt = crear_conexion(ip_kernel, puerto_kernel_interrupt);
	log_info(logger_cpu, "NOS CONECTAMOS AL KERNEL DESDE INTERRUPT!");

	log_info(logger_cpu, "Intentando conectarse al PUERTO DISPATCH del KERNEL");
	fd_conexion_kernel_dispatch = crear_conexion(ip_kernel, puerto_kernel_dispatch);
	log_info(logger_cpu, "NOS CONECTAMOS AL KERNEL DESDE DISPATCH!");

	log_info(logger_cpu, "Intentando conectarse a la MEMORIA");
	fd_conexion_memoria = crear_conexion(ip_kernel, puerto_memoria);
	log_info(logger_cpu, "NOS CONECTAMOS A LA MEMORIA");

	// VALIDAMOS LOS FILE DESCRIPTORS DE LAS CONEXIONES AL KERNEL
	if(fd_conexion_kernel_interrupt == -1){
			log_error(logger_cpu, "Error al iniciar conexion de interrupt");
			abort();
		}

	if(fd_conexion_kernel_dispatch == -1){
        log_error(logger_cpu, "Error al iniciar conexion de dispatch");
        abort();
    }

	if(fd_conexion_memoria == -1){
			log_error(logger_cpu, "Error al iniciar conexion de MEMORIA");
			abort();
		}
	// ENVIAMOS MENSAJES DE PRUEBA A AMBAS CONEXIONES DEL KERNEL
	enviar_mensaje(valor_prueba_interrupt, fd_conexion_kernel_interrupt);
	enviar_mensaje(valor_prueba_dispatch, fd_conexion_kernel_dispatch);
	enviar_mensaje(cpu_id, fd_conexion_kernel_dispatch);
	enviar_mensaje(valor_prueba_memoria, fd_conexion_memoria);

	//SIMULO PETICION DE INSTRUCCION A MEMORIA
	// t_peticion_instruccion peticion = {
    //     .pid = 1,
    //     .pc = 0
    // };

	// int size_peticion;
	// void* buffer_peticion = serializar_peticion_instruccion(&peticion, &size_peticion);

	// send(fd_conexion_memoria, buffer_peticion, size_peticion, 0);
	// free(buffer_peticion);

	// int size_respuesta;
	// void* buffer_respuesta = recibir_buffer(&size_respuesta, fd_conexion_memoria);

	// t_respuesta_instruccion* respuesta = deserializar_respuesta_instruccion(buffer_respuesta);
	// log_info(logger_cpu, "Instrucción recibida de Memoria: %s", respuesta->instruccion);

	// free(respuesta->instruccion);
	// free(respuesta);
	// free(buffer_respuesta);
	//FIN SIMULACION PETICION DE INSTRUCCION A MEMORIA
	
//	while(1){
//	};
	pthread_t hilo_cliente_cpuInt_akernel;
    pthread_create(&hilo_cliente_cpuInt_akernel, NULL, (void*)manejar_conexion_kernel_interrupt, NULL);
    pthread_detach(hilo_cliente_cpuInt_akernel);

	pthread_t hilo_cliente_cputDispatch_akernel;
    pthread_create(&hilo_cliente_cputDispatch_akernel, NULL, (void*)manejar_conexion_kernel_dispatch, NULL);
    pthread_detach(hilo_cliente_cputDispatch_akernel);

	pthread_t hilo_cliente_cpu_amemoria;
    pthread_create(&hilo_cliente_cpu_amemoria, NULL, (void*)manejar_conexion_memoria, NULL);
    pthread_join(hilo_cliente_cpu_amemoria, NULL);

    config_destroy(config_cpu);
	log_destroy(logger_cpu);


    return 0;
}
