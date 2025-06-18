#ifndef KERNEL_STRUCTS_ENUMS_H_
#define KERNEL_STRUCTS_ENUMS_H_

    #include <utils/utils.h>
    #include <utils/proceso/process.h>
    #include <utils/contrato/contrato.h>

    typedef enum{
		SOCKET_INTERRUPT,
		SOCKET_DISPATCH
	 } t_sockets_cpu;

	typedef struct {
        char* dispositivo;
        int64_t tiempo;
    } t_syscall_io;

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

    // Lista de las IOs
    
    typedef struct {
        t_list* lista_ios;
        pthread_mutex_t mutex_lista;
    } t_lista_io;

    typedef struct { // Elemento en 't_list* lista_ios'
        char* nombre;
        int socket;
        bool enabled;
        sem_t bin_interfaz_disponible;
        t_list* procesos;
    } t_io;
   
    typedef struct { // Elemento en 't_list* procesos'
        uint8_t pid;
	    int64_t tiempo;
    } t_info_proceso_en_io;

    typedef struct {
		uint8_t pid;
		bool respuesta;
	} t_respuesta_dump;

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

#endif // KERNEL_STRUCTS_ENUMS_H_