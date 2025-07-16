#ifndef CPU_K_INTERRUPT_H_
#define CPU_K_INTERRUPT_H_
#include "cpu-gestor.h"
#include "cpu-utils.h"
int manejar_conexion_kernel_interrupt();

void enviar_proceso_desalojado(int socket_servidor, uint8_t pid, uint16_t pc);

#endif // CPU_K_INTERRUPT_H_