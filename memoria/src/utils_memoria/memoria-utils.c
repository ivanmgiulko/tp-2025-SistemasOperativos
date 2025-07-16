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
procesos_en_swap_t crear_lista_procesos_en_swap() {
    procesos_en_swap_t procesos_en_swap;
    procesos_en_swap.procesos_swap = list_create();
    pthread_mutex_init(&(procesos_en_swap.mutex_procesos_swap), NULL);
    return procesos_en_swap;
}
int buscar_indice_de_proceso_en_memoria(uint8_t pid){
    int encontrado = -1;
    // Buscar el indice de proceso por PID
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
    return encontrado;
}

void inicializar_swap() {
    char* path_swap = config_memoria->PATH_SWAPFILE;

    FILE* archivo_swap = fopen(path_swap, "w+b");
    if (!archivo_swap) {
        log_error(logger_memoria, "No se pudo crear el archivo SWAP.");
        exit(EXIT_FAILURE);
    }

    fclose(archivo_swap);

    log_info(logger_memoria, "SWAP inicializado en: %s", path_swap);
}

void suspender_proceso_swap(uint8_t pid) {
    // Buscamos proceso e inicializamos configuraciones
    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "PID %d no encontrado en memoria para suspender", pid);
        return;
    }
    int tam_pagina = atoi(config_memoria->TAM_PAGINA);
    int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);

    // Instanciamos proceso del swap
    t_proceso_swap* proceso_swap = malloc(sizeof(t_proceso_swap));
    proceso_swap->pid = pid;
    proceso_swap->cantidad_paginas = proceso->tamanioMemoria / tam_pagina;
    proceso_swap->posiciones_swap = malloc(sizeof(uint32_t) * proceso_swap->cantidad_paginas);

    // Abrimos archivo de swap
    FILE* swapfile = fopen(config_memoria->PATH_SWAPFILE, "r+b");
    if (!swapfile) {
        log_error(logger_memoria, "No se pudo abrir el archivo SWAP para escritura.");
        return;
    }

    // Copiamos el contenido de las paginas del proceso en el SWAP según los marcos asignados a c/u 
    for (uint32_t i = 0; i < proceso_swap->cantidad_paginas; i++) {
        fseek(swapfile, 0, SEEK_END);
        uint32_t offset = ftell(swapfile);
        proceso_swap->posiciones_swap[i] = offset;

        int marco = buscar_marco_en_tabla_full(proceso->tabla_primera, i, cantidad_niveles, entradas_por_tabla);
        void* origen = memoria_del_sistema->memoria_principal + marco * tam_pagina;
        fwrite(origen, 1, tam_pagina, swapfile);

        log_trace(logger_memoria, "PID %d: Página %d escrita en offset %d de SWAP", pid, i, offset);
    }

    // el pepe
    fclose(swapfile);
    pthread_mutex_lock(&(procesos_en_swap->mutex_procesos_swap));
    list_add(procesos_en_swap->procesos_swap, proceso_swap);
    pthread_mutex_unlock(&(procesos_en_swap->mutex_procesos_swap));

}

