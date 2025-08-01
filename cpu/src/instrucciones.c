
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
            return mapa_instrucciones[i].tipo;
        }
    }
    return CANT_INSTRUCCIONES; 
}
t_instruccion* parse_noop(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_NOOP;
    return instr;
}


t_instruccion* parse_read(char* linea) {

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

   
    string_array_destroy(partes);

    return instr;  
}


t_instruccion* parse_write(char* linea) {

    // Dividir la línea en partes
    char** partes = string_split(linea, " ");  
    

    // Crear la instrucción
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_WRITE;
    instr->parametros.write.direccion = string_duplicate(partes[1]);
    instr->parametros.write.datos = string_duplicate(partes[2]);

  
    // Liberar memoria de las partes
    string_array_destroy(partes);

    return instr; 
}
t_instruccion* parse_goto(char* linea) {

    char** partes = string_split(linea, " ");  

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_GOTO;
    instr->parametros.go_to.valor = atoi(partes[1]);

   

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

   
    string_array_destroy(partes);
    return instr;       
}
t_instruccion* parse_init_proc(char* linea) {

    char** partes = string_split(linea, " ");  

    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_INIT_PROC;
    instr->parametros.init_proc.archivo = string_duplicate(partes[1]);
    instr->parametros.init_proc.tamanio = atoi(partes[2]);

  
    string_array_destroy(partes);

    return instr; 
}
t_instruccion* parse_dump_memory(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_DUMP_MEMORY; 

  
    return instr;   
}
t_instruccion* parse_exit(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    instr->tipo = INSTR_EXIT;    
  

    return instr;
}

