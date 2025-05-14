#include "kernel-main.h"

int main(int argc, char* argv[]) {
	// if(argc <= CANT_MINIMA_ARGUMENTOS){
	// 	fprintf(stderr, "Falta nombre Archivo de pseudocodigo y/o tamanio del proceso papu lince \n");
	// 	return EXIT_FAILURE;
	// }

	logger_kernel = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); // Crea el logger del Kernel

	configuracion_kernel = crear_config_kernel("./kernel.config", logger_kernel);

	// INICIO SERVER KERNEL PARA RECIBIR A CPU
	char* puerto_kernel_interrupt = configuracion_kernel->PUERTO_ESCUCHA_INTERRUPT;
	char* puerto_kernel_dispatch = configuracion_kernel->PUERTO_ESCUCHA_DISPATCH;
	char* puerto_io = configuracion_kernel->PUERTO_ESCUCHA_IO;

	// VALIDAR QUE EL VALOR DE LOS FILE DESCRIPTORS SON CORRECTOS
	fd_server_kernel_interrupt = iniciar_servidor(puerto_kernel_interrupt, logger_kernel);
	if(fd_server_kernel_interrupt == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }
	
	fd_server_kernel_dispatch = iniciar_servidor(puerto_kernel_dispatch, logger_kernel);
	if(fd_server_kernel_dispatch == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }
	
	fd_server_io = iniciar_servidor(puerto_io, logger_kernel);
	if(fd_server_io == -1){
        log_error(logger_kernel, "Error al iniciar servidor de IO");
        abort();
    }

	// decir_algoritmo();
	inicializar_estructuras();
	// Habria que contemplar que pasa si el usuario es un imbecil y no mete ni el path ni el tamanio del proceso -_-
	// Le borramos system32 por pescado.-S
	argv[1] = "/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt";
	argv[2] = "400";
	log_trace(logger_kernel, "Creando proceso cero con el path: %s y tamanio: %s", argv[1], argv[2]);
	t_pcb* pcb_proceso_cero = crear_proceso_cero(argv[1], atoi(argv[2]));
	
	// HILOS PARA MANEJAR LAS PETICIONES

	
	pthread_t hilo_servidor_io;
    pthread_create(&hilo_servidor_io, NULL, (void*)manejar_conexion_kernel_io, NULL);
    pthread_detach(hilo_servidor_io);

	pthread_t hilo_servidor_kernel_interrupt;
    pthread_create(&hilo_servidor_kernel_interrupt, NULL, (void*)manejar_conexion_kernel_interrupt, NULL);
    pthread_detach(hilo_servidor_kernel_interrupt);

	
	pthread_t hilo_servidor_kernel_dispatch;
    pthread_create(&hilo_servidor_kernel_dispatch, NULL, (void*)manejar_conexion_kernel_dispatch, NULL);
    pthread_detach(hilo_servidor_kernel_dispatch);


	pthread_t hilo_planificador_largo_plazo;
 	pthread_create(&hilo_planificador_largo_plazo, NULL, (void*)iniciar_planificador_largo_plazo, NULL);
	pthread_join(hilo_planificador_largo_plazo, NULL);
	
}