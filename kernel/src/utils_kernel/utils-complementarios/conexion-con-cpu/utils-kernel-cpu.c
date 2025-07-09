#include "utils-kernel-cpu.h"
#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>

void _agregar_socket_en_cpu(uint8_t id_cpu, t_sockets_cpu tipo_socket, int valor_socket) {

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    t_cpu_conectada* cpu_a_utilizar = _buscar_cpu_en_lista(id_cpu);
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    if(cpu_a_utilizar == NULL) { 
        pthread_mutex_lock(&lista_cpus->mutex_lista);
        cpu_a_utilizar = _agregar_cpu_en_lista(id_cpu);
        pthread_mutex_unlock(&lista_cpus->mutex_lista);
    }

    
    switch (tipo_socket) {
        case SOCKET_INTERRUPT:
            pthread_mutex_lock(&lista_cpus->mutex_lista);
            cpu_a_utilizar->socket_interrupt = valor_socket; // Asignar el socket de interrupt
            pthread_mutex_unlock(&lista_cpus->mutex_lista);
            break;
        case SOCKET_DISPATCH:
            pthread_mutex_lock(&lista_cpus->mutex_lista);
            cpu_a_utilizar->socket_dispatch = valor_socket; // Asignar el socket de dispatch
            pthread_mutex_unlock(&lista_cpus->mutex_lista);
            break;
    }
}

t_cpu_conectada* _agregar_cpu_en_lista(uint8_t id_cpu) 
{
    t_cpu_conectada* cpu_agregada = malloc(sizeof(t_cpu_conectada));

    cpu_agregada->id_cpu = id_cpu;
    cpu_agregada->socket_interrupt = -1; 
    cpu_agregada->socket_dispatch  = -1; 
    cpu_agregada->pid_en_cpu       = -1; 
    
    list_add(lista_cpus->lista_cpus, cpu_agregada);

    sem_post(&bin_cpu_disponible); // Posteo en base a los cpus disponibles -> 50000000 DE IQ

    return cpu_agregada;
}

void liberar_cpu_de_proceso(uint8_t pid) 
{
    t_cpu_conectada* cpu_a_liberar = NULL;
    
    pthread_mutex_lock(&lista_cpus->mutex_lista);
    cpu_a_liberar = _buscar_proceso_en_lista_cpu(pid);
    cpu_a_liberar->pid_en_cpu = -1;
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    log_debug(logger_kernel, "Se libero un CPU CARAJO");

    sem_post(&bin_cpu_disponible);
}

bool lista_de_io_vacia() 
{
    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    bool lista_vacia = lista_de_io == NULL;
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
    return lista_vacia;
}