t_instruccion* decode(char* linea) {

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
    int indice_pagina_cache, bytes, direccion_logica;
    t_pre_direccion_fisica pre_direccion_fisica;
    uint32_t direccion_fisica_final, marco_correspondiente;
    t_paquete* paquete = crear_paquete_con_codigo(PAQUETE);
    

    switch(instruccion->tipo) {
		case INSTR_NOOP:
        log_info(logger_cpu, "##PID <%d> | Ejecutando: <%s>", 
            pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo));            
            pcb_actual->pc++;
            
            sem_post(&sem_cpu); 

            eliminar_paquete(paquete);
			break;
		case INSTR_WRITE:

            //Prepara los datos para calculos de direcciones
            direccion_logica = atoi(instruccion->parametros.write.direccion);
            pre_direccion_fisica = calcular_pre_direccion_fisica(direccion_logica);

            //CACHE
            if(cache_esta_activada()){
                indice_pagina_cache = buscar_pagina_en_cache(memoria_cache, pre_direccion_fisica.nro_pagina);
                //Si NO encuentra La pagina en cache, la carga desde memoria
                if(indice_pagina_cache == -1){     
                    indice_pagina_cache = manejar_cache_miss(pre_direccion_fisica);           
                }
                //RETARDO DE CACHE:
                usleep(memoria_cache->retardo * 1000);
                escribir_en_cache(indice_pagina_cache, pre_direccion_fisica.desplazamiento, instruccion->parametros.write.datos);
                log_debug(logger_cpu, "Contenido de página: %.10s", memoria_cache->paginas[indice_pagina_cache].contenido);

                pcb_actual->pc++;
                sem_post(&sem_cpu); // Libera el semáforo de CPU
                eliminar_paquete(paquete);

            }
            //Si la cache no esta activada
            else{
                //TLB
                if(tlb_esta_activada()){
                //Verifica si la página está en TLB
                marco_correspondiente = esta_en_tlb(pre_direccion_fisica.nro_pagina);

                if (marco_correspondiente == -1){
                    //TLB_MISS, accede a tabla de paginas en memoria para obtener marco
                    marco_correspondiente = tlb_miss(pre_direccion_fisica);
                }
                //TLB_HIT    
                }
                else marco_correspondiente = solicitar_marco_a_memoria(pre_direccion_fisica, pcb_actual->pid);
                //Calcula direccion fisica
                direccion_fisica_final = calcular_direccion_fisica_final(marco_correspondiente, pre_direccion_fisica);
                mmu->ultima_direccion_fisica_calculada = direccion_fisica_final;
                
                //Envia el WRITE a memoria
                enviar_write_a_memoria(pcb_actual->pid, direccion_fisica_final, instruccion->parametros.write.datos, strlen(instruccion->parametros.write.datos)+1);
                log_debug(logger_cpu, "WRITE enviado a Memoria. Esperando respuesta...");
                eliminar_paquete(paquete);
                
                if (ultima_escritura) 
                    free(ultima_escritura);
                ultima_escritura = strdup(instruccion->parametros.write.datos);

               sem_post(&sem_memoria); // Espera a que la memoria confirme el read
            }
            free(pre_direccion_fisica.entrada_nivel);
			break;

		case INSTR_READ:
            //Calculos de predireccion
            direccion_logica = atoi(instruccion->parametros.read.direccion);
            pre_direccion_fisica = calcular_pre_direccion_fisica(direccion_logica);  

            //CACHE
            if(cache_esta_activada()){
                indice_pagina_cache = buscar_pagina_en_cache(memoria_cache, pre_direccion_fisica.nro_pagina);
                //Si NO encuentra La pagina en cache, la carga desde memoria
                if(indice_pagina_cache == -1){     
                    indice_pagina_cache = manejar_cache_miss(pre_direccion_fisica);           
                }
                //RETARDO DE CACHE:
                usleep(memoria_cache->retardo * 1000);
                char* datos_leidos = leer_de_cache(indice_pagina_cache, pre_direccion_fisica.desplazamiento, instruccion->parametros.read.tamanio);
                log_info(logger_cpu, "<CACHE> PID: <%d> - Accion: <LEER> - Valor : <%s>", pcb_actual->pid, datos_leidos);
                free(datos_leidos);
                pcb_actual->pc++;
                sem_post(&sem_cpu); // Libera el semáforo de CPU
                eliminar_paquete(paquete);

            }
            //Si la cache no esta activada
            else{
                //TLB
                if(tlb_esta_activada()){
                //Verifica si la página está en TLB
                marco_correspondiente = esta_en_tlb(pre_direccion_fisica.nro_pagina);

                if (marco_correspondiente == -1){
                    //TLB_MISS, accede a tabla de paginas en memoria para obtener marco
                    marco_correspondiente = tlb_miss(pre_direccion_fisica);
                }
                //TLB_HIT    
                }
                else marco_correspondiente = solicitar_marco_a_memoria(pre_direccion_fisica, pcb_actual->pid);
                //Calcula direccion fisica
                direccion_fisica_final = calcular_direccion_fisica_final(marco_correspondiente, pre_direccion_fisica);
                mmu->ultima_direccion_fisica_calculada = direccion_fisica_final;
                
                //Envia el read a memoria
                enviar_read_a_memoria(pcb_actual->pid, direccion_fisica_final, instruccion->parametros.read.tamanio);
                eliminar_paquete(paquete);
                
                sem_post(&sem_memoria); // Espera a que la memoria confirme el read
               
            }
            free(pre_direccion_fisica.entrada_nivel);
			break;
        
		case INSTR_GOTO:
            log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s> - <%d>",
            pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo), instruccion->parametros.go_to.valor);
            pcb_actual->pc = instruccion->parametros.go_to.valor;
            //pcb_actual->pc++;
            sem_post(&sem_cpu); 
        //    pedir_instruccion_a_memoria(pcb_actual); 
            eliminar_paquete(paquete);

			break;

			////////////////////////////
			/////////SYSCALLS///////////
			////////////////////////////
                        
		case INSTR_IO:
			log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s> - <%s> <%d>", pcb_actual->pid, obtener_nombre_instruccion(instruccion->tipo),
			instruccion->parametros.io.dispositivo, instruccion->parametros.io.tiempo);

            char* dispositivo = instruccion->parametros.io.dispositivo;
            int tiempo = instruccion->parametros.io.tiempo;

            pcb_actual->pc++;
            paquete->codigo_operacion = SYSCALL_IO;
            
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(uint8_t));

            agregar_a_paquete(paquete, &(pcb_actual->pc), sizeof(uint16_t));

            int len_dispositivo = strlen(dispositivo) + 1; 
            agregar_a_paquete(paquete, dispositivo, len_dispositivo);

            // Agregar tiempo (serializa el int)
            agregar_a_paquete(paquete, &tiempo, sizeof(int32_t));

            // Serializar y enviar
            bytes = sizeof(int) + sizeof(uint32_t) + paquete->buffer->size;
            void* paquete_io = serializar_paquete(paquete, bytes);

            send(fd_conexion_kernel_interrupt, paquete_io, bytes, 0);
            eliminar_paquete(paquete);

            free(paquete_io);

            log_debug(logger_cpu, "Enviando SYSCALL_IO a Kernel");
            //sem_post(&sem_cpu_kernel);
            


			break;
		case INSTR_INIT_PROC:
			log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s> - <%s> <%d>", pcb_actual->pid, 
                obtener_nombre_instruccion(instruccion->tipo),
			instruccion->parametros.init_proc.archivo, instruccion->parametros.init_proc.tamanio);
            char* archivo = instruccion->parametros.init_proc.archivo;
            uint32_t tamanio = instruccion->parametros.init_proc.tamanio;
          
            paquete->codigo_operacion = SYSCALL_INIT_PROC;
            
            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(uint8_t));

            uint32_t len_archivo = strlen(archivo) + 1; 
            agregar_a_paquete(paquete, archivo, len_archivo);

            // Agregar tamanio (serializa el int)
            agregar_a_paquete(paquete, &tamanio, sizeof(uint32_t));

            // Serializar y enviar
            bytes = sizeof(uint32_t) + sizeof(uint32_t) + paquete->buffer->size;
            void* paquete_init_proc = serializar_paquete(paquete, bytes);

            send(fd_conexion_kernel_interrupt, paquete_init_proc, bytes, 0);
            eliminar_paquete(paquete);

            free(paquete_init_proc);

            
            pcb_actual->pc++;
           int valor =0;
            sem_post(&sem_cpu); 
            sem_getvalue(&sem_cpu, &valor);
            log_trace(logger_cpu, "wtf innit proc valor semaforo: %d", valor);
			break;
		case INSTR_DUMP_MEMORY:

			log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s>", pcb_actual->pid, 
                obtener_nombre_instruccion(instruccion->tipo));

            pcb_actual->pc++;

            paquete->codigo_operacion = SYSCALL_DUMP_MEMORY;

            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(uint8_t));

            agregar_a_paquete(paquete, &(pcb_actual->pc), sizeof(uint16_t));

            bytes = sizeof(uint32_t)+ sizeof(uint32_t) + paquete->buffer->size;
            void* paquete_dump_memory = serializar_paquete(paquete, bytes);
            
            send(fd_conexion_kernel_interrupt, paquete_dump_memory, bytes, 0);

            eliminar_paquete(paquete);

            free(paquete_dump_memory);
            
            log_debug(logger_cpu, "Enviando SYSCALL_DUMP_MEMORY a Kernel");

           // sem_post(&sem_cpu_kernel);

            // pedir_instruccion_a_memoria(pcb_actual); 
			break;
		case INSTR_EXIT:

			log_info(logger_cpu, "##PID: <%d> | Ejecutando: <%s>", pcb_actual->pid, 
                obtener_nombre_instruccion(instruccion->tipo));

            
            paquete->codigo_operacion = SYSCALL_EXIT;

            agregar_a_paquete(paquete, &(pcb_actual->pid), sizeof(uint8_t));

            agregar_a_paquete(paquete, &(pcb_actual->pc), sizeof(uint16_t));

            bytes = sizeof(uint32_t)+ sizeof(uint32_t) + paquete->buffer->size;
            void* paquete_exit = serializar_paquete(paquete, bytes);
            
            send(fd_conexion_kernel_interrupt, paquete_exit, bytes, 0);

            eliminar_paquete(paquete);
            free(paquete_exit);

            log_debug(logger_cpu, "Enviando SYSCALL_EXIT a Kernel");
            flag_exit = true;
            if(cache_esta_activada())
                actualizar_memoria_principal_completa();
            if(tlb_esta_activada())
				limpiar_tlb();
          //  sem_post(&sem_cpu_kernel);
			break;
		default:
			log_error(logger_cpu, "Instrucción desconocida: %d", instruccion->tipo); 
			break;
	}

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
