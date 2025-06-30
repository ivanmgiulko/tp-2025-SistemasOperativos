#include <utils/utils.h>

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

char* convertir_cod_op_a_string(op_code codigo_operacion){
	switch(codigo_operacion) {
		case INSTRUCCION:
			return "INSTRUCCION";
		case FIN_PID:
			return "FIN_PID";
		case PROCESO_MEMORIA:
			return "PROCESO_MEMORIA";
		case PROCESO_SUSPENDIDO_MEMORIA:
			return "PROCESO_SUSPENDIDO_MEMORIA";
		case SUSPENSION_HECHA:
			return "SUSPENSION_HECHA";
		case RESPUESTA_MEMORIA:
			return "RESPUESTA_MEMORIA";
		case TAMANIO_PROCESO:
			return "TAMANIO_PROCESO";
		case CPU_PIDE_DATOS:
			return "CPU_PIDE_DATOS";
		case DATOS_DE_MEMORIA:
			return "DATOS_DE_MEMORIA";
		case OBTENER_MARCO_CORRESPONDIENTE:
			return "OBTENER_MARCO_CORRESPONDIENTE";
		case PROCESO_DUMPEAR:
			return "PROCESO_DUMPEAR";
		case RESPUESTA_DUMPEO:
			return "RESPUESTA_DUMPEO";
		case PROCESO_DESALOJAR:
			return "PROCESO_DESALOJAR";
		case PROCESO_DESALOJADO:
			return "PROCESO_DESALOJADO";
		case PROCESO_FINALIZAR:
			return "PROCESO_FINALIZAR";
		case PROCESO_FINALIZADO:
			return "PROCESO_FINALIZADO";
		case INFO_PROC_EXEC:
			return "INFO_PROC_EXEC";
		case INTERFAZ:
			return "INTERFAZ";
		case WRITE_MEMORIA:
			return "WRITE_MEMORIA";
		case READ_MEMORIA:
			return "READ_MEMORIA";
		default:
			return "[ERROR] Código de operación desconocido:";
	}
}