#ifndef PROCESS_H_
#define PROCESS_H_

    #include <./utils/utils.h>

typedef enum {
    NEW, 
    READY, 
    EXEC, 
    BLOCKED,
    EXIT,
    SUSP_READY,
    SUSP_BLOCEKD
} p_estados;

typedef struct metricas_estado
{
    uint8_t cantVecesNew;
    uint8_t cantVecesReady;
    uint8_t cantVecesExec;
    uint8_t cantVecesBlocked;
    uint8_t cantVecesSuspReady;
    uint8_t cantVecesSuspBlocked;
    uint8_t cantVecesExit;
} metricas_estado;

typedef struct metricas_tiempo // Los tiempos son devueltos en milisegundos. Revisar commons para esto
{
    int64_t tiempoEnNew;
    int64_t tiempoEnReady;
    int64_t tiempoEnExec;
    int64_t tiempoEnBlocked;
    int64_t tiempoEnSuspReady;
    int64_t tiempoEnSuspBlocked;
    int64_t tiempoEnExit;
} metricas_tiempo;

typedef struct 
{
    uint8_t pid;
    uint16_t pc;
    metricas_estado metricas_estado;
    metricas_tiempo metricas_tiempo;
    p_estados estadoProceso;

    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    uint32_t tamanioMemoria;

} t_pcb; 

typedef struct 
{
    uint8_t pid;
    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    uint32_t tamanioMemoria;

} t_pcbMemoria; // Esto es lo que Memoria debe saber del PCB que es enviado desde Kernel

typedef struct {
        uint8_t pid;
		int64_t tiempo;
    } t_info_proceso_en_io;

typedef struct  {
    int valor;
    pthread_mutex_t mutex;
} t_contador;

typedef struct {
    char* nombre;
	pthread_mutex_t mutex;
    t_list* procesos;
    int socket;
	uint64_t tiempo_ultimo_bloqueo;
	bool enabled;
} t_io;


metricas_estado iniciarMetricasEstado();
metricas_tiempo iniciarMetricasTiempo();
t_pcb* iniciarPCB(char* , int , int );
void enviarProceso_A_Memoria(t_pcb , int );
void enviar_proceso_a_finalizar_Memoria(t_pcb , int );

// void enviar_proceso_a_finalizar_kernel(t_paquete, int, int);

t_pcbMemoria* deserializarProceso(t_buffer* );

t_contador* inicializar_contador();

#endif // PROCESS_H_
