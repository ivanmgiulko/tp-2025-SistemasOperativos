
#include "instrucciones.h"

instruccion_map_entry mapa_instrucciones[] = {
    {"NOOP", INSTR_NOOP},
    {"WRITE", INSTR_WRITE},
    {"READ", INSTR_READ},
    {"GOTO", INSTR_GOTO},
    {"IO", INSTR_IO},
    {"INIT_PROC", INSTR_INIT_PROC},
    {"DUMP_MEMORY", INSTR_DUMP_MEMORY},
    {"EXIT", INSTR_EXIT},
    {NULL, 0}  
};

func_parse_t parse_funciones[CANT_INSTRUCCIONES] = {
    [INSTR_NOOP]        = parse_noop,
    [INSTR_WRITE]       = parse_write,
    [INSTR_READ]        = parse_read,
    [INSTR_GOTO]        = parse_goto,
    [INSTR_IO]          = parse_io,
    [INSTR_INIT_PROC]   = parse_init_proc,
    [INSTR_DUMP_MEMORY] = parse_dump_memory,
    [INSTR_EXIT]        = parse_exit
};
char* obtener_nombre_instruccion(instruccion_t tipo) {
    for (int i = 0; mapa_instrucciones[i].nombre != NULL; i++) {
        if (mapa_instrucciones[i].tipo == tipo) {
            return mapa_instrucciones[i].nombre;
        }
    }
    return "NULL"; // Si no se encuentra el tipo
}
instruccion_t obtener_tipo(char* nombre_instr) {
    for (int i = 0; mapa_instrucciones[i].nombre != NULL; i++) {
        if (string_equals_ignore_case(mapa_instrucciones[i].nombre, nombre_instr)) {
            log_debug(logger_cpu, "Instrucción encontrada: %s", mapa_instrucciones[i].nombre);
            return mapa_instrucciones[i].tipo;
        }
    }
    return CANT_INSTRUCCIONES; 
}
t_instruccion* parse_noop(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    log_debug(logger_cpu, "Instrucción NOOP");
    instr->tipo = INSTR_NOOP;
    log_info(logger_cpu, "ADENTRO DEL PARSER NOOP A VER EL TIPO: %d", instr->tipo);
    return instr;
}


t_instruccion* parse_read(char* linea) {
    log_debug(logger_cpu, "Instrucción READ. LINEA: %s", linea);

    char** partes = string_split(linea, " ");  
    if (partes[0] == NULL || partes[1] == NULL || partes[2] == NULL) {
        log_error(logger_cpu, "Error: la línea no tiene el formato esperado.");
        string_array_destroy(partes);
        return NULL;
    }

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_READ;
    instr->parametros.read.direccion = string_duplicate(partes[1]);
    instr->parametros.read.tamanio = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción READ creada correctamente: archivo=%s, tamaño=%d",
              instr->parametros.read.direccion, instr->parametros.read.tamanio);

    string_array_destroy(partes);

    return instr;  
}
// SUPUESTAMENTE ESTA FUNCION ES MEJOR. TODAVIA NO LA PROBAMOS. COPAILOT CREE EN LEAN, YO (IVAN) NO CREO EN EL.


