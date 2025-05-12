#include "memoria-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header
t_memoriaDelSistema crear_memoria_del_sistema() {
    t_memoriaDelSistema memoria;
    memoria.procesos = NULL;
    memoria.cant_procesos = 0;
    return memoria;
}
char** leer_instrucciones(char* pathArchivoPseudocodigo, int* cantidad) {
    log_debug(logger_memoria, "Leyendo instrucciones desde el archivo: %s", pathArchivoPseudocodigo);
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
    char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    if (!instrucciones) return;

    memoriaDelSistema->procesos = realloc(memoriaDelSistema->procesos, (memoriaDelSistema->cant_procesos + 1) * sizeof(t_procesoEnMemoria));

    t_procesoEnMemoria nuevoProceso;
    nuevoProceso.pid = pcb->pid;
    nuevoProceso.instrucciones = instrucciones;
    nuevoProceso.cant_instrucciones = cant_inst;

    memoriaDelSistema->procesos[memoriaDelSistema->cant_procesos] = nuevoProceso;
    memoriaDelSistema->cant_procesos++;
}

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
    if(pc <0) return NULL; // PC inválido
    for (int i = 0; i < memoriaDelSistema->cant_procesos; i++) {
        if (memoriaDelSistema->procesos[i].pid == pid) {
            if (pc < memoriaDelSistema->procesos[i].cant_instrucciones) {
                return memoriaDelSistema->procesos[i].instrucciones[pc];
            } else {
                return NULL; // PC inválido
            }
        }
    }
    return NULL; // PID no encontrado
}