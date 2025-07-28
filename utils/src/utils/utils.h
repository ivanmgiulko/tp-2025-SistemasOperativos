#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include <time.h>
#include <sys/stat.h>

typedef enum
{
	MENSAJE = 0,
	PAQUETE = 1,

	INSTRUCCION,
	FIN_PID,

	PROCESO_MEMORIA,
	PROCESO_SUSPENDIDO_MEMORIA,
	SUSPENSION_HECHA,
	RESPUESTA_MEMORIA, 
	TAMANIO_PROCESO, 
	CPU_PIDE_DATOS,
	DATOS_DE_MEMORIA,
	OBTENER_MARCO_CORRESPONDIENTE,
	KERNEL_CONECTADO,
	
	PROCESO_DUMPEAR,
	RESPUESTA_DUMPEO,

	PROCESO_DESALOJAR,
	PROCESO_DESALOJADO,

	PROCESO_FINALIZAR,
	PROCESO_FINALIZADO,

	INFO_PROC_EXEC,
	
	INTERFAZ,
	WRITE_MEMORIA,
	READ_MEMORIA,
	// Syscalls que recibe Kernel desde CPU INTERRUPT

	SYSCALL_IO,
	SYSCALL_EXIT,
	SYSCALL_INIT_PROC,
	SYSCALL_DUMP_MEMORY,

	PROCESO_BLOQUEADO,
	PROCESO_SUSPENDIDO_BLOQUEADO,
	PROCESO_DESBLOQUEADO,
	PROCESO_SUSPENDIDO_DESBLOQUEADO,

	LINUS_TORVALDS = -1
} op_code;


typedef struct
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void saludar(char* quien);
t_config* iniciar_config(char*);
void log_obligatorio(t_log* log, int pid, char* texto);

char* convertir_cod_op_a_string(op_code codigo_operacion);
#endif
