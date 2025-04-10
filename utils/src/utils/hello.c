#include <utils/hello.h>

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	return nuevo_logger;
}

t_config* iniciar_config(char* ruta)
{
	t_config* nuevo_config = config_create(ruta);
	
	return nuevo_config;
}

void saludar(char* quien) {
    printf("Hola desde %s!!\n", quien);
}
