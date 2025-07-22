#include "kernel-main.h"

#include <utils_kernel/utils-complementarios/archivo-main/utils-main.h>
#include <utils_kernel/manejar-conexiones/modulo-cpu/manejar-conexion-cpu.h>
#include <utils_kernel/manejar-conexiones/modulo-io/manejar-conexion-io.h>

int main(int argc, char* argv[]) {
	// if(argc <= CANT_MINIMA_ARGUMENTOS){
	// 	fprintf(stderr, "Falta nombre Archivo de pseudocodigo y/o tamanio del proceso papu lince \n");
	// 	return EXIT_FAILURE;
	// }

	// _chequear_segundo_argumento_es_numero(argv[2]);

	argv[1] = "PATH_INSTRUCCIONES";

	argv[2] = "256";

	argv[3] = "prueba/";

	argv[4] = "kernel.config";

	char* path_relativo = string_duplicate("/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/prueba/");
	string_append(&path_relativo, argv[3]);
	
	// Iniciamos las Colas con sus respectivos Mutex (como si fuera un monitor); tambien inicia Log y Config
	inicializar_estructuras(path_relativo, argv[4]);
	configuracion_kernel->RUTA_RELATIVA = string_duplicate(path_relativo);
	
	// Iniciamos la comunicacion con CPUs y con IO 
	_iniciar_server_para_cpu_interrupt();

	_iniciar_server_para_cpu_dispatch();

	_iniciar_server_para_io();

	string_append(&path_relativo, argv[1]);
	string_append(&path_relativo, ".txt");

	log_debug(logger_kernel, "proc 0 - path: %s - tamanio: %s", path_relativo, argv[2]);
	crear_proceso_cero(path_relativo, atoi(argv[2]));
	
	pthread_t hilo_planificador_largo_plazo;
 	pthread_create(&hilo_planificador_largo_plazo, NULL, (void*)iniciar_planificacion_largo_plazo, NULL);
	pthread_join(hilo_planificador_largo_plazo, NULL);

	free(configuracion_kernel);	

	free(path_relativo);

	eliminar_lista_cpu();
	
}