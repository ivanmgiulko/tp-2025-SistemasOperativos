#include "io-main.h"

int main(int argc, char* argv[]) {
	
	if(argc <= CANT_MINIMA_ARGUMENTOS){
		fprintf(stderr, "Falta nombre de identificacion de dispositivo IO \n");
		return EXIT_FAILURE;
	}

	inicializar_estructuras(argv[2]);

	enviar_interfaz_a_kernel(argv[1]);
	
	pthread_t hilo_cliente_io_akernel;
    pthread_create(&hilo_cliente_io_akernel, NULL, (void*)manejar_conexion_io, (void*) conexion_kernel_fd);
    pthread_join(hilo_cliente_io_akernel, NULL);
	
	finalizar_estructuras();
    
    return 0;
}

