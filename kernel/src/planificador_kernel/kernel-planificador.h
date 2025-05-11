#ifndef KERNEL_PLANIFICADOR_H_
#define KERNEL_PLANIFICADOR_H_

    #include "kernel-gestor.h"

    /**
	 * @file
	 * @brief #include "planificador_kernel/kernel-planificador.h"
	 */
    
    /**
	 * @struct t_estado
	 * @brief similar al concepto de Monitor. Asocia una cola a un mutex
	 */
    

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
    extern sem_t sem_hay_espacio_en_memoria;
    

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
	* @brief Indica el algortimo que se utiliza a la hora de ejecutar el planificador de largo plazo
    *        (El algortimo es sacado del .config del Kernel)
	*/
    void decir_algoritmo();

    /**
	* @brief encola el proceso en la cola (dentro del struct t_estado) que nosotros le pasemos.
    * @param estado struct que contiene la cola en cuestion y un semaforo
    * @param pcb proceso para ser encolado en la cola que le pasamos
	*/
    void encolar_pcb(t_estado* estado , t_pcb* pcb);

    t_pcb* crear_proceso_cero(char*, int);

    void pasar_pcb_a_new(t_pcb*);

    void inicializar_pid();

    int asignar_pid();

    t_pcb* pop_cola_mutex(t_estado*);

    t_pcb* push_cola_mutex(t_estado* , t_pcb*);
    
    t_pcb* peek_pcb_en_new();
    
    void pasar_pcb_new_a_ready(t_pcb* pcb);

    bool verificar_cola_new_estaba_vacia();

    void enviar_proceso_a_io(uint8_t pid, int64_t tiempo, int socket_cliente);

    bool preguntar_a_memoria_espacio(t_pcb* pcb_en_new);

#endif

