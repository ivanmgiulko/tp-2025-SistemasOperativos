#include <utils/client.h>

int main(int argc, char* argv[]) {

	char* ip_kernel;
	char* puerto_kernel;
	char* valor_prueba = "LLEGUE AL SERVER!!!";

	t_log* logger_io = iniciar_logger();
	t_config* config_io = iniciar_config("./io.config");
    
	/* ---------------- LOGGING ---------------- */

	logger_io = log_create("io.log", "log", true, LOG_LEVEL_TRACE);
    // log_info(logger_io, "Primer log");
	
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");
	log_info(logger_io, ip_kernel);
	log_info(logger_io, puerto_kernel);
	
    // Creamos una conexión hacia el servidor, en este caso el KERNEL
	int conexion = crear_conexion(ip_kernel, puerto_kernel);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor_prueba, conexion);

	// 	paquete(conexion);
    config_destroy(config_io);
	log_destroy(logger_io);


    return 0;
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete = crear_paquete();

	while (1) {
        leido = readline("> ");

        if (leido) {
            add_history(leido);
        }
        if (!strncmp(leido, "exit", 4) || !strncmp(leido, "\0", 2)){
            free(leido);
            break;
        }
		// Leemos y esta vez agregamos las lineas al paquete
        agregar_a_paquete(paquete, leido, strlen(leido) + 1);
        free(leido);
    }
	
	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	enviar_paquete(paquete, conexion);
	
	eliminar_paquete(paquete);

}