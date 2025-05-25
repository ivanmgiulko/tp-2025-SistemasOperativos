#include"kernel-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

void _chequear_segundo_argumento_es_numero(char* numero_en_string) 
{
    int longitud_string = string_length(numero_en_string);

    int num_prueba = atoi(numero_en_string);
    char* num_prueba_en_string = string_itoa(num_prueba);
    int num_prueba_en_string_length = string_length(num_prueba_en_string);
    free(num_prueba_en_string);
    
    if(!(longitud_string == num_prueba_en_string_length)) { 
        fprintf(stderr, "Bro realmente puso mal el tamanio en memoria \n");
        abort();
    }
}

// HILOS PARA MANEJAR LAS PETICIONES
void _iniciar_server_para_cpu_interrupt() 
{ 
    char* puerto_kernel_interrupt = configuracion_kernel->PUERTO_ESCUCHA_INTERRUPT;

    fd_server_kernel_interrupt = iniciar_servidor(puerto_kernel_interrupt, logger_kernel);
	if(fd_server_kernel_interrupt == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-interrupt");
        abort();
    }

    pthread_t hilo_servidor_kernel_interrupt;
    pthread_create(&hilo_servidor_kernel_interrupt, NULL, (void*)manejar_conexion_kernel_interrupt, NULL);
    pthread_detach(hilo_servidor_kernel_interrupt);
}

void _iniciar_server_para_cpu_dispatch() 
{ 
    char* puerto_kernel_dispatch = configuracion_kernel->PUERTO_ESCUCHA_DISPATCH;

    fd_server_kernel_dispatch = iniciar_servidor(puerto_kernel_dispatch, logger_kernel);
	if(fd_server_kernel_dispatch == -1){
        log_error(logger_kernel, "Error al iniciar servidor de CPU-dispatch");
        abort();
    }

    pthread_t hilo_servidor_kernel_dispatch;
    pthread_create(&hilo_servidor_kernel_dispatch, NULL, (void*)manejar_conexion_kernel_dispatch, NULL);
    pthread_detach(hilo_servidor_kernel_dispatch);
}

void _iniciar_server_para_io() 
{ 
    char* puerto_io = configuracion_kernel->PUERTO_ESCUCHA_IO;

    fd_server_io = iniciar_servidor(puerto_io, logger_kernel);
	if(fd_server_io == -1){
        log_error(logger_kernel, "Error al iniciar servidor de IO");
        abort();
    }

    pthread_t hilo_servidor_io;
    pthread_create(&hilo_servidor_io, NULL, (void*)manejar_conexion_kernel_io, NULL);
    pthread_detach(hilo_servidor_io);
}

void _enviar_desde_new_a_ready(bool _cola_new_estaba_vacia, char* algortimo_ingreso_ready) 
{ 
    if(_cola_new_estaba_vacia) {

    _enviar_proceso_new_a_cola_ready();

    } else {

        if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
            
             _enviar_proceso_new_a_cola_ready();
            
        }

         if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                
            list_sort(estado_new->cola, _tiene_menos_tamanio);

            _enviar_proceso_new_a_cola_ready();
        }
    }
}

void _enviar_desde_susp_ready_a_ready(bool _cola_new_estaba_vacia, char* algortimo_ingreso_ready) 
{
    if(_cola_new_estaba_vacia) {
           
        _enviar_proceso_susp_ready_a_cola_ready();

    } else {

         if(strcmp(algortimo_ingreso_ready, "FIFO") == 0) { 
                
            _enviar_proceso_susp_ready_a_cola_ready();
            
        }

        if(strcmp(algortimo_ingreso_ready, "PMCP") == 0) {
                
            list_sort(estado_susp_ready->cola, _tiene_menos_tamanio);

            _enviar_proceso_susp_ready_a_cola_ready();
        }
    }
}

void _iniciar_cuando_apreta_enter() { 
    char *leido;
	bool lineaVacia = false;
	do {
		leido = readline("> ");
		if(strcmp(leido, "") == 0){
			lineaVacia = !lineaVacia;
            // Se rompe el While e inicia el planificador a largo plazo
		} else {
			free(leido);
		}
	} while(!lineaVacia);
}

t_io* buscar_io_en_lista(t_list* lista_base, uint8_t pid) {
    bool flag = true;
    int tamanio_lista = list_size(lista_base), cont = 0;
    
    do { 
        t_io* _elemento_pivote = list_get(lista_de_io, cont);

        t_info_proceso_en_io* _proceso_encontrado = buscar_proceso_en_elemento(_elemento_pivote->procesos, pid);

        if(_proceso_encontrado == NULL) { 
            cont++;
        } else { 
            flag = true;
            _elemento_pivote->tiempo_ultimo_bloqueo = _proceso_encontrado->tiempo;  
            return _elemento_pivote;
        }

    } while(!flag && cont < tamanio_lista);

}

t_info_proceso_en_io* buscar_proceso_en_elemento(t_list* lista_procesos_de_io, uint8_t pid) {

    bool _es_el_proceso(void* elemento) {
        t_info_proceso_en_io* info_proc = (t_info_proceso_en_io*) elemento;
        return info_proc->pid == pid;
    }

    return list_find(lista_procesos_de_io, _es_el_proceso);
}
