#include "utils-kernel-io.h"

void inicializar_lista_io()
{
	lista_de_io = malloc(sizeof(t_lista_io));
    lista_de_io->lista_ios = list_create();
	pthread_mutex_init(&lista_de_io->mutex_lista, NULL);
    log_info(logger_kernel, "Lista de IO inicializada");
}

t_io* inicializar_io(char* nombre_io, int socket_io) 
{
    t_io* io = malloc(sizeof(t_io));
    io->nombre = nombre_io;
    io->procesos = list_create();
    io->instancias = list_create();
    io->socket = socket_io;
	io->enabled = true;
    list_add(lista_de_io->lista_ios, io);
    log_debug(logger_kernel, "IO inicializado: %s", io->nombre);
    return io;
}

void insertar_nueva_instancia_io(t_list* instancias, int socket_io){
    t_instancia_io* instancia = malloc(sizeof(t_instancia_io));
    instancia->socket_io = socket_io;
    instancia->pid = -1;
    list_add(instancias, instancia);
    log_debug(logger_kernel, "Nueva instancia de IO agregada");
}

void encolar_pcb_en_interfaz(t_io* interfaz, uint8_t pid) 
{
    pthread_mutex_lock(&(lista_de_io->mutex_lista));
    list_add(interfaz->procesos, pid);
    pthread_mutex_unlock(&(lista_de_io->mutex_lista));
}

void alternar_estado_io(t_io* io){
    pthread_mutex_lock(&lista_de_io->mutex_lista);
    io->enabled = !(io->enabled);
    pthread_mutex_unlock(&lista_de_io->mutex_lista);
}

void eliminar_proceso_de_io(t_list* procesos_en_io, uint8_t pid) 
{
    // Función para comparar el PID
    bool _pid_en_io(void* ptr) {
        uint8_t proceso_pid = *((uint8_t*) ptr); // Desreferenciar el puntero correctamente
        return proceso_pid == pid;
    }

    // Eliminar el proceso de la lista
    void* elemento_eliminado = list_remove_by_condition(procesos_en_io, _pid_en_io);

    if (elemento_eliminado == NULL) {
        log_warning(logger_kernel, "No se encontró el proceso con PID %d en la lista de IO.", pid);
    } else {
        log_info(logger_kernel, "Proceso con PID %d eliminado de la lista de IO.", pid);
        free(elemento_eliminado); // Liberar la memoria del elemento eliminado
    }
}
