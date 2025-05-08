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
    char* linea_trimmed = string_duplicate(linea); 
    string_trim(&linea_trimmed); 
    log_debug(logger_cpu, "Instrucción READ. LINEA: %s", linea_trimmed);

    char** partes = string_split(linea_trimmed, " ");  
    if (partes[0] == NULL || partes[1] == NULL || partes[2] == NULL) {
        log_error(logger_cpu, "Error: la línea no tiene el formato esperado.");
        string_array_destroy(partes);
        free(linea_trimmed); 
        return NULL;
    }

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_READ;
    instr->parametros.read.direccion = string_duplicate(partes[1]);
    instr->parametros.read.tamanio = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción READ creada correctamente: archivo=%s, tamaño=%d",
              instr->parametros.read.direccion, instr->parametros.read.tamanio);

    string_array_destroy(partes);
    free(linea_trimmed); 

    return instr;  
}
t_instruccion* parse_write(char* linea) {
    char* linea_trimmed = string_duplicate(linea); 
    string_trim(&linea_trimmed); 
    log_debug(logger_cpu, "Instrucción WRITE. LINEA: %s", linea_trimmed);

    char** partes = string_split(linea_trimmed, " ");  
    if (partes[0] == NULL || partes[1] == NULL || partes[2] == NULL) {
        log_error(logger_cpu, "Error: la línea no tiene el formato esperado.");
        string_array_destroy(partes);
        free(linea_trimmed); 
        return NULL;
    }

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_WRITE;
    instr->parametros.write.datos = string_duplicate(partes[1]);
    instr->parametros.write.direccion = string_duplicate(partes[2]);

    log_debug(logger_cpu, "Instrucción WRITE creada correctamente: archivo=%s, tamaño=%s",
              instr->parametros.write.datos, instr->parametros.write.direccion);

    string_array_destroy(partes);
    free(linea_trimmed); 

    return instr; 
}
t_instruccion* parse_goto(char* linea) {
    char* linea_trimmed = string_duplicate(linea); 

    string_trim(&linea_trimmed); 
    char** partes = string_split(linea_trimmed, " ");  
    
    if (partes[1] == NULL) return NULL;


    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_GOTO;
    instr->parametros.go_to.valor = atoi(partes[1]);

    log_debug(logger_cpu, "Instrucción GOTO creada correctamente: valor=%d",
              instr->parametros.go_to.valor);

    string_array_destroy(partes);
    free(linea_trimmed); 
    return instr;        
}
t_instruccion* parse_io(char* linea) {
    char* linea_trimmed = string_duplicate(linea); 
    string_trim(&linea_trimmed); 
    char** partes = string_split(linea_trimmed, " "); 
    
    if (partes[1] == NULL) return NULL;


    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_IO;
    instr->parametros.io.dispositivo = string_duplicate(partes[1]);
    instr->parametros.io.tiempo = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción IO creada correctamente: dispositivo=%s, tiempo=%d",
              instr->parametros.io.dispositivo, instr->parametros.io.tiempo);

    string_array_destroy(partes);
    free(linea_trimmed); 
    return instr;       
}
t_instruccion* parse_init_proc(char* linea) {
    char* linea_trimmed = string_duplicate(linea); 
    string_trim(&linea_trimmed); 
    log_debug(logger_cpu, "Instrucción INIT_PROC. LINEA: %s", linea_trimmed);

    char** partes = string_split(linea_trimmed, " ");  
    if (partes[0] == NULL || partes[1] == NULL || partes[2] == NULL) {
        log_error(logger_cpu, "Error: la línea no tiene el formato esperado.");
        string_array_destroy(partes);
        free(linea_trimmed); 
        return NULL;
    }

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_INIT_PROC;
    instr->parametros.init_proc.archivo = string_duplicate(partes[1]);
    instr->parametros.init_proc.tamanio = atoi(partes[2]);

    log_debug(logger_cpu, "Instrucción INIT_PROC creada correctamente: archivo=%s, tamaño=%d",
              instr->parametros.init_proc.archivo, instr->parametros.init_proc.tamanio);

    string_array_destroy(partes);
    free(linea_trimmed); 

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
    char* linea_trimmed = string_duplicate(linea); 
    string_trim(&linea_trimmed); 
    log_debug(logger_cpu, "Decodificando instrucción: %s", linea_trimmed);

    char** partes = string_split(linea_trimmed, " ");  
    if (partes[0] == NULL) {
        log_error(logger_cpu, "Error: la instrucción no tiene tipo");
        string_array_destroy(partes);
        free(linea_trimmed); 
        return NULL;
    }

    instruccion_t tipo = obtener_tipo(partes[0]);
    log_debug(logger_cpu, "Tipo de instrucción: %d", tipo);
    string_array_destroy(partes);
    free(linea_trimmed); 

    t_instruccion* instruccion = parse_funciones[tipo](linea);
    if (instruccion == NULL) {
        log_error(logger_cpu, "Error: no se pudo decodificar la instrucción");
        return NULL;
    }

    log_debug(logger_cpu, "Instrucción decodificada correctamente: %d", instruccion->tipo);
    return instruccion;
}

void ejecutar_instruccion(t_instruccion* instruccion, int socket_memoria){
    switch(instruccion->tipo) {
		case INSTR_NOOP:
			log_info(logger_cpu, "Ejecutando instrucción NOOP con parametros");
			break;
		case INSTR_WRITE:
                //char* direccion = instruccion->parametros.write.direccion;
                //char* datos = instruccion->parametros.write.datos;

            log_info(logger_cpu, "Ejecutando instrucción WRITE con parametros %s %s",
				instruccion->parametros.write.datos, instruccion->parametros.write.direccion);
                //t_paquete* paquete = crear_paquete(); // Averiguar si modificar crear_paquete() para que tome un OP_CODE
                //agregar_a_paquete(paquete, direccion, strlen(direccion) + 1);
                //agregar_a_paquete(paquete, datos, strlen(datos) + 1);
                //enviar_paquete(paquete, socket_memoria);
                //eliminar_paquete(paquete);

                //log_info(logger_cpu, "WRITE enviado a Memoria.");
			break;
		case INSTR_READ:
			log_info(logger_cpu, "Ejecutando instrucción READ con parametros %s %d",
				instruccion->parametros.read.direccion, instruccion->parametros.read.tamanio);
			break;
		case INSTR_GOTO:
			log_info(logger_cpu, "Ejecutando instrucción GOTO con parametros %d",
				instruccion->parametros.go_to.valor);
			break;

			////////////////////////////
			/////////SYSCALLS///////////
			////////////////////////////
		
		case INSTR_IO:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.io.dispositivo, instruccion->parametros.io.tiempo);
			break;
		case INSTR_INIT_PROC:
			log_info(logger_cpu, "syscall detectada... parametros %s %d",
				instruccion->parametros.init_proc.archivo, instruccion->parametros.init_proc.tamanio);
			break;
		case INSTR_DUMP_MEMORY:
			log_info(logger_cpu, "syscall detectada... parametros ");
			break;
		case INSTR_EXIT:
			log_info(logger_cpu, "syscall detectada... parametros ");
			break;
		default:
			log_error(logger_cpu, "Instrucción desconocida: %d", instruccion->tipo);
			break;
	}
}

