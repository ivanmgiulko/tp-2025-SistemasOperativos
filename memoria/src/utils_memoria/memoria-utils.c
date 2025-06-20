#include "memoria-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
t_memoria_del_sistema crear_memoria_del_sistema() {
    t_memoria_del_sistema memoria;
    pthread_mutex_init(&memoria.mutex, NULL); 
    memoria.procesos = malloc(sizeof(t_proceso_en_memoria)); 
    memoria.cant_procesos = 0;
    memoria.tam_memoria = atoi(config_memoria->TAM_MEMORIA);
    memoria.tam_pagina = atoi(config_memoria->TAM_PAGINA);
    memoria.cant_marcos = memoria.tam_memoria / memoria.tam_pagina;

    memoria.memoria_principal = malloc(memoria.tam_memoria);
    if (memoria.memoria_principal == NULL) {
        log_error(logger_memoria, "Error al reservar la memoria principal");
        exit(EXIT_FAILURE);
    }

    memoria.bitmap_marcos_ocupados = calloc(memoria.cant_marcos, sizeof(bool));
    if (memoria.bitmap_marcos_ocupados == NULL) {
        log_error(logger_memoria, "Error al instanciar el bitmap de frames");
        exit(EXIT_FAILURE);
    }

    log_info(logger_memoria, "Memoria instanciada - Tamaño: %d bytes, Marcos: %d marcos de %d bytes", memoria.tam_memoria, memoria.cant_marcos, memoria.tam_pagina);

    return memoria;
}

//hecho por copialot
// char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
//     if (pathArchivoPseudocodigo == NULL || strlen(pathArchivoPseudocodigo) == 0) {
//         log_error(logger_memoria, "El path del archivo es inválido");
//         return NULL;
//     }

//     log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
//     FILE* archivo = fopen(pathArchivoPseudocodigo, "r");
//     if (!archivo) {
//         perror("Error abriendo archivo de pseudocodigo");
//         return NULL;
//     }

//     int capacidad = 10;
//     char** instrucciones = malloc(capacidad * sizeof(char*));
//     if (!instrucciones) {
//         fclose(archivo);
//         return NULL;
//     }

//     char buffer[MAX_LINEA];
//     int count = 0;

//     while (fgets(buffer, MAX_LINEA, archivo)) {
//         buffer[strcspn(buffer, "\n")] = 0;

//         if (count >= capacidad) {
//             char** temp = realloc(instrucciones, capacidad * 2 * sizeof(char*));
//             if (!temp) {
//                 log_error(logger_memoria, "Error al reasignar memoria para instrucciones");
//                 for (int i = 0; i < count; i++) {
//                     free(instrucciones[i]);
//                 }
//                 free(instrucciones);
//                 fclose(archivo);
//                 return NULL;
//             }
//             instrucciones = temp;
//             capacidad *= 2;
//         }

//         instrucciones[count] = strdup(buffer);
//         if (!instrucciones[count]) {
//             log_error(logger_memoria, "Error al duplicar la línea de instrucciones");
//             for (int i = 0; i < count; i++) {
//                 free(instrucciones[i]);
//             }
//             free(instrucciones);
//             fclose(archivo);
//             return NULL;
//         }
//         count++;
//     }

//     fclose(archivo);
//     *cantidad = count;
//     return instrucciones;
// }

// char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
//     if (pathArchivoPseudocodigo == NULL || string_is_empty(pathArchivoPseudocodigo)) {
//         log_error(logger_memoria, "El path del archivo es inválido o está vacío");
//         return NULL;
//     }

//     log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
//     FILE* archivo = fopen(pathArchivoPseudocodigo, "r");
//     if (!archivo) {
//         perror("Error abriendo archivo de pseudocodigo");
//         return NULL;
//     }

//     char** instrucciones = string_array_new(); // Crear un array de strings vacío
//     char buffer[MAX_LINEA];

//     while (fgets(buffer, MAX_LINEA, archivo)) {
//         string_array_push(&instrucciones, string_duplicate(buffer)); // Agregar la instrucción al array
//     }

//     fclose(archivo);
//     *cantidad = string_array_size(instrucciones); // Obtener la cantidad de instrucciones
//     return instrucciones;
// }

// void agregar_proceso(t_pcbMemoria* pcb) {
//     int cant_inst = 0;
//     char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    
//     if (!instrucciones) {
//         log_error(logger_memoria, "No se pudieron leer las instrucciones para el proceso PID %d", pcb->pid);
//         return;
//     }

