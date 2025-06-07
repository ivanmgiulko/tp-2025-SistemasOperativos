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
    t_temporal* tiempoEnNew;
    t_temporal* tiempoEnReady;
    t_temporal* tiempoEnExec;
    t_temporal* tiempoEnBlocked;
    t_temporal* tiempoEnSuspReady;
    t_temporal* tiempoEnSuspBlocked;
    t_temporal* tiempoEnExit;
} metricas_tiempo;

typedef struct metricas_proceso
{
    uint8_t cantVecesTP; // Accesos a Tablas de Paginas
    uint8_t cantVecesInstrucciones; // Instrucciones solicitadas
    uint8_t cantVecesSWAP; // Bajadas a SWAP
    uint8_t cantVecesMP; // Subidad a Memoria Principal o espacio contiguo de memoria
    uint8_t cantVecesRead;
    uint8_t cantVecesWrite;
} metricas_proceso;

typedef struct 
{
    uint8_t pid;
    uint16_t pc;
    metricas_estado* metricas_estado;
    metricas_tiempo* metricas_tiempo;
    p_estados estadoProceso;

    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    uint32_t tamanioMemoria;

} t_pcb; 

typedef struct 
{
    uint8_t pid;
    metricas_proceso metricas_proceso;

    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    uint32_t tamanioMemoria;

} t_pcbMemoria; // Esto es lo que Memoria debe saber del PCB que es enviado desde Kernel

typedef struct  {
    int valor;
    pthread_mutex_t mutex;
} t_contador;



metricas_estado* iniciarMetricasEstado();
metricas_tiempo* iniciarMetricasTiempo();
metricas_proceso iniciarMetricasProceso();

t_pcb* iniciarPCB(char* , int , int );
void enviarProceso_A_Memoria(t_pcb , int );
void enviar_proceso_a_finalizar_Memoria(t_pcb , int );

// void enviar_proceso_a_finalizar_kernel(t_paquete, int, int);

t_pcbMemoria* deserializarProceso(t_buffer* );

t_contador* inicializar_contador();

#endif // PROCESS_H_
