#ifndef MODULO_MEMORIA_H_
#define MODULO_MEMORIA_H_

    #include <./utils_kernel/kernel-structs-enums.h>

    t_respuesta_dump* recibir_respuesta_dump(t_buffer* buffer);

    /**
	* @brief envia el tamanio del proceso a la memoria 
	* @param tam_proceso tamanio del proceso a serializar
	* @param socket_cliente socket de a quien le enviamos el paquete serializado
	*/
	void enviar_tamanio_proceso(char* tam_proceso, int socket_cliente);

	void enviar_a_liberar_memoria(int socket_memoria, t_pcb proceso);

	void _avisar_kernel_a_memoria(int socket_memoria);

#endif // MODULO_MEMORIA_H_