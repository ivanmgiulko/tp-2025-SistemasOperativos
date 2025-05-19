#include "memoria-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
t_memoria_del_sistema crear_memoria_del_sistema() {
    t_memoria_del_sistema memoria;
    memoria.procesos = malloc(sizeof(t_proceso_en_memoria)); 
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
    char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    
    if (!instrucciones){
        log_error(logger_memoria, "Error al cargar instrucciones, falla al crear proceso");
        return;
    } 

    memoria_del_sistema->procesos = realloc(memoria_del_sistema->procesos, (memoria_del_sistema->cant_procesos + 1) * sizeof(t_proceso_en_memoria));

    t_proceso_en_memoria nuevoProceso;
    nuevoProceso.pid = pcb->pid;
    nuevoProceso.instrucciones = instrucciones;
    nuevoProceso.cant_instrucciones = cant_inst;

    memoria_del_sistema->procesos[memoria_del_sistema->cant_procesos] = nuevoProceso;
    memoria_del_sistema->cant_procesos++;
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
        //printf("No se encontró el proceso con PID %d\n", pid);
        return -1;
    }

    // Liberar instrucciones del proceso
    for (int j = 0; j < memoria_del_sistema->procesos[encontrado].cant_instrucciones; j++) {
        free(memoria_del_sistema->procesos[encontrado].instrucciones[j]);
    }
    free(memoria_del_sistema->procesos[encontrado].instrucciones);

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
    for (int i = 0; i < memoria_del_sistema->cant_procesos; i++) {
        if (memoria_del_sistema->procesos[i].pid == pid) {
            log_trace(logger_memoria, "Instrucción solicitada: PID %d, PC %d", pid, pc);
            if (pc < memoria_del_sistema->procesos[i].cant_instrucciones) {
                return memoria_del_sistema->procesos[i].instrucciones[pc];
            } else {  
                //Si el pc es mayor o igual a la cant de instrucciones:
                return "PC FINALIZADO"; 
            }            
        }
    }
    
    log_error(logger_memoria, "PID %d no encontrado", pid);
    log_error(logger_memoria, "cant procesos: %d", memoria_del_sistema->cant_procesos);
    return NULL; // PID no encontrado
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