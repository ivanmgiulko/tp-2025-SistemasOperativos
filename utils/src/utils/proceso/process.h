#ifndef PROCESS_H_
#define PROCESS_H_

    #include <./utils/utils.h>
    #include <./utils/serializacion/serializacion.h>
    
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
    uint8_t cantVecesTP;            // Accesos a Tablas de Paginas
    uint8_t cantVecesInstrucciones; // Instrucciones solicitadas
    uint8_t cantVecesSWAP;          // Bajadas a SWAP
    uint8_t cantVecesMP;            // Subidad a Memoria Principal o espacio contiguo de memoria
    uint8_t cantVecesRead;
    uint8_t cantVecesWrite;
} metricas_proceso;

typedef struct {
        t_list* lista_ios;
        pthread_mutex_t mutex_lista;
} t_lista_io;

typedef struct { // Elemento en 't_list* lista_ios'
        char* nombre;
        int socket;
        bool enabled;
        t_list* instancias;
        t_list* procesos; // Solo PIDs (uint8_t)
        pthread_mutex_t mutex_lista;
} t_io;
   
typedef struct { // Elemento en 't_list* instancias'
        int socket_io;
        int8_t pid;
        pthread_mutex_t mutex_instancia;
} t_instancia_io;

typedef struct {
        char* dispositivo;
        t_instancia_io* instancia_utilizada; 
        int32_t tiempo;
} t_datos_io;

typedef struct 
{
    uint8_t pid;
    uint16_t pc;
    metricas_estado* metricas_estado;
    metricas_tiempo* metricas_tiempo;
    p_estados estadoProceso;

    int64_t estimacion_actual;
    int64_t estimacion_rafaga_anterior;
    int64_t tiempo_rafaga;
    int64_t estimacion_aux;

    char* pathArchivoPseudocodigo;
    uint32_t path_length;
    uint32_t tamanioMemoria;

    // Datos de IO
    t_datos_io* datos_io; 
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

    t_pcb* iniciarPCB(char* path, uint32_t tamanio, uint8_t pid, uint64_t estimacion_inicial);

    void enviar_proceso_a_memoria(t_pcb , int , uint32_t);

    t_pcbMemoria* deserializar_proceso(t_buffer* );

    t_contador* inicializar_contador();

    uint8_t _deserializar_pid(int* offset, t_paquete* paquete);

    uint16_t _deserializar_pc(int* offset, t_paquete* paquete);

#endif // PROCESS_H_
