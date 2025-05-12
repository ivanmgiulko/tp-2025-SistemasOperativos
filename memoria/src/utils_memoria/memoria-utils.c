#include"memoria-utils.h"
// Aca desarrollamos el cuerpo de las funciones que tenemos en el Header

char** leer_instrucciones(const char* pathArchivoPseudocodigo, int* cantidad) {
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

void agregar_proceso(t_memoriaDelSistema* memoria, t_pcbMemoria* pcb) {
    int cant_inst = 0;
    char** instrucciones = leer_instrucciones(pcb->pathArchivoPseudocodigo, &cant_inst);
    if (!instrucciones) return;

    memoria->procesos = realloc(memoria->procesos, (memoria->cant_procesos + 1) * sizeof(Proceso));

    t_procesoEnMemoria nuevoProceso;
    nuevoProceso.pid = pcb->pid;
    nuevoProceso.instrucciones = instrucciones;
    nuevoProceso.cant_instrucciones = cant_inst;

    memoria->procesos[memoria->cant_procesos] = nuevoProceso;
    memoria->cant_procesos++;
}

int finalizar_proceso(t_memoriaDelSistema* memoria, int pid) {
    int encontrado = -1;

    // Buscar el proceso por PID
    for (int i = 0; i < memoria->cant_procesos; i++) {
        if (memoria->procesos[i].pid == pid) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        //printf("No se encontró el proceso con PID %d\n", pid);
        return -1;
    }

    // Liberar instrucciones del proceso
    for (int j = 0; j < memoria->procesos[encontrado].cant_instrucciones; j++) {
        free(memoria->procesos[encontrado].instrucciones[j]);
    }
    free(memoria->procesos[encontrado].instrucciones);

    // Desplazar los procesos siguientes para llenar el hueco
    for (int i = encontrado; i < memoria->cant_procesos - 1; i++) {
        memoria->procesos[i] = memoria->procesos[i + 1];
    }

    memoria->cant_procesos--;

    // Redimensionar el array de procesos si hay procesos restantes
    if (memoria->cant_procesos > 0) {
        memoria->procesos = realloc(memoria->procesos, memoria->cant_procesos * sizeof(Proceso));
    } else {
        free(memoria->procesos);
        memoria->procesos = NULL;
    }

    //printf("Proceso con PID %d liberado correctamente.\n", pid);
    return pid;
}

char* obtener_instruccion(t_memoriaDelSistema* memoria, int pid, int pc) {
    if(pc <0) return NULL; // PC inválido
    for (int i = 0; i < memoria->cant_procesos; i++) {
        if (memoria->procesos[i].pid == pid) {
            if (pc < memoria->procesos[i].cant_instrucciones) {
                return memoria->procesos[i].instrucciones[pc];
            } else {
                return NULL; // PC inválido
            }
        }
    }
    return NULL; // PID no encontrado
}