t_instruccion* parse_write(char* linea) {
    log_debug(logger_cpu, "Instrucción WRITE. LINEA: %s", linea);

    // Dividir la línea en partes
    char** partes = string_split(linea, " ");  
    

    // Crear la instrucción
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_WRITE;
    instr->parametros.write.datos = string_duplicate(partes[1]);
    instr->parametros.write.direccion = string_duplicate(partes[2]);

    log_debug(logger_cpu, "Instrucción WRITE creada correctamente: datos=%s, dirección=%s",
              instr->parametros.write.datos, instr->parametros.write.direccion);

    // Liberar memoria de las partes
    string_array_destroy(partes);

    return instr; 
}
t_instruccion* parse_goto(char* linea) {

    char** partes = string_split(linea, " ");  

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_GOTO;
    instr->parametros.go_to.valor = atoi(partes[1]);

    log_debug(logger_cpu, "Instrucción GOTO creada correctamente: valor=%d",
              instr->parametros.go_to.valor);

    string_array_destroy(partes);
    return instr;        
}
t_instruccion* parse_io(char* linea) {
    char** partes = string_split(linea, " "); 
    
    if (partes[1] == NULL) return NULL;


    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_IO;
    instr->parametros.io.dispositivo = string_duplicate(partes[1]);
    instr->parametros.io.tiempo = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción IO creada correctamente: dispositivo=%s, tiempo=%d",
              instr->parametros.io.dispositivo, instr->parametros.io.tiempo);

    string_array_destroy(partes);
    return instr;       
}
t_instruccion* parse_init_proc(char* linea) {
    log_debug(logger_cpu, "Instrucción INIT_PROC. LINEA: %s", linea);

    char** partes = string_split(linea, " ");  

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_INIT_PROC;
    instr->parametros.init_proc.archivo = string_duplicate(partes[1]);
    instr->parametros.init_proc.tamanio = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción INIT_PROC creada correctamente: archivo=%s, tamaño=%d",
              instr->parametros.init_proc.archivo, instr->parametros.init_proc.tamanio);

    string_array_destroy(partes);

    return instr; 
}
t_instruccion* parse_dump_memory(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_DUMP_MEMORY; 

    log_debug(logger_cpu, "Instrucción DUMP_MEMORY creada correctamente: %d", instr->tipo);
    return instr;   
}
t_instruccion* parse_exit(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_EXIT;    
    log_debug(logger_cpu, "Instrucción EXIT creada correctamente: %d", instr->tipo);

    return instr;
}

t_instruccion* decode(char* linea) {
    log_debug(logger_cpu, "Decodificando instrucción: %s", linea);

    // Dividir la línea en partes
    char** partes = string_split(linea, " ");  
    if (partes[0] == NULL) {
        log_error(logger_cpu, "Error: la instrucción no tiene tipo");
        string_array_destroy(partes);
        return NULL;
    }

    // Obtener el tipo de instrucción
    instruccion_t tipo = obtener_tipo(partes[0]);
    string_array_destroy(partes);

    // Llamar a la función de parseo correspondiente
    t_instruccion* instruccion = parse_funciones[tipo](linea);
    if (instruccion == NULL) {
        log_error(logger_cpu, "Error: no se pudo decodificar la instrucción");
        return NULL;
    }

    return instruccion;
}