//     t_proceso_en_memoria* temp = realloc(memoria_del_sistema->procesos, (memoria_del_sistema->cant_procesos + 1) * sizeof(t_proceso_en_memoria));
//     if (!temp) {
//         log_error(logger_memoria, "Error al reasignar memoria para procesos");
//         for (int i = 0; i < cant_inst; i++) {
//             free(instrucciones[i]);
//         }
//         free(instrucciones);
//         return;
//     }
//     memoria_del_sistema->procesos = temp;

//     t_proceso_en_memoria nuevoProceso;
//     nuevoProceso.pid = pcb->pid;
//     nuevoProceso.instrucciones = instrucciones;
//     nuevoProceso.cant_instrucciones = cant_inst;

//     memoria_del_sistema->procesos[memoria_del_sistema->cant_procesos] = nuevoProceso;
//     memoria_del_sistema->cant_procesos++;
// }

//hehco por nosotros

char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
   // pathArchivoPseudocodigo = "/home/utnso/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt";
    log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
    //esto lo use para probar algo ignorar /borrar cuando este todo ok (lean)
   
    FILE* archivo = fopen(pathArchivoPseudocodigo, "r");
    if (!archivo) {
        perror("Error abriendo archivo de pseudocodigo");
        return NULL;
    }

    int capacidad = 10;
    char** instrucciones = malloc(capacidad * sizeof(char*));
    if (!instrucciones) {
        fclose(archivo);
        return NULL;
    }

    char buffer[MAX_LINEA];
    int count = 0;

    while (fgets(buffer, MAX_LINEA, archivo)) {
        // Eliminar salto de línea si lo hay
        buffer[strcspn(buffer, "\n")] = 0;

        if (count >= capacidad) {
            capacidad *= 2;
            instrucciones = realloc(instrucciones, capacidad * sizeof(char*));
        }

        instrucciones[count] = strdup(buffer); // Copia segura
        count++;
    }

    fclose(archivo);
    *cantidad = count;
    return instrucciones;
}

void agregar_proceso(t_pcbMemoria* pcb) {
    int cant_inst = 0;
    log_debug(logger_memoria, "Path recibido en PCB: %s", pcb->pathArchivoPseudocodigo);

    pcb->metricas_proceso = iniciarMetricasProceso();
    char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    
    if (!instrucciones){
        log_error(logger_memoria, "Error al cargar instrucciones, falla al crear proceso");
        return;
    } 
    memoria_del_sistema->procesos = realloc(memoria_del_sistema->procesos, (memoria_del_sistema->cant_procesos + 1) * sizeof(t_proceso_en_memoria));

    int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);
    int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    int tam_pagina = atoi(config_memoria->TAM_PAGINA);

    t_proceso_en_memoria nuevoProceso;
    nuevoProceso.pid = pcb->pid;
    nuevoProceso.instrucciones = instrucciones;
    nuevoProceso.cant_instrucciones = cant_inst;
    nuevoProceso.metricas_proceso = pcb->metricas_proceso;
    nuevoProceso.tamanioMemoria = pcb->tamanioMemoria;

    int paginas_necesarias = (pcb->tamanioMemoria + tam_pagina -1) / tam_pagina;
    int pagina_actual = 0;
    nuevoProceso.tabla_primera = crear_tabla_paginacion(0, cantidad_niveles, entradas_por_tabla, &pagina_actual, paginas_necesarias);

    int paginas_asignadas = 0;
    asignar_marcos_tabla(nuevoProceso.tabla_primera, memoria_del_sistema, paginas_necesarias, &paginas_asignadas);
    log_debug(logger_memoria, "Creo una tabla de páginas (%d niveles, %d entradas) con %d paginas útiles y %d marcos ocupados para el proceso con PID %d", cantidad_niveles, entradas_por_tabla, paginas_necesarias, paginas_asignadas, nuevoProceso.pid);

    memoria_del_sistema->procesos[memoria_del_sistema->cant_procesos] = nuevoProceso;
    memoria_del_sistema->cant_procesos++;
}

void informar_metricas_memoria(int pid){
    
    // Buscar el proceso por PID
    int encontrado = -1;
    for (int i = 0; i < memoria_del_sistema->cant_procesos; i++) {
        if (memoria_del_sistema->procesos[i].pid == pid) {
            encontrado = i;
            break;
        }
    }
    metricas_proceso m = memoria_del_sistema->procesos[encontrado].metricas_proceso;

    log_info(logger_memoria,
    "## PID: %d - Proceso Destruido \n"
    "- Métricas - \n"
    "Acc.T.Pag: %d;\n"
    "Inst.Sol.: %d;\n"
    "SWAP: %d;\n"
    "Mem.Prin.: %d;\n"
    "Lec.Mem.: %d;\n"
    "Esc.Mem.: %d",
    pid, m.cantVecesTP, m.cantVecesInstrucciones, m.cantVecesSWAP,
    m.cantVecesMP, m.cantVecesRead, m.cantVecesWrite);
}