void desuspender_proceso_swap(uint8_t pid) {
    // Buscamos proceso SWAP
    pthread_mutex_lock(&(procesos_en_swap->mutex_procesos_swap));
    t_proceso_swap* proceso_swap = buscar_proceso_en_swap(pid);
    pthread_mutex_unlock(&(procesos_en_swap->mutex_procesos_swap));

    if (!proceso_swap) {
        log_error(logger_memoria, "PID %d no encontrado en SWAP para desuspender", pid);
        return;
    }

    // Busco proceso en memoria a desuspender
    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "PID %d no encontrado en memoria para desuspender", pid);
        return;
    }

    // Abro archivo SWAP
    FILE* swapfile = fopen(config_memoria->PATH_SWAPFILE, "r+b");
    if (!swapfile) {
        log_error(logger_memoria, "No se pudo abrir el archivo SWAP para lectura.");
        return;
    }

    // Configuración de parámetros
    int tam_pagina = atoi(config_memoria->TAM_PAGINA);
    int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);

    // Asigno marcos en memoria para todas las páginas a restaurar
    int paginas_asignadas = 0;
    asignar_marcos_tabla(proceso->tabla_primera, memoria_del_sistema, proceso_swap->cantidad_paginas, &paginas_asignadas);

    // Restaurar página por página desde SWAP a la memoria
    for (uint32_t i = 0; i < proceso_swap->cantidad_paginas; i++) {
        long offset = proceso_swap->posiciones_swap[i];
        fseek(swapfile, offset, SEEK_SET);

        // Obtengo el marco donde fue asignada esta página
        int marco = buscar_marco_en_tabla_full(proceso->tabla_primera, i, cantidad_niveles, entradas_por_tabla);
        if (marco == -1) {
            log_error(logger_memoria, "No se pudo obtener el marco de la página %d del PID %d", i, pid);
        }
        void* destino = memoria_del_sistema->memoria_principal + marco * tam_pagina;

        fread(destino, 1, tam_pagina, swapfile);
        log_trace(logger_memoria, "PID %d: Página %d restaurada desde SWAP (offset %ld) al marco %d", pid, i, offset, marco);
    }

    fclose(swapfile);
}

void liberar_swap_proceso(int pid) {
    pthread_mutex_lock(&(procesos_en_swap->mutex_procesos_swap));
    t_proceso_swap* proceso_swap = buscar_proceso_en_swap(pid);
    pthread_mutex_unlock(&(procesos_en_swap->mutex_procesos_swap));

    if (!proceso_swap){
        log_error(logger_memoria, "PID %d no encontrado para liberar en SWAP", pid);
        return;
    }
    pthread_mutex_lock(&(procesos_en_swap->mutex_procesos_swap));
    list_remove_element(procesos_en_swap->procesos_swap, proceso_swap);
    pthread_mutex_unlock(&(procesos_en_swap->mutex_procesos_swap));

    free(proceso_swap->posiciones_swap);
    free(proceso_swap);
    log_info(logger_memoria, "Liberado espacio SWAP del proceso PID %d", pid);
}

t_proceso_swap* buscar_proceso_en_swap(uint8_t pid) {
    for (int i = 0; i < list_size(procesos_en_swap->procesos_swap); i++) {
        t_proceso_swap* proceso_swap = list_get(procesos_en_swap->procesos_swap, i);
        if (proceso_swap->pid == pid) return proceso_swap;
    }
    return NULL;
}

