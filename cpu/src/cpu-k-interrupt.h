#ifndef CPU_K_INTERRUPT_H_
#define CPU_K_INTERRUPT_H_
#include "cpu-gestor.h"
#include "cpu-utils.h"
int manejar_conexion_kernel_interrupt();

void enviar_proceso_desalojado(int socket_servidor, int pid, int pc);

#endif // CPU_K_INTERRUPT_H_