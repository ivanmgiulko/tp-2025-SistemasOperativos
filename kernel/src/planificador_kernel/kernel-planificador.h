#ifndef KERNEL_PLANIFICADOR_H_
#define KERNEL_PLANIFICADOR_H_

    #include "kernel-gestor.h"
    #include <utils/kernel-utils/kernel-structs-enums.h>

    /**
	 * @file
	 * @brief #include "planificador_kernel/kernel-planificador.h"
	 */
    
    /**
	 * @struct t_estado
	 * @brief similar al concepto de Monitor. Asocia una cola a un mutex
	 */
    
    extern t_lista_cpus* lista_cpus;

    /**
	 * @enum p_algoritmos
	 * @brief Algoritmos utilizados en el planificador de corto plazo
	 */
    typedef enum {
        FIFO,
        SJF,
        PMCP,
        SJF_SIN_DESALOJO
    } p_algoritmos;

    typedef struct {
        t_list* cola;
        pthread_mutex_t mutex;
    } t_estado;

    extern t_estado* estado_new;        
    extern t_estado* estado_ready;
    extern t_estado* estado_susp_ready;
    extern t_estado* estado_exec;
    extern t_estado* estado_blocked;
    extern t_estado* estado_susp_blocked;
    extern t_estado* estado_exit;

    extern pthread_mutex_t mutex_pid;
    extern sem_t sem_cantidad_pcbs_en_new;
    extern sem_t sem_cantidad_pcbs_en_ready;
    extern sem_t sem_cantidad_pcbs_en_blocked;
    extern sem_t sem_hay_espacio_en_memoria;

    extern sem_t bin_proceso_bloqueado;
    extern sem_t bin_proceso_eliminar;
    extern sem_t bin_cpu_disponible;
    
    
    /**
	* @brief Inicializa el struct t_estado
	* @returns un t_struct relacionado al manejo de colas
	*/
    t_estado* inicializar_estado(); 

    /**
	* @brief Inicializa todas las colas que se piden la consigna del TP (2025 - 1er Cuatri)
	* 
	*/
    void inicializar_estructuras();

    /**
	* @brief Pone en marcha la ejecucion del planificador a largo plazo
    * @param pcb_inicial pcb del proceso inicial enviado como argumento
    * 
	*/
    void iniciar_planificacion_largo_plazo();

    /**
	* @brief Inicia el planificador de largo plazo poniendo un "" en la consola (sacandolo del estado STOP)
    * @param pcb_inicial pcb del proceso inicial enviado como argumento
    * 
	*/
    void iniciar_planificador_largo_plazo();

    /**
	* @brief Inicia el planificador de mediano plazo poniendo un "" en la consola (sacandolo del estado STOP)
    * 
	*/
    void iniciar_planificador_mediano_plazo();

    /**
	* @brief Inicia el planificador de corto plazo poniendo un "" en la consola (sacandolo del estado STOP)
    * 
	*/
    void iniciar_planificador_corto_plazo();

    /**
	* @brief encola el proceso en la cola (dentro del struct t_estado) que nosotros le pasemos.
    * @param estado struct que contiene la cola en cuestion y un semaforo
    * @param pcb proceso para ser encolado en la cola que le pasamos
	*/
    void encolar_pcb_en_estado(t_estado* estado , t_pcb* pcb);

    /**
	* @brief crea el proceso 0, que es pasado por parametro por el ./bin/kernel
    * @param char* path del archivo de pseudocodigo del proceso 0
    * @param int tamanio en memoria del proceso 0
    * @return retorna el proceso 0 
	*/
    void crear_proceso_cero(char*, int);

    
    /**
	* @brief inicializa el pid_contador en 0 para ser puesto en los procesos
	*/
    void inicializar_pid();

    /**
	* @brief le asigna al proceso que le invoca un PID
    * @return devuelve el valor del PID de manera thread-safe
	*/
    int asignar_pid();

    /**
	* @brief quita el primer elemento de la cola del estado que le pasemos por parametro
    * @param t_estado estado del cual agarramos la cola y sacamos el primer proceso
    * @return devuelve el proceso sacado 
	*/
    t_pcb* pop_cola_mutex(t_estado*);

    
    /**
	* @brief agarra el primer proceso en la cola del estado NEW
    * @return devuelve el primer proceso en la cola NEW
	*/
    t_pcb* peek_pcb_en_new();

    /**
	* @brief agarra el primer proceso en la cola del estado que le pasemos
    * @return devuelve el primer proceso en la cola que le pasamos por parametro
	*/
    t_pcb* peek_cola_mutex(t_estado* cola_mutex);
    
    /**
	* @brief pasa el proceso que le pasemos por parametro al estado NEW
    * @param t_pcb* proceso que va a ser pasado a NEW
	*/
    void pasar_pcb_a_new(t_pcb*);

    /**
	* @brief pasa el proceso que le pasemos por parametro del estado NEW a READY
    * @param t_pcb* proceso que va a ser pasado a READY
	*/
    void pasar_pcb_new_a_ready(t_pcb* pcb);

    /**
	* @brief pasa el proceso que le pasemos por parametro del estado READY a EXEC
    * @param t_pcb* proceso que va a ser pasado a EXEC
	*/
    void pasar_pcb_ready_a_exec(t_pcb* pcb);

    void pasar_pcb_blocked_a_ready(t_pcb* pcb);

    void pasar_de_exec_a_blocked(t_pcb* pcb);
    
    void pasar_de_exec_a_exit(t_pcb* pcb);

    void pasar_pcb_blocked_a_exit(t_pcb* pcb);

    /**
	* @brief verifica si la cola NEW estaba vacia antes de que llegase un proceso a esta
    * @return devuelve TRUE si estaba vacia, o FALSE si no lo estaba.
	*/
    bool _verificar_cola_new_estaba_vacia();

    /**
	* @brief Le pregunta a la memoria si tiene espacio para enviar un proceso a la cola de READY
    * @return devuelve TRUE si es que hay espacio, o FALSE si no lo hay.
	*/
    bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new);

    /**
	* @brief Funcion utilizada en el algoritmo PMCP para ordenar la cola del estado NEW en relacion al tamnio de los procesos
    * @return devuelve TRUE cuando un proceso tiene menso tamanio que otro, o FALSO en el caso contrario.
	*/
    bool _tiene_menos_tamanio(void* a, void* b);

    /**
	* @brief Manda un proceso a la cola de NEW teniendo en cuenta el algortimo de ingreso a ready y si hay tamanio en la memoria
	*/
    void _enviar_proceso_new_a_cola_ready();

    void _enviar_proceso_susp_ready_a_cola_ready();

    bool _verificar_cola_susp_ready_esta_vacia();

    t_cpu_conectada* _buscar_cpu_libre();

#endif

