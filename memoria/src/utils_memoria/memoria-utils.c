#include "memoria-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
t_memoriaDelSistema crear_memoria_del_sistema() {
    t_memoriaDelSistema memoria;
    memoria.procesos = malloc(sizeof(t_procesoEnMemoria)); 
    memoria.cant_procesos = 0;
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
char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
    if (pathArchivoPseudocodigo == NULL || string_is_empty(pathArchivoPseudocodigo)) {
        log_error(logger_memoria, "El path del archivo es inválido o está vacío");
        return NULL;
    }

    log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
    FILE* archivo = fopen(pathArchivoPseudocodigo, "r");
    if (!archivo) {
        perror("Error abriendo archivo de pseudocodigo");
        return NULL;
    }

    char** instrucciones = string_array_new(); // Crear un array de strings vacío
    char buffer[MAX_LINEA];

    while (fgets(buffer, MAX_LINEA, archivo)) {
        string_array_push(&instrucciones, string_duplicate(buffer)); // Agregar la instrucción al array
    }

    fclose(archivo);
    *cantidad = string_array_size(instrucciones); // Obtener la cantidad de instrucciones
    return instrucciones;
}
void agregar_proceso(t_pcbMemoria* pcb) {
    int cant_inst = 0;
    char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    
    if (!instrucciones) {
        log_error(logger_memoria, "No se pudieron leer las instrucciones para el proceso PID %d", pcb->pid);
        return;
    }

    t_procesoEnMemoria* temp = realloc(memoriaDelSistema->procesos, (memoriaDelSistema->cant_procesos + 1) * sizeof(t_procesoEnMemoria));
    if (!temp) {
        log_error(logger_memoria, "Error al reasignar memoria para procesos");
        for (int i = 0; i < cant_inst; i++) {
            free(instrucciones[i]);
        }
        free(instrucciones);
        return;
    }
    memoriaDelSistema->procesos = temp;

    t_procesoEnMemoria nuevoProceso;
    nuevoProceso.pid = pcb->pid;
    nuevoProceso.instrucciones = instrucciones;
    nuevoProceso.cant_instrucciones = cant_inst;

    memoriaDelSistema->procesos[memoriaDelSistema->cant_procesos] = nuevoProceso;
    memoriaDelSistema->cant_procesos++;
}

//hehco por nosotros

// char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
//    // pathArchivoPseudocodigo = "/home/utnso/Desktop/tp-2025-1c-FAMILIA-MATRIX/kernel/PATH_INSTRUCCIONES.txt";
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
//         // Eliminar salto de línea si lo hay
//         buffer[strcspn(buffer, "\n")] = 0;

//         if (count >= capacidad) {
//             capacidad *= 2;
//             instrucciones = realloc(instrucciones, capacidad * sizeof(char*));
//         }

//         instrucciones[count] = strdup(buffer); // Copia segura
//         count++;
//     }

//     fclose(archivo);
//     *cantidad = count;
//     return instrucciones;
// }

// void agregar_proceso(t_pcbMemoria* pcb) {
//     int cant_inst = 0;
//     char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    
//     if (!instrucciones) return;

//     memoriaDelSistema->procesos = realloc(memoriaDelSistema->procesos, (memoriaDelSistema->cant_procesos + 1) * sizeof(t_procesoEnMemoria));

//     t_procesoEnMemoria nuevoProceso;
//     nuevoProceso.pid = pcb->pid;
//     nuevoProceso.instrucciones = instrucciones;
//     nuevoProceso.cant_instrucciones = cant_inst;

//     memoriaDelSistema->procesos[memoriaDelSistema->cant_procesos] = nuevoProceso;
//     memoriaDelSistema->cant_procesos++;
// }

