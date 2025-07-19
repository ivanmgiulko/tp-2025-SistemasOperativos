#ifndef UTILS_KERNEL_CPU_H_
#define UTILS_KERNEL_CPU_H_

    #include "kernel-gestor.h"

    void _agregar_socket_en_cpu(uint8_t, t_sockets_cpu, int);

	uint8_t _recibir_handshake_de_cpu(int , int );

	t_cpu_conectada* _agregar_cpu_en_lista(uint8_t );

	void liberar_cpu_de_proceso(uint8_t pid);


#endif // UTILS_KERNEL_CPU_H_