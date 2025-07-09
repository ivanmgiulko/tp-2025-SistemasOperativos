#ifndef UTILS_PLANIFICADOR_H_
#define UTILS_PLANIFICADOR_H_

    #include "kernel-gestor.h"
    #include "utils_kernel/kernel-structs-enums.h"
    

    extern t_lista_cpus* lista_cpus;

    extern t_estado* estado_new;        
    extern t_estado* estado_ready;
    extern t_estado* estado_susp_ready;
    extern t_estado* estado_exec;
    extern t_estado* estado_blocked;
    extern t_estado* estado_blocked_aux;
    extern t_estado* estado_susp_blocked;
    extern t_estado* estado_exit;
 
    extern pthread_mutex_t mutex_pid;
    extern sem_t sem_cantidad_pcbs_en_new;
    extern sem_t sem_cantidad_pcbs_en_ready;
    extern sem_t sem_cantidad_pcbs_en_blocked;
    extern sem_t sem_hay_espacio_en_memoria;

    extern sem_t bin_eliminar_procesos_en_interfaces;
    extern sem_t bin_proceso_eliminar;
    extern sem_t bin_cpu_disponible;

    extern t_contador* pid_contador;
    extern t_temporal* tiempo_esperando;

    void _enviar_desde_new_a_ready(bool , char* );

    void _enviar_desde_susp_ready_a_ready(bool , char* );

    void _iniciar_cuando_apreta_enter();

    bool _chequear_interfaz_disponible(t_io* interfaz);

    void _enviar_proceso_susp_ready_a_cola_ready();

    bool _verificar_cola_susp_ready_esta_vacia();

    void administrar_proceso_bloqueado(void* _proceso_bloqueado);

    t_pcb* proceso_a_desalojar();

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
	* @brief Inicializa todas las colas que se piden la consigna del TP (2025 - 1er Cuatri)
	*/
    void inicializar_estructuras();

    /**
	* @brief Inicializa el struct t_estado
	* @returns un t_struct relacionado al manejo de colas
	*/
    t_estado* inicializar_estado(); 

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
	* @brief quita el primer elemento de la cola del estado que le pasemos por parametro
    * @param t_estado estado del cual agarramos la cola y sacamos el primer proceso
    * @return devuelve el proceso sacado 
	*/
    t_pcb* pop_cola_mutex(t_estado*);

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

    bool _menor_estimacion(void* a, void* b);

    void* _mayor_estimacion(void* a, void* b);

    void planificar_con_fifo();

    void planificar_con_sjf();
    
    void planificar_con_srt();

    void enviar_a_ejecutar_proceso(t_cpu_conectada* , t_pcb*);

#endif // UTILS_PLANIFICADOR_H_