#include "busqueda-de-structs.h"

t_cpu_conectada* _buscar_cpu_libre() { 

    bool _cpu_esta_libre(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->pid_en_cpu == -1;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_esta_libre);
    
}

t_cpu_conectada* _buscar_cpu_en_lista(uint8_t id_cpu)
{
    bool _cpu_tiene_id(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->id_cpu == id_cpu;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_tiene_id);
}

t_cpu_conectada* buscar_cpu_que_usa_proceso(t_list* cpus, uint8_t pid) 
{
    bool _es_el_proceso(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->pid_en_cpu == pid;
    }
    return list_find(cpus, _es_el_proceso);
}

t_info_proceso_en_io* buscar_proceso_en_io(t_list* lista_procesos, uint8_t pid) 
{
    bool _esta_el_proceso(void* ptr) {
        t_info_proceso_en_io* info_proceso = (t_info_proceso_en_io*) ptr;
        return info_proceso->pid == pid;
    }

    return list_find(lista_procesos, _esta_el_proceso);
}


t_cpu_conectada* _buscar_proceso_en_lista_cpu(uint8_t pid)
{
    bool _cpu_tiene_pid(void* ptr) {
        t_cpu_conectada* cpu_con_proceso = (t_cpu_conectada*) ptr;
        return cpu_con_proceso->pid_en_cpu == pid;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_tiene_pid);
}

t_io* buscar_io(t_list* lista_de_io, char* nombre_io) {
    bool _es_el_io(void* elemento) {
        t_io* io = (t_io*) elemento;
        return string_contains(io->nombre, nombre_io);
    }

    return list_find(lista_de_io, _es_el_io);
}

t_instancia_io* buscar_instancia_disponible(t_list* lista_de_instancias) {
    bool _esta_libre(void* elemento) {
        t_instancia_io* io = (t_instancia_io*) elemento;
        return io->pid == -1; 
    }
    t_instancia_io* instancia_libre = list_find(lista_de_instancias, _esta_libre);
    return instancia_libre;
}

t_instancia_io* devolver_instancia_disponible(char* nombre_interfaz) { 
    t_io* tipo_de_io = buscar_io(lista_de_io->lista_ios, nombre_interfaz);
    
    if(tipo_de_io != NULL){
        return buscar_instancia_disponible(tipo_de_io->instancias);
    }

    return NULL;
}

t_pcb* buscar_proceso_en_cola_exit(t_list* cola_exit, uint8_t pid) 
{
	bool _tiene_el_pid(void* ptr) {
		t_pcb* proceso = (t_pcb*) ptr;
		return proceso->pid == pid;
	}
	return list_find(cola_exit, _tiene_el_pid);
}

// funcion que busca la io donde se encuentra el pid
t_io* buscar_io_en_lista(t_list* lista_base, uint8_t pid) {
    bool flag = false;
    int tamanio_lista = list_size(lista_base), cont = 0;
    
    do {
        
        t_io* _elemento_pivote = list_get(lista_de_io->lista_ios, cont);
        
        t_info_proceso_en_io* _proceso_encontrado = buscar_proceso_en_elemento(_elemento_pivote->procesos, pid);

        if(_proceso_encontrado == NULL) { 
            cont++;
        } else { 
            flag = true; // Creo que esto es al pedo si haces un return. En todo caso sacás el return y que el return fuera del while te de _proceso_encontrado
            return _elemento_pivote;
        }

    } while(!flag && cont <= tamanio_lista);
    
    return NULL;
}

t_info_proceso_en_io* buscar_proceso_en_elemento(t_list* lista_procesos_de_io, uint8_t pid) {

    bool _es_el_proceso(void* elemento) {
        t_info_proceso_en_io* info_proc = (t_info_proceso_en_io*) elemento;
        return info_proc->pid == pid;
    }

    return list_find(lista_procesos_de_io, _es_el_proceso);
}