void ejecutar_instruccion(t_instruccion* instruccion) {
    int bytes;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete);
    switch(instruccion->tipo) {
		case INSTR_NOOP:
        log_info(logger_cpu, "##PID <%d> | Ejecutando: <%s>", 
            pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo));            
            pcb_actual->pc++;
			break;
		case INSTR_WRITE:
                //char* direccion = instruccion->parametros.write.direccion;
                //char* datos = instruccion->parametros.write.datos;

            log_info(logger_cpu, "##PID <%d> - Ejecutando: <%s> - <%s> <%s>", 
                pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo),
                instruccion->parametros.write.datos, instruccion->parametros.write.direccion);


                //t_paquete* paquete = crear_paquete(); // Averiguar si modificar crear_paquete() para que tome un OP_CODE
                //agregar_a_paquete(paquete, direccion, strlen(direccion) + 1);
                //agregar_a_paquete(paquete, datos, strlen(datos) + 1);
                //enviar_paquete(paquete, fd_conexion_memoria) {;
                //eliminar_paquete(paquete);
                pcb_actual->pc++;

                //log_info(logger_cpu, "WRITE enviado a Memoria.");
			break;
		case INSTR_READ:
            log_info(logger_cpu, "##PID <%d> | Ejecutando: <%s> con parametros %s %d",
            pcb_actual->pid,obtener_nombre_instruccion(instruccion->tipo),
            instruccion->parametros.read.direccion, instruccion->parametros.read.tamanio);
            pcb_actual->pc++;

			break;
		case INSTR_GOTO:
            log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s> con parametros %d",
            pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo), instruccion->parametros.go_to.valor);
              //  pcb_actual->pc = instruccion->parametros.go_to.valor;
                pcb_actual->pc++;
			break;

			////////////////////////////
			/////////SYSCALLS///////////
			////////////////////////////
		
		case INSTR_IO:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.io.dispositivo, instruccion->parametros.io.tiempo);
            char* dispositivo = instruccion->parametros.io.dispositivo;
            int tiempo = instruccion->parametros.io.tiempo;
          
            paquete->codigo_operacion = SYSCALL_IO;
            
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(int));

            int len_dispositivo = strlen(dispositivo) + 1; 
            agregar_a_paquete(paquete, dispositivo, len_dispositivo);

            // Agregar tiempo (serializa el int)
            agregar_a_paquete(paquete, &tiempo, sizeof(int));

            // Serializar y enviar
            bytes = sizeof(int) + sizeof(int) + paquete->buffer->size;
            void* paquete_io = serializar_paquete(paquete, bytes);

            send(fd_conexion_kernel_interrupt, paquete_io, bytes, 0);
            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete_io);
            free(paquete);

            log_info(logger_cpu, "Enviando SYSCALL_IO a Kernel");

            pcb_actual->pc++;

			break;
		case INSTR_INIT_PROC:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.init_proc.archivo, instruccion->parametros.init_proc.tamanio);
            char* archivo = instruccion->parametros.init_proc.archivo;
            int tamanio = instruccion->parametros.init_proc.tamanio;
          
            paquete->codigo_operacion = SYSCALL_INIT_PROC;
            
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(int));

            int len_archivo = strlen(archivo) + 1; 
            agregar_a_paquete(paquete, archivo, len_archivo);

            // Agregar tamanio (serializa el int)
            agregar_a_paquete(paquete, &tamanio, sizeof(int));

            // Serializar y enviar
            bytes = sizeof(int) + sizeof(int) + paquete->buffer->size;
            void* paquete_init_proc = serializar_paquete(paquete, bytes);

            send(fd_conexion_kernel_interrupt, paquete_init_proc, bytes, 0);
            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete_init_proc);
            free(paquete);

            log_info(logger_cpu, "Enviando SYSCALL_INIT_PROC a Kernel");

                pcb_actual->pc++;
			break;
		case INSTR_DUMP_MEMORY:
			log_info(logger_cpu, "syscall detectada... parametros ");

            

            paquete->codigo_operacion = SYSCALL_DUMP_MEMORY;
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(int));
            bytes = sizeof(int)+ sizeof(int) + paquete->buffer->size;
            void* paquete_dump_memory = serializar_paquete(paquete, bytes);
            
            send(fd_conexion_kernel_interrupt, paquete_dump_memory, bytes, 0);

            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete_dump_memory);
            free(paquete);

            log_info(logger_cpu, "Enviando SYSCALL_DUMP_MEMORY a Kernel");

            pcb_actual->pc++;
			break;
		case INSTR_EXIT:

			log_info(logger_cpu, "syscall detectada... parametros ");


            paquete->codigo_operacion = SYSCALL_EXIT;
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(int));
            bytes = sizeof(int)+ sizeof(int) + paquete->buffer->size;
            void* paquete_exit = serializar_paquete(paquete, bytes);
            
            send(fd_conexion_kernel_interrupt, paquete_exit, bytes, 0);

            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete_exit);
            free(paquete);

            log_info(logger_cpu, "Enviando SYSCALL_EXIT a Kernel");
            pcb_actual->pc++;
			break;
		default:
			log_error(logger_cpu, "Instrucción desconocida: %d", instruccion->tipo); 
			break;
	}
    sem_post(&sem_cpu);
    log_trace(logger_cpu, "PID: %d | PC: %d", pcb_actual->pid, pcb_actual->pc);
    pedir_instruccion_a_memoria(pcb_actual); 
}

void free_instruccion(t_instruccion* instruccion) {
    if (!instruccion) return;

    switch (instruccion->tipo) {
        case INSTR_WRITE:
            free(instruccion->parametros.write.direccion);
            free(instruccion->parametros.write.datos);
            break;

        case INSTR_READ:
            free(instruccion->parametros.read.direccion);
            break;

        case INSTR_IO:
            free(instruccion->parametros.io.dispositivo);
            break;

        case INSTR_INIT_PROC:
            free(instruccion->parametros.init_proc.archivo);
            break;

        case INSTR_NOOP:
        case INSTR_GOTO:
        case INSTR_DUMP_MEMORY:
        case INSTR_EXIT:
            // No hay memoria dinámica en estos casos
            break;

        default:
            log_error(logger_cpu, "Tipo de instrucción desconocido: %d", instruccion->tipo);
            break;
    }

    free(instruccion); // Finalmente, libera la estructura principal
}