int finalizar_proceso(int pid) {
    int encontrado = -1;

    // Buscar el proceso por PID
    for (int i = 0; i < memoria_del_sistema->cant_procesos; i++) {
        if (memoria_del_sistema->procesos[i].pid == pid) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        log_error(logger_memoria, "No se encontró el proceso con PID %d\n", pid);
        return -1;
    }

    // Liberar instrucciones del proceso
    pthread_mutex_lock(&memoria_del_sistema->mutex); 
    for (int j = 0; j < memoria_del_sistema->procesos[encontrado].cant_instrucciones; j++) {
        log_trace(logger_memoria, "libero instruccion %s del proceso con PID %d", memoria_del_sistema->procesos[encontrado].instrucciones[j], pid);
        free(memoria_del_sistema->procesos[encontrado].instrucciones[j]);
    }
    log_trace(logger_memoria, "libero: %d instrucciones del proceso con PID %d", memoria_del_sistema->procesos[encontrado].cant_instrucciones, pid);
    free(memoria_del_sistema->procesos[encontrado].instrucciones);
    liberar_marcos_tabla(memoria_del_sistema->procesos[encontrado].tabla_primera, memoria_del_sistema);
    liberar_tabla(memoria_del_sistema->procesos[encontrado].tabla_primera);
    log_trace(logger_memoria, "libero: tabla de páginas del proceso con PID %d", pid);

    //Informar métricas:
    informar_metricas_memoria(pid);

    // Desplazar los procesos siguientes para llenar el hueco
    for (int i = encontrado; i < memoria_del_sistema->cant_procesos - 1; i++) {
        memoria_del_sistema->procesos[i] = memoria_del_sistema->procesos[i + 1];
    }

    memoria_del_sistema->cant_procesos--;

    // Redimensionar el array de procesos si hay procesos restantes
    if (memoria_del_sistema->cant_procesos > 0) {
        memoria_del_sistema->procesos = realloc(memoria_del_sistema->procesos, memoria_del_sistema->cant_procesos * sizeof(t_proceso_en_memoria));
    } else {
        free(memoria_del_sistema->procesos);
        memoria_del_sistema->procesos = NULL;
    }

    pthread_mutex_unlock(&memoria_del_sistema->mutex); 
    //printf("Proceso con PID %d liberado correctamente.\n", pid);
    return pid;
}

char* obtener_instruccion(int pid, int pc) {

    log_debug(logger_memoria, "ENTRA A OBTENER INSTRUCCION PID %d PC %d", pid, pc);

    //Esto no debería pasar nunca!!
    // if (memoria_del_sistema == NULL || memoria_del_sistema->procesos == NULL) {
    //     log_error(logger_memoria, "memoria_del_sistema o procesos no están inicializados");
    //     return NULL;
    // }

    //log_debug(logger_memoria, "Cant de procesos actual: %d", memoria_del_sistema->cant_procesos);
    pthread_mutex_lock(&memoria_del_sistema->mutex);

    for (int i = 0; i < memoria_del_sistema->cant_procesos; i++) {
        if (memoria_del_sistema->procesos[i].pid == pid) {
            log_trace(logger_memoria, "Instrucción solicitada: PID %d, PC %d", pid, pc);
            if (pc < memoria_del_sistema->procesos[i].cant_instrucciones) {
                memoria_del_sistema->procesos[i].metricas_proceso.cantVecesInstrucciones++;
                //log_debug(logger_memoria, "Se incrementa la métrica CantInstrucciones del proceso pid: %d", pid);
                pthread_mutex_unlock(&memoria_del_sistema->mutex);
                return memoria_del_sistema->procesos[i].instrucciones[pc];
            } else {  
                //Si el pc es mayor o igual a la cant de instrucciones:
                pthread_mutex_unlock(&memoria_del_sistema->mutex);
                return "PC FINALIZADO"; 
            }            
        }
    }

    pthread_mutex_unlock(&memoria_del_sistema->mutex);
    
    log_error(logger_memoria, "PID %d no encontrado", pid);
    log_error(logger_memoria, "cant procesos: %d", memoria_del_sistema->cant_procesos);
    return "NULL"; // PID no encontrado
}

