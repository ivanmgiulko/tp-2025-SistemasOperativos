#ifndef MODULO_IO_H_
#define MODULO_IO_H_

    #include <./utils_kernel/kernel-structs-enums.h>

    /**
	* @brief envia un proceso al modulo de IO para que se bloquee
	* @param uint8_t PID enviado al modulo para que se bloquee
	* @param int64_t tiempo que el proceso va a estar bloqueado
	* @param int socket de conexion IO con Kernel
	*/
	void enviar_proceso_a_io_para_bloqueo(uint8_t pid, int64_t tiempo, int socket_cliente);

    /**
	* @brief recibe un proceso que fue desbloqueado desde el modulo de IO
	* @param t_buffer buffer donde esta contenido el PID del proceso desbloqueado
	* @return el PID dle proceso recien desbloqueado
	*/
	uint8_t _recibir_proceso_bloqueado(t_buffer* buffer);
	

#endif // MODULO_IO_H_