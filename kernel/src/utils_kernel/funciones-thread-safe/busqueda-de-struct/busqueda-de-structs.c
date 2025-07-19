#include "busqueda-de-structs.h"

t_cpu_conectada* _buscar_cpu_libre() { 

    bool _cpu_esta_libre(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->pid_en_cpu == -1;
    }

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    t_cpu_conectada* cpu_encontrada = list_find(lista_cpus->lista_cpus, _cpu_esta_libre);
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    return cpu_encontrada;
}

t_cpu_conectada* _buscar_cpu_en_lista(uint8_t id_cpu)
{
    bool _cpu_tiene_id(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->id_cpu == id_cpu;
    }
    pthread_mutex_lock(&lista_cpus->mutex_lista);
    t_cpu_conectada* cpu_encontrada = list_find(lista_cpus->lista_cpus, _cpu_tiene_id);
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    return cpu_encontrada;
}

t_cpu_conectada* buscar_cpu_que_usa_proceso(t_list* cpus, uint8_t pid) 
{
    bool _es_el_proceso(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->pid_en_cpu == pid;
    }

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    t_cpu_conectada* cpu_encontrada = list_find(cpus, _es_el_proceso);
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    return cpu_encontrada;
}

t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid)
{
    bool _cpu_tiene_pid(void* ptr) {
        t_cpu_conectada* cpu_con_proceso = (t_cpu_conectada*) ptr;
        return cpu_con_proceso->pid_en_cpu == pid;
    }

    pthread_mutex_lock(&lista_cpus->mutex_lista);
    t_cpu_conectada* cpu_encontrada = list_find(lista_cpus->lista_cpus, _cpu_tiene_pid);
    pthread_mutex_unlock(&lista_cpus->mutex_lista);

    return cpu_encontrada;
}

t_io* buscar_io(char* nombre_io) {
    
    bool _es_el_io(void* elemento) {
        t_io* io = (t_io*) elemento;
        return string_contains(io->nombre, nombre_io);
    }

    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    t_io* io_encontrado = list_find(lista_de_io->lista_ios, _es_el_io);
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));

    return io_encontrado;

}

t_instancia_io* buscar_instancia_disponible(t_io* io) {

    bool _esta_libre(void* elemento) {
        t_instancia_io* io = (t_instancia_io*) elemento;
        return io->pid == -1; 
    }
    
    pthread_mutex_lock(&(io->mutex_lista));
    t_instancia_io* instancia_io_encontrada = list_find(io->instancias, _esta_libre);
    pthread_mutex_unlock(&(io->mutex_lista));

    return instancia_io_encontrada;
}

t_instancia_io* eliminar_y_devolver_instancia(t_io* io, int socket_io) {
    
    bool _esta_libre(void* elemento) {
        t_instancia_io* io = (t_instancia_io*) elemento;
        return io->socket_io == socket_io; 
    }
    
    pthread_mutex_lock(&(io->mutex_lista));
    t_instancia_io* instancia_io_removida = list_remove_by_condition(io->instancias, _esta_libre);
    pthread_mutex_unlock(&(io->mutex_lista));

    return instancia_io_removida;

}

t_instancia_io* devolver_instancia_disponible(char* nombre_interfaz) { 
    t_io* tipo_de_io = buscar_io(nombre_interfaz);
    
    if(tipo_de_io != NULL){
        return buscar_instancia_disponible(tipo_de_io);
    }

    return NULL;
}

// funcion que busca la io donde se encuentra el pid
t_io* buscar_io_en_lista(int socket) {
    
    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    int tamanio_lista = list_size(lista_de_io->lista_ios);
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
    int cont = 0;
    
    do {
        
        pthread_mutex_lock(&(lista_de_io->mutex_lista));
        t_io* _elemento_pivote = list_get(lista_de_io->lista_ios, cont);
        pthread_mutex_unlock(&(lista_de_io->mutex_lista));

        t_instancia_io* _proceso_encontrado = buscar_proceso_en_elemento(_elemento_pivote, socket);

        if(_proceso_encontrado == NULL) { 
            cont++;
        } else { 
            return _elemento_pivote;
        }

    } while(cont <= tamanio_lista);
    
    return NULL;
}

t_instancia_io* buscar_proceso_en_elemento(t_io* io, int socket) {

    bool _es_la_instancia(void* elemento) {
        t_instancia_io* instancia = (t_instancia_io*) elemento;
        return instancia->socket_io == socket;
    }

    pthread_mutex_lock(&(io->mutex_lista));
    t_instancia_io* instancia_io_encontrada = list_find(io->instancias, _es_la_instancia);
    pthread_mutex_unlock(&(io->mutex_lista));

    return instancia_io_encontrada;
}


t_pcb* buscar_proceso_en_cola(t_estado* estado, uint8_t pid) 
{
    bool _tiene_el_pid(void* ptr) {
        t_pcb* proceso = (t_pcb*) ptr;
        return proceso->pid == pid;
    }
    pthread_mutex_lock(&(estado->mutex));
    t_pcb * proceso_encontrado = list_find(estado->cola, _tiene_el_pid);
    pthread_mutex_unlock(&(estado->mutex));

    return proceso_encontrado;

}