char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
    pathArchivoPseudocodigo = "/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt";
    log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
    
    // Construir la ruta correcta. Si es una ruta relativa, agregarle "../" para salir del directorio memoria
    char* ruta_corregida;
    if (pathArchivoPseudocodigo[0] == '/') {
        // Es una ruta absoluta, usarla tal como está
        ruta_corregida = strdup(pathArchivoPseudocodigo);
    } else {
        // Es una ruta relativa, construir la ruta desde el directorio memoria hacia kernel
        ruta_corregida = malloc(strlen(pathArchivoPseudocodigo) + 4); // +4 para "../"
        sprintf(ruta_corregida, "../%s", pathArchivoPseudocodigo);
    }
    
    log_debug(logger_memoria, "Intentando abrir archivo en: %s", ruta_corregida);
    FILE* archivo = fopen(ruta_corregida, "r");
    if (!archivo) {
        perror("Error abriendo archivo de pseudocodigo");
        free(ruta_corregida);
        return NULL;
    }

    int capacidad = 10;
    char** instrucciones = malloc(capacidad * sizeof(char*));
    if (!instrucciones) {
        fclose(archivo);
        free(ruta_corregida);
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
    free(ruta_corregida);
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

void informar_metricas_memoria(uint8_t pid){
    
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

uint8_t finalizar_proceso(uint8_t pid) {
    int encontrado = buscar_indice_de_proceso_en_memoria(pid);

    // Liberar instrucciones del proceso
    pthread_mutex_lock(&memoria_del_sistema->mutex); 
    for (int j = 0; j < memoria_del_sistema->procesos[encontrado].cant_instrucciones; j++) {
        log_trace(logger_memoria, "libero instruccion %s del proceso con PID %d", memoria_del_sistema->procesos[encontrado].instrucciones[j], pid);
        free(memoria_del_sistema->procesos[encontrado].instrucciones[j]);
    }
    log_trace(logger_memoria, "libero: %d instrucciones del proceso con PID %d", memoria_del_sistema->procesos[encontrado].cant_instrucciones, pid);
    free(memoria_del_sistema->procesos[encontrado].instrucciones);

    //Liberar espacio en memoria
    liberar_espacios_memoria_usuario(memoria_del_sistema->procesos[encontrado].tabla_primera,memoria_del_sistema);
    //Liberar tablas de pagina
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

    // liberar proceso en lista de procesos en swap
    liberar_swap_proceso(pid);

    pthread_mutex_unlock(&memoria_del_sistema->mutex); 
    //printf("Proceso con PID %d liberado correctamente.\n", pid);
    return pid;
}

char* obtener_instruccion(uint8_t pid, uint16_t pc) {

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
                //METRICA
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

t_proceso_en_memoria* buscar_proceso_en_memoria(uint8_t pid) {
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

int32_t buscar_marco_en_tabla(t_tabla_pagina* tabla_primera, uint32_t* entradas_por_nivel, int cantidad_niveles) {
    t_tabla_pagina* actual = tabla_primera;

    // Recorremos los niveles intermedios
    for (int nivel = 0; nivel < cantidad_niveles - 1; nivel++) {
        int indice = entradas_por_nivel[nivel];
        if (indice >= actual->cant_entradas || actual->subtablas[indice] == NULL) {
            log_error(logger_memoria, "Entrada de tabla de nivel intermedio inválida (nivel %d, índice %d)", nivel, indice);
            return -1;
        }
        actual = actual->subtablas[indice];
    }

    // En el nivel final, buscamos la entrada correspondiente
    int entrada_final = entradas_por_nivel[cantidad_niveles - 1];
    if (entrada_final >= actual->cant_entradas) {
        log_error(logger_memoria, "Entrada de nivel final inválida: %d", entrada_final);
        return -1;
    }

    t_entrada_pagina entrada = actual->entradas[entrada_final];
    if (!entrada.presente) {
        log_error(logger_memoria, "Página no presente en memoria: %d", entrada.num_pagina);
        return -1;
    }

    return entrada.marco;
}

// Recorre todas las entradas de la tabla - TEMPORAL
int buscar_marco_en_tabla_full(t_tabla_pagina* tabla_primera, int nro_pagina, int cantidad_niveles, int entradas_por_tabla) {
    t_tabla_pagina* actual = tabla_primera;
    int pagina_restante = nro_pagina;

    // Recorremos hasta el nivel final
    for (int nivel = 0; nivel < cantidad_niveles - 1; nivel++) {
        int indice = pagina_restante / (int)pow(entradas_por_tabla, cantidad_niveles - 1 - nivel);
        pagina_restante %= (int)pow(entradas_por_tabla, cantidad_niveles - 1 - nivel);

        if (indice >= actual->cant_entradas || actual->subtablas[indice] == NULL) {
            log_error(logger_memoria, "Entrada de tabla de nivel intermedio inválida (nivel %d, índice %d)", nivel, indice);
            return -1;
        }

        actual = actual->subtablas[indice];
    }

    // En nivel final, buscamos la entrada que representa la página lógica
    int entrada_final = pagina_restante;
    if (entrada_final >= actual->cant_entradas) {
        log_error(logger_memoria, "Entrada de nivel final inválida: %d", entrada_final);
        return -1;
    }

    t_entrada_pagina entrada = actual->entradas[entrada_final];
    if (!entrada.presente) {
        log_error(logger_memoria, "Página no presente en memoria: %d", entrada.num_pagina);
        return -1;
    }

    return entrada.marco;
}

void liberar_espacios_memoria_usuario(t_tabla_pagina* tabla_primera, t_memoria_del_sistema* memoria_del_sistema) {
    if (!tabla_primera) return;

    if (tabla_primera->tipo == NIVEL_INTERMEDIO) {
        for (int i = 0; i < tabla_primera->cant_entradas; i++) {
            if (tabla_primera->subtablas[i])
                liberar_espacios_memoria_usuario(tabla_primera->subtablas[i], memoria_del_sistema);
        }
    } else if (tabla_primera->tipo == NIVEL_FINAL) {
        for (int i = 0; i < tabla_primera->cant_entradas; i++) {
            t_entrada_pagina* entrada = &tabla_primera->entradas[i];
            if (entrada->presente) {
                int marco = entrada->marco;
                memoria_del_sistema->bitmap_marcos_ocupados[marco] = false;
                //  limpiar el contenido de la página en memoria_principal
                memset(memoria_del_sistema->memoria_principal + marco * memoria_del_sistema->tam_pagina, 0, memoria_del_sistema->tam_pagina);
            }
        }
    }
}

//FUNCIONES DE CONEXION:

void manejar_peticion_de_instruccion(int socket_cliente, t_paquete* paquete) {
    if (paquete->buffer->size < sizeof(int) * 2) {
        log_error(logger_memoria, "El tamaño del buffer es insuficiente para deserializar la instrucción");
        return;
    }

    t_peticion_instruccion* peticion = deserializar_peticion_instruccion(paquete->buffer->stream);

    log_info(logger_memoria, "PID recibido: %d", peticion->pid);
    log_info(logger_memoria, "PC recibido: %d", peticion->pc);

	//Obtengo la instruccion correspondiente al PID y PC recibido de cpu
	char* respuesta_instruccion = string_duplicate(obtener_instruccion(peticion->pid, peticion->pc));

	//Entra a este if cuando el pc es mayor a cant de instrucciones
	if(strcmp(respuesta_instruccion, "PC FINALIZADO")== 0){
	
		log_info(logger_memoria, "No hay más instrucciones a ejecutar para este proceso");
		log_debug(logger_memoria, "Serializando paquete:");
		log_debug(logger_memoria, "Código de operación: %d", FIN_PID);

		t_paquete* paquete_fin = crear_paquete_con_codigo(FIN_PID);
        enviar_paquete(paquete_fin, socket_cliente);

        //Libero memoria
		eliminar_paquete(paquete_fin);
        free(respuesta_instruccion);
        free(peticion);
		return;
	}

	//Entra aca si Memoria del sistema no incializada o pid no encontrado
	else if(strcmp(respuesta_instruccion, "NULL")== 0){
	 	log_error(logger_memoria, "pid no encontrado o memoria_del_sistema/procesos no están inicializados");
		free(respuesta_instruccion);
        free(peticion);
        return;
	} 

	//Entra acá si encontro el proceso y la instrucción
	else{
		log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s",peticion->pid, peticion->pc, respuesta_instruccion);
		
        //Serializo la respuesta
		t_paquete* paquete_instruccion = crear_paquete_con_codigo(INSTRUCCION);
        agregar_a_paquete(paquete_instruccion, respuesta_instruccion, strlen(respuesta_instruccion)+1);
        
		log_debug(logger_memoria, "Serializando paquete:");
		log_debug(logger_memoria, "Código de operación: %d", paquete_instruccion->codigo_operacion);
		log_debug(logger_memoria, "Tamaño del buffer: %d", paquete_instruccion->buffer->size);
		log_debug(logger_memoria, "Instrucción: %s", respuesta_instruccion);

		//Envio la instruccion serializada envio a CPU 
		
		log_info(logger_memoria, "Enviando Instrucción a CPU");
		enviar_paquete(paquete_instruccion, socket_cliente);
        log_debug(logger_memoria, "[SEND] Instrucción ENVIADA al socket: %d, suerte cpu!!", socket_cliente);
	    //Libero memoria
        eliminar_paquete(paquete_instruccion);
	    free(respuesta_instruccion);
        free(peticion);
    }
}

void manejar_escritura_memoria(int socket_cliente, t_paquete* paquete) {

	
	//Deserializo el paquete:
    int offset = 0;
	uint8_t pid = leer_uint8_desde_buffer(paquete->buffer, &offset);
	uint32_t direccion_fisica = leer_uint32_desde_buffer(paquete->buffer, &offset);
	char* datos = leer_string_desde_buffer(paquete->buffer, &offset);

    //ESCRIBE EN MEMORIA
	memcpy(memoria_del_sistema->memoria_principal + direccion_fisica, datos, strlen(datos));

	// Log obligatorio
	log_trace(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d ", pid, direccion_fisica);

	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesWrite++;

    // Enviar confirmación de éxito al cpu
	t_paquete* paquete_confirmacion_write = crear_paquete_con_codigo(WRITE_MEMORIA);
    char* mensaje_confirmacion_write = "WRITE completado con éxito";
    agregar_a_paquete(paquete_confirmacion_write, mensaje_confirmacion_write, strlen(mensaje_confirmacion_write)+1);

    log_info(logger_memoria, "Enviando confirmación de WRITE a CPU: %s", mensaje_confirmacion_write);
    enviar_paquete(paquete_confirmacion_write, socket_cliente);

	free(datos);
	eliminar_paquete(paquete_confirmacion_write);
}

void manejar_lectura_memoria(int socket_cliente, t_paquete* paquete) {

    //Deserializo el paquete:
    int offset = 0;
	uint32_t pid = leer_uint32_desde_buffer(paquete->buffer, &offset);
	uint32_t direccion_fisica = leer_uint32_desde_buffer(paquete->buffer, &offset);
	uint32_t tamanio_a_leer = leer_uint32_desde_buffer(paquete->buffer, &offset);

    log_info(logger_memoria, "[MEMORIA] READ recibido - Dirección: %d | Tamaño: %d", direccion_fisica, tamanio_a_leer);

	void* datos_leidos = malloc(tamanio_a_leer);
	memcpy(datos_leidos, memoria_del_sistema->memoria_principal + direccion_fisica, tamanio_a_leer);
	char* datos_leidos_como_string = calloc(tamanio_a_leer + 1, sizeof(char));
	memcpy(datos_leidos_como_string, datos_leidos, tamanio_a_leer);

	// Log obligatorio
	log_trace(logger_memoria, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tamanio_a_leer);
	log_trace(logger_memoria, "Contenido leído: %s", datos_leidos_como_string);

	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesWrite++;

    // Enviar confirmación de éxito al cliente (reemplazar luego por lógica de lectura en memoria)
	t_paquete* paquete_confirmacion_read = crear_paquete_con_codigo(READ_MEMORIA);
	
    agregar_a_paquete(paquete_confirmacion_read, datos_leidos_como_string, strlen(datos_leidos_como_string)+1);

	log_debug(logger_memoria, "[SEND] Enviando código de operación: %d (READ_MEMORIA)", READ_MEMORIA);
	log_debug(logger_memoria, "[SEND] Enviando tamaño: %d", tamanio_a_leer);
	log_debug(logger_memoria, "[SEND] Enviando contenido leído: %s", datos_leidos_como_string);
	
    enviar_paquete(paquete_confirmacion_read, socket_cliente);

	free(datos_leidos);
	free(datos_leidos_como_string);
	eliminar_paquete(paquete_confirmacion_read);
}

void manejar_acceso_tablas_de_paginas(int socket_cliente, t_paquete* paquete) {
	
	//deserializo el paquete pid, nro pagina, entradas por nivel
	int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);
    int desplazamiento = 0;

    uint8_t pid = leer_uint8_desde_buffer(paquete->buffer, &desplazamiento);

	t_pre_direccion_fisica direccion;
    direccion.nro_pagina= leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);
    direccion.entrada_nivel = malloc(sizeof(uint32_t) * cantidad_niveles);

    for (int i = 0; i < cantidad_niveles; i++){
        direccion.entrada_nivel[i] = leer_uint32_desde_buffer(paquete->buffer, &desplazamiento);
	}

	log_debug(logger_memoria, "Iniciando busqueda de marco de pagina %d para proceso PID: %d", direccion.nro_pagina, pid);

    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "PID %d no encontrado en memoria", pid);
        free(direccion.entrada_nivel);
        return;
    }

    int32_t marco = buscar_marco_en_tabla(proceso->tabla_primera, direccion.entrada_nivel, cantidad_niveles);
    if (marco == -1) {
        log_error(logger_memoria, "No se pudo encontrar el marco solicitado de la pagina %d para PID %d",direccion.nro_pagina, pid);
        free(direccion.entrada_nivel);
        return;
    }
	
    log_trace(logger_memoria, "PID: %d - Página: %d - Marco: %d", pid, direccion.nro_pagina, marco);

	//METRICAS
	int indice = buscar_indice_de_proceso_en_memoria(pid);
	memoria_del_sistema->procesos[indice].metricas_proceso.cantVecesTP += atoi(config_memoria->CANTIDAD_NIVELES);

    // Enviar el marco
    t_paquete* paquete_marco = crear_paquete_con_codigo(OBTENER_MARCO_CORRESPONDIENTE);

    agregar_a_paquete(paquete_marco, &marco, sizeof(int32_t));
    
	// LOG DEL TAMAÑO DEL BUFFER Y CONTENIDO
    log_debug(logger_memoria, "[SEND] Enviando código de operación: %d (OBTENER_MARCO_CORRESPONDIENTE)", OBTENER_MARCO_CORRESPONDIENTE);
    log_debug(logger_memoria, "[SEND] Enviando tamaño: %d", paquete_marco->buffer->size);
    log_debug(logger_memoria, "[SEND] Enviando marco: %d", marco);
   
    enviar_paquete(paquete_marco, socket_cliente);
    log_debug(logger_memoria, "[SEND] MARCO ENVIADO al socket: %d, suerte cpu!!", socket_cliente);
    eliminar_paquete(paquete_marco);
    free(direccion.entrada_nivel);
}

void enviar_respuesta_kernel(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = crear_paquete_con_codigo(PROCESO_MEMORIA);
    
    agregar_a_paquete(paquete, mensaje, strlen(mensaje)+1);

    enviar_paquete(paquete,socket_cliente);

	eliminar_paquete(paquete);
}

void enviar_proceso_terminado(uint8_t pid, int socket_cliente) {

    t_paquete* paquete_proceso_eliminado = crear_paquete_con_codigo(PROCESO_FINALIZADO);
    
    agregar_a_paquete(paquete_proceso_eliminado, &pid, sizeof(uint8_t));

    enviar_paquete(paquete_proceso_eliminado, socket_cliente);

    eliminar_paquete(paquete_proceso_eliminado);
}

t_pcb* recibir_proceso_a_dumpear_desde_kernel(t_buffer* buffer) 
{
	t_pcb* proceso_a_dumpear = malloc(sizeof(t_pcb));

    memcpy(&(proceso_a_dumpear->pid), buffer->stream, sizeof(uint8_t));

    return proceso_a_dumpear;
}

void enviar_respuesta_dump_memory(uint8_t pid, bool respuesta, int socket_cliente) {

    t_paquete* paquete = crear_paquete_con_codigo(RESPUESTA_DUMPEO);

    agregar_a_paquete(paquete, &pid, sizeof(uint8_t));
    agregar_a_paquete(paquete, &respuesta, sizeof(bool));

    enviar_paquete(paquete, socket_cliente);
    
    eliminar_paquete(paquete);
}

bool realizar_dump_memory(uint8_t pid) {
	// Buscamos proceso a dumpear
    t_proceso_en_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "DUMP_MEMORY: proceso %d no registrado en memoria", pid);
        return false;
    }

	int tam_pagina = atoi(config_memoria->TAM_PAGINA);
    int entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);
    int cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);

    // Creo directorio
    char* dump_path = config_memoria->DUMP_PATH;
    int resultado = mkdir(dump_path, 0755);
	if (resultado == 0) {
        log_trace(logger_memoria, "DUMP_MEMORY: directorio creado con éxito");
    } else {
        log_trace(logger_memoria, "DUMP_MEMORY: el directorio ha sido creado anteriormente");
    }

    // Creo el timestamp para el nombre del archivo a dumpear
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", &tm);

    // Armamos nombre completo del timestamp anashei
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%d-%s.dmp", dump_path, pid, timestamp);

    // Abrimos puerta al nether
    FILE* file = fopen(filename, "w+b");
    if (!file) {
        log_error(logger_memoria, "DUMP_MEMORY: no pudo abrirse el archivo del dump %s", filename);
        return false;
    }

	// Recorrer páginas y volcar su contenido
    int paginas_totales = proceso->tamanioMemoria / tam_pagina;

    for (int nro_pagina = 0; nro_pagina < paginas_totales; nro_pagina++) {
		uint32_t* entradas_por_nivel = calcular_entradas_por_nivel(nro_pagina, cantidad_niveles, entradas_por_tabla);
		if (!entradas_por_nivel) {
			log_error(logger_memoria, "Error al calcular las entradas por nivel");
			break;
		}

        int marco = buscar_marco_en_tabla(proceso->tabla_primera, entradas_por_nivel, cantidad_niveles);
		free(entradas_por_nivel);

        if (marco == -1) {
            log_warning(logger_memoria, "Página %d no asignada (se saltea)", nro_pagina);
            char vacio[tam_pagina];
            memset(vacio, 0, tam_pagina);
            fwrite(vacio, 1, tam_pagina, file);  // rellena con ceros si no está asignada
        } else {
            void* origen = memoria_del_sistema->memoria_principal + (marco * tam_pagina);
            fwrite(origen, 1, tam_pagina, file);
        }
    }

    fclose(file);
    log_trace(logger_memoria, "DUMP_MEMORY: archivo creado %s", filename);
    return true;
}

