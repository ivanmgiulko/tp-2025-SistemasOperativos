#include <utils/cliente/client.h>
#include <utils/server/server.h>
#include <utils/proceso/process.h>
#include "kernel-header.h"

int main(int argc, char* argv[]) {
// [archivo_pseudocodigo] [tamanio_proceso] 
	
	t_config* config_kernel = iniciar_config("./kernel.config");
	logger_servidor = log_create("kernel.log", "log", true, LOG_LEVEL_TRACE); // Crea el logger del Kernel
	
	int pid = 0;
	process proceso_ejemplo = iniciarProceso(pid);
	char *msgPrueba = string_new(); // Gloria eterna al creador de las commons
	string_append(&msgPrueba, string_itoa(proceso_ejemplo.metricas_estado.cantVecesExec));
	string_append(&msgPrueba," Se crea el proceso - Estado: NEW");
	log_info(logger_servidor, "%s", msgPrueba);
	free(msgPrueba);
	
	// CONEXION DE KERNEL A MEMORIA Y MENSAJE DE PRUEBA
	char* ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
	char* puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
	log_info(logger_servidor, "IP Memoria: %s",ip_memoria);
	int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("ALOHA DESDE KERNEL!", conexion_memoria);

	// INICIO SERVER KERNEL PARA RECIBIR A CPU
	char* puerto_kernel_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
	char* puerto_io = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");

	// VALIDAR QUE EL VALOR DE LOS FILE DESCRIPTORS SON CORRECTOS
	int server_kernel_interrupt_fd = iniciar_servidor(puerto_kernel_interrupt);
	if(server_kernel_interrupt_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }
	log_info(logger_servidor, "KERNEL listo para recibir al cliente CPU-Interrupt");

	int server_kernel_dispatch_fd = iniciar_servidor(puerto_kernel_dispatch);
	if(server_kernel_dispatch_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }
	log_info(logger_servidor, "KERNEL listo para recibir al cliente CPU-Dispatch");

	int server_io_fd = iniciar_servidor(puerto_io);
	if(server_io_fd == -1){
        log_error(logger_servidor, "Error al iniciar servidor de IO");
        abort();
    }
	log_info(logger_servidor, "KERNEL listo para recibir al cliente IO");

	// HILOS PARA MANEJAR LAS PETICIONES
	pthread_t hilo_servidor_kernel_interrupt;
    pthread_create(&hilo_servidor_kernel_interrupt, NULL, (void*)manejar_hilos, (void*)server_kernel_interrupt_fd);
    pthread_detach(hilo_servidor_kernel_interrupt);

	pthread_t hilo_servidor_io;
    pthread_create(&hilo_servidor_io, NULL, (void*)manejar_hilos, (void*)server_io_fd);
    pthread_detach(hilo_servidor_io);

	pthread_t hilo_servidor_kernel_dispatch;
    pthread_create(&hilo_servidor_kernel_dispatch, NULL, (void*)manejar_hilos, (void*)server_kernel_dispatch_fd);
    pthread_join(hilo_servidor_kernel_dispatch, NULL);
	
}