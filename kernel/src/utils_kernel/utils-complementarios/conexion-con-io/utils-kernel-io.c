#include "utils-kernel-io.h"

void inicializar_lista_io()
{
	lista_de_io = malloc(sizeof(t_lista_io));
    lista_de_io->lista_ios = list_create();
	pthread_mutex_init(&lista_de_io->mutex_lista, NULL);
    log_info(logger_kernel, "Lista de IO inicializada");
}

void inicializar_io(char* nombre_io, int socket_io) 
{
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->socket = socket_io;
	io->enabled = true;
    sem_init(&io->bin_interfaz_disponible, 0, 0);
    list_add(lista_de_io->lista_ios, io);
    log_debug(logger_kernel, "IO inicializado: %s", io->nombre);
}

void encolar_pcb_en_interfaz(t_io* interfaz, t_info_proceso_en_io* pcb) 
{
    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    list_add(interfaz->procesos, pcb);
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
}

void alternar_estado_io(t_io* io){
    pthread_mutex_lock(&lista_de_io->mutex_lista);
    io->enabled = !(io->enabled);
    pthread_mutex_unlock(&lista_de_io->mutex_lista);
}

void eliminar_proceso_de_io(t_list* procesos_en_io, uint8_t pid) 
{
    void liberar_proceso_en_io(void* ptr){
        t_info_proceso_en_io* pcb = (t_info_proceso_en_io*) ptr;
        free(pcb);
    }

    bool _pid_en_io(void* ptr) {
        t_info_proceso_en_io* pcb = (t_info_proceso_en_io*) ptr;
        return pcb->pid == pid;
    }

    list_remove_and_destroy_by_condition(procesos_en_io, _pid_en_io, liberar_proceso_en_io);
}