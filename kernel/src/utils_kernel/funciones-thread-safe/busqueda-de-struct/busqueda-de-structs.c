#include "busqueda-de-structs.h"

t_cpu_conectada* _buscar_cpu_libre() { 

    bool _cpu_esta_libre(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->pid_en_cpu == -1;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_esta_libre);

}

t_info_proceso_en_io* buscar_proceso_en_io(t_list* lista_procesos, uint8_t pid) 
{
    bool _esta_el_proceso(void* ptr) {
        t_info_proceso_en_io* info_proceso = (t_info_proceso_en_io*) ptr;
        return info_proceso->pid == pid;
    }

    return list_find(lista_procesos, _esta_el_proceso);
}

t_cpu_conectada* _buscar_cpu_en_lista(uint8_t id_cpu)
{
    bool _cpu_tiene_id(void* ptr) {
        t_cpu_conectada* cpu = (t_cpu_conectada*) ptr;
        return cpu->id_cpu == id_cpu;
    }

    return list_find(lista_cpus->lista_cpus, _cpu_tiene_id);
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

t_io* funcion_syscall_IO(char* nombreInterfaz) { 
    return buscar_io(lista_de_io->lista_ios, nombreInterfaz);
}

t_pcb* buscar_proceso_en_cola_exit(t_list* cola_exit, uint8_t pid) 
{
	bool _tiene_el_pid(void* ptr) {
		t_pcb* proceso = (t_pcb*) ptr;
		return proceso->pid == pid;
	}
	return list_find(cola_exit, _tiene_el_pid);
}