char* leer_string_desde_buffer(t_buffer* buffer, int* desplazamiento) {
    int tamanio;
    memcpy(&tamanio, buffer->stream + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    char* string = malloc(tamanio);
    memcpy(string, buffer->stream + *desplazamiento, tamanio);
    *desplazamiento += tamanio;

    return string;
}

t_tabla_pagina* crear_tabla_paginacion(int nivel_actual, int cantidad_niveles, int entradas_por_tabla, int* pagina_actual, int paginas_totales){
    t_tabla_pagina* tabla = malloc(sizeof(t_tabla_pagina));
    tabla->cant_entradas = entradas_por_tabla;

    if (nivel_actual == cantidad_niveles - 1) {
        tabla->tipo = NIVEL_FINAL;
        tabla->entradas = calloc(entradas_por_tabla, sizeof(t_entrada_pagina));

        for (int i = 0; i < entradas_por_tabla; i++) {
            if (*pagina_actual >= paginas_totales) break;

            tabla->entradas[i].presente = false;
            tabla->entradas[i].marco = 0;
            tabla->entradas[i].num_pagina = *pagina_actual;
            tabla->entradas[i].uso = false;
            tabla->entradas[i].modificado = false;

            (*pagina_actual)++;
        }
    } else {
        tabla->tipo = NIVEL_INTERMEDIO;
        tabla->subtablas = calloc(entradas_por_tabla, sizeof(t_tabla_pagina*));

        for (int i = 0; i < entradas_por_tabla; i++) {
            tabla->subtablas[i] = crear_tabla_paginacion(nivel_actual + 1, cantidad_niveles, entradas_por_tabla, pagina_actual, paginas_totales);
        }
    }
    
    return tabla;
}

void liberar_tabla(t_tabla_pagina* tabla) {
    if (tabla->tipo == NIVEL_INTERMEDIO) { // Si es de NIVEL_INTERMEDIO recorre todas las subtablas y las libera
        for (int i = 0; i < tabla->cant_entradas; i++) {
            liberar_tabla(tabla->subtablas[i]);
        }
        free(tabla->subtablas);
    } else {
        free(tabla->entradas); // Cuando es NIVEL_FINAL libera las entradas
    }
    free(tabla);
}

t_proceso_en_memoria* buscar_proceso_en_memoria(int pid) {
    for (int i = 0; i < memoria_del_sistema->cant_procesos; i++) {
        if (memoria_del_sistema->procesos[i].pid == pid)
            return &memoria_del_sistema->procesos[i];
    }
    return NULL;
}

void asignar_marcos_tabla(t_tabla_pagina* tabla, t_memoria_del_sistema* memoria, int paginas_necesarias, int* paginas_asignadas) {
    if (*paginas_asignadas >= paginas_necesarias) return;
    // Si es nivel intermedio, recorro hasta encontrar el nivel final
    if (tabla->tipo == NIVEL_INTERMEDIO) {
        for (int i = 0; i < tabla->cant_entradas; i++) {
            asignar_marcos_tabla(tabla->subtablas[i], memoria, paginas_necesarias, paginas_asignadas);
            if (*paginas_asignadas >= paginas_necesarias) break;
        }
    } else {
        for (int i = 0; i < tabla->cant_entradas; i++) {
            if (*paginas_asignadas >= paginas_necesarias) break;
            if (tabla->entradas[i].presente) continue; // Evita doble asignación

            int marco = buscar_marco_libre(memoria);
            if (marco == -1) {
                log_error(logger_memoria, "No hay marcos libres suficientes");
                exit(EXIT_FAILURE);
            }

            tabla->entradas[i].marco = marco;
            tabla->entradas[i].presente = true;
            tabla->entradas[i].uso = false;
            tabla->entradas[i].modificado = false;
            tabla->entradas[i].num_pagina = i;
            memoria->bitmap_marcos_ocupados[marco] = true;

            (*paginas_asignadas)++;
        }
    }
}

void liberar_marcos_tabla(t_tabla_pagina* tabla, t_memoria_del_sistema* memoria) {
    if (tabla->tipo == NIVEL_INTERMEDIO) {
        for (int i = 0; i < tabla->cant_entradas; i++) {
            if (tabla->subtablas[i] != NULL) {
                liberar_marcos_tabla(tabla->subtablas[i], memoria);
            }
        }
    } else {
        for (int i = 0; i < tabla->cant_entradas; i++) {
            if (tabla->entradas[i].presente) {
                int marco = tabla->entradas[i].marco;
                memoria->bitmap_marcos_ocupados[marco] = false;
                log_trace(logger_memoria, "Se liberó el marco %d", marco);
            }
        }
    }
}

int buscar_marco_libre(t_memoria_del_sistema* memoria) {
    for (int i = 0; i < memoria->cant_marcos; i++) {
        if (!memoria->bitmap_marcos_ocupados[i]) {
            return i;
        }
    }
    return -1;
}