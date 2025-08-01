#include "utils-kernel-io.h"

t_io* inicializar_io(char* nombre_io, int socket_io) 
{
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->instancias = list_create();
    io->socket = socket_io;
	io->enabled = true;
    pthread_mutex_init(&(io->mutex_lista), NULL);

    pthread_mutex_lock(&lista_de_io->mutex_lista);
    list_add(lista_de_io->lista_ios, io);
    pthread_mutex_unlock(&lista_de_io->mutex_lista);
    
    log_debug(logger_kernel, "IO inicializado: %s", io->nombre);
    return io;
}

t_instancia_io* insertar_nueva_instancia_io(t_io* io, int socket_io){
    t_instancia_io* instancia = malloc(sizeof(t_instancia_io));
    instancia->socket_io = socket_io;
    instancia->pid = -1;
    pthread_mutex_init(&(instancia->mutex_instancia), NULL);

    pthread_mutex_lock(&io->mutex_lista);
    list_add(io->instancias, instancia);
    pthread_mutex_unlock(&io->mutex_lista);

    return instancia;
    log_debug(logger_kernel, "Nueva instancia de IO agregada | socket: [%d]", socket_io);
}

void encolar_pcb_en_interfaz(t_io* interfaz, uint8_t* pid) 
{
    pthread_mutex_lock(&(interfaz->mutex_lista));
    list_add(interfaz->procesos, pid);
    pthread_mutex_unlock(&(interfaz->mutex_lista));
}

void alternar_estado_io(t_io* io){
    pthread_mutex_lock(&io->mutex_lista);
    io->enabled = !(io->enabled);
    pthread_mutex_unlock(&io->mutex_lista);
}

void eliminar_proceso_de_io(t_io* io, uint8_t pid) 
{

    // Función para comparar el PID
    bool _pid_en_io(void* ptr) {
        uint8_t* proceso_pid = (uint8_t*) ptr;
        return *proceso_pid == pid;
    }

    pthread_mutex_lock(&(io->mutex_lista));
    list_remove_by_condition(io->procesos, _pid_en_io);
    pthread_mutex_unlock(&(io->mutex_lista));
}

void eliminar_interfaz(t_io* interfaz) {
    list_destroy(interfaz->instancias);
    list_destroy(interfaz->procesos);
    free(interfaz->nombre);
	free(interfaz);
}