int finalizar_proceso(int pid) {
    int encontrado = -1;

    // Buscar el proceso por PID
    for (int i = 0; i < memoriaDelSistema->cant_procesos; i++) {
        if (memoriaDelSistema->procesos[i].pid == pid) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        //printf("No se encontró el proceso con PID %d\n", pid);
        return -1;
    }

    // Liberar instrucciones del proceso
    for (int j = 0; j < memoriaDelSistema->procesos[encontrado].cant_instrucciones; j++) {
        free(memoriaDelSistema->procesos[encontrado].instrucciones[j]);
    }
    free(memoriaDelSistema->procesos[encontrado].instrucciones);

    // Desplazar los procesos siguientes para llenar el hueco
    for (int i = encontrado; i < memoriaDelSistema->cant_procesos - 1; i++) {
        memoriaDelSistema->procesos[i] = memoriaDelSistema->procesos[i + 1];
    }

    memoriaDelSistema->cant_procesos--;

    // Redimensionar el array de procesos si hay procesos restantes
    if (memoriaDelSistema->cant_procesos > 0) {
        memoriaDelSistema->procesos = realloc(memoriaDelSistema->procesos, memoriaDelSistema->cant_procesos * sizeof(t_procesoEnMemoria));
    } else {
        free(memoriaDelSistema->procesos);
        memoriaDelSistema->procesos = NULL;
    }

    //printf("Proceso con PID %d liberado correctamente.\n", pid);
    return pid;
}

char* obtener_instruccion(int pid, int pc) {
    log_debug(logger_memoria, "ENTRA A OBTENER INSTRUCCION PID %d PC %d", pid, pc);
    if (memoriaDelSistema == NULL || memoriaDelSistema->procesos == NULL) {
        log_error(logger_memoria, "memoriaDelSistema o procesos no están inicializados");
        return NULL;
    }
   // if(pc <0) return NULL; // PC inválido
    for (int i = 0; i < memoriaDelSistema->cant_procesos; i++) {
        if (memoriaDelSistema->procesos[i].pid == pid) {
            log_trace(logger_memoria, "Instrucción solicitada: PID %d, PC %d", pid, pc);
            if (pc < memoriaDelSistema->procesos[i].cant_instrucciones) {
                return memoriaDelSistema->procesos[i].instrucciones[pc];
            } else {
                log_debug(logger_memoria, "PID %d no encontrado", memoriaDelSistema->procesos[i].pid);
                log_debug(logger_memoria, "PID %d no encontrado", memoriaDelSistema->procesos[i].cant_instrucciones);
                log_debug(logger_memoria, "PID %d no encontrado", memoriaDelSistema->cant_procesos);
                log_debug(logger_memoria, "PID %s no encontrado", memoriaDelSistema->procesos[i].instrucciones[pc]);
                return NULL; // PC inválido
            }
                log_debug(logger_memoria, "1er if PID %d no encontrado", memoriaDelSistema->procesos[i].pid);
                log_debug(logger_memoria, "1er if PID %d no encontrado", memoriaDelSistema->procesos[i].cant_instrucciones);
                log_debug(logger_memoria, "1er if PID %d no encontrado", memoriaDelSistema->cant_procesos);
                log_debug(logger_memoria, "1er if PID %s no encontrado", memoriaDelSistema->procesos[i].instrucciones[pc]);
        }
        
        log_debug(logger_memoria, "for PID %d no encontrado", memoriaDelSistema->procesos[i].pid);
        log_debug(logger_memoria, "for PID %d no encontrado", memoriaDelSistema->procesos[i].cant_instrucciones);
        log_debug(logger_memoria, "for PID %d no encontrado", memoriaDelSistema->cant_procesos);
        log_debug(logger_memoria, "for PID %s no encontrado", memoriaDelSistema->procesos[i].instrucciones[pc]);
    }
    log_error(logger_memoria, "PID %d no encontrado", pid);
    log_error(logger_memoria, "cant procesos: %d", memoriaDelSistema->cant_procesos);
    return NULL; // PID no encontrado
}