void avisar_kernel_mande_otro_proceso(int socket_cliente) {

	t_paquete* paquete = crear_paquete_con_codigo(SUSPENSION_HECHA);
    
	enviar_paquete(paquete,socket_cliente);

	eliminar_paquete(paquete);
}

void enviar_datos_a_cpu(int socket_cliente){
	// Serializar los datos de memoria en el buffer del paquete
	uint32_t tam_pagina = atoi(config_memoria->TAM_PAGINA);
	uint32_t cantidad_niveles = atoi(config_memoria->CANTIDAD_NIVELES);
	uint32_t entradas_por_tabla = atoi(config_memoria->ENTRADAS_POR_TABLA);

	t_paquete* paquete = crear_paquete_con_codigo(DATOS_DE_MEMORIA);
    agregar_a_paquete(paquete, &tam_pagina, sizeof(uint32_t));
    agregar_a_paquete(paquete, &cantidad_niveles, sizeof(uint32_t));
    agregar_a_paquete(paquete, &entradas_por_tabla, sizeof(uint32_t));

    enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

uint32_t* calcular_entradas_por_nivel(uint32_t nro_pagina, int cantidad_niveles, int entradas_por_tabla) {
    uint32_t* entradas = malloc(sizeof(uint32_t) * cantidad_niveles);
    if (!entradas) return NULL;

    for (int nivel = cantidad_niveles - 1; nivel >= 0; nivel--) {
        entradas[nivel] = nro_pagina % entradas_por_tabla;
        nro_pagina /= entradas_por_tabla;
    }

    return entradas;
}