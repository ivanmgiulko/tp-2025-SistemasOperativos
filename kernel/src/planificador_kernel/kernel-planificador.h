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
    typedef struct {
        t_queue* cola;
        pthread_mutex_t mutex;
    } t_estado;

    /**
	 * @enum p_algoritmos
	 * @brief Algoritmos utilizados en el planificador de corto plazo
	 */
    typedef enum{
        FIFO,
        SJF,
        SJF_SIN_DESALOJO
    }p_algoritmos;

    extern pthread_mutex_t mutex_pid;
    extern sem_t sem_cantidad_pcbs_en_new;

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
    void iniciar_planificacion_largoPlazo(t_pcb* pcb_inicial);

    /**
	* @brief Inicia el planificador de largo plazo poniendo un "" en la consola (sacandolo del estado STOP)
    * @param pcb_inicial pcb del proceso inicial enviado como argumento
    * 
	*/
    void iniciar_planificador_largoPlazo(t_pcb* pcb_inicial);

    /**
	* @brief Indica el algortimo que se utiliza a la hora de ejecutar el planificador de largo plazo
    *        (El algortimo es sacado del .config del Kernel)
	*/
    void decir_algoritmo();

    void encolar_pcb(t_estado* estado , t_pcb* pcb);

#endif

