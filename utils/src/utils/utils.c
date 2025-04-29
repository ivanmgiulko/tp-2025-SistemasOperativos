#include <utils/utils.h>

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

void log_obligatorio(t_log* log, int pid, char* texto) {
	char *msgPrueba = string_new(); // Gloria eterna al creador de las commons
	string_append(&msgPrueba, string_itoa(pid));
	string_append(&msgPrueba, texto);
	log_info(log, "%s", msgPrueba);
	free(msgPrueba);

}