#ifndef KERNEL_STRUCTS_ENUMS_H_
#define KERNEL_STRUCTS_ENUMS_H_

    #include <utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/contrato/contrato.h>

    typedef enum{
		SOCKET_INTERRUPT,
		SOCKET_DISPATCH
	 } t_sockets_cpu;

    /* LISTAS COMPARTIDAS CON SUS ELEMENTOS */
    
    // Lista de las CPUs
    typedef struct {
        t_list* lista_cpus;
        pthread_mutex_t mutex_lista;
    } t_lista_cpus;
    
    typedef struct { 
	    int socket_interrupt;
	    int socket_dispatch;
	    uint8_t id_cpu;
	    int8_t pid_en_cpu;
    } t_cpu_conectada; // Este elemento se agrega en el t_list* lista_cpus

    typedef struct {
		uint8_t pid;
		bool respuesta;
	} t_respuesta_dump;

    typedef struct {
        t_list* cola;
        pthread_mutex_t mutex;
    } t_estado;

#endif // KERNEL_STRUCTS_ENUMS_H_