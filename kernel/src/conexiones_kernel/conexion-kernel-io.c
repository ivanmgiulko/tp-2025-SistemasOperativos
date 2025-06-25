#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/manejar-conexiones/modulo-io/manejar-conexion-io.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>

t_lista_io* lista_de_io = NULL;

int manejar_cliente_io(void* socket_cliente_ptr){
    int socket_cliente_io = *(int*)socket_cliente_ptr;
	size_t tamanio_interfaz;
	int32_t resultado_handshake_exitoso = 1;
	if(recv(socket_cliente_io, &tamanio_interfaz, sizeof(size_t), 0) != sizeof(size_t)){
		log_error(logger_kernel, "Error al recibir el tamanio de la interfaz");
		return EXIT_FAILURE;
	}

	void* stream = malloc(tamanio_interfaz);
	if(recv(socket_cliente_io, stream, tamanio_interfaz, 0) != tamanio_interfaz){
		log_error(logger_kernel, "Error al recibir el nombre de la interfaz");
		return EXIT_FAILURE;
	}

	// Enviar respuesta al cliente
	send(socket_cliente_io, &resultado_handshake_exitoso, sizeof(int32_t), 0);

	char* nombre_io = malloc(tamanio_interfaz + 1);
	memcpy(nombre_io, stream, tamanio_interfaz);
	nombre_io[tamanio_interfaz] = '\0';  // Asegurarse de que termine en \0

	// Si la lista de interfaces ya fue iniciada, no se crea de nuevo
	if(lista_de_io == NULL){
		inicializar_lista_io();
	}

	// Busco si existe IO del mismo tipo en la lista de IOs
	t_io* io_a_encolar_instancia = buscar_io(lista_de_io->lista_ios, nombre_io);

	if(io_a_encolar_instancia == NULL) {
		io_a_encolar_instancia = inicializar_io(nombre_io, socket_cliente_io);
	}

	insertar_nueva_instancia_io(io_a_encolar_instancia->instancias, socket_cliente_io);

	free(stream);

	while (1) {
		t_paquete* paquete = malloc(sizeof(t_paquete));
		crear_buffer(paquete);
		paquete->codigo_operacion = recibir_operacion(socket_cliente_io);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente_io, logger_kernel);
			free(paquete);
			break;

		case PROCESO_DESBLOQUEADO:

			recibir_paquete(socket_cliente_io, paquete);

			uint8_t pid_desbloqueado = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "%d finalizó IO y pasa a READY", pid_desbloqueado);
			
			t_pcb* _proceso_desbloqueado = _sacar_pcb_de_cola(pid_desbloqueado, estado_blocked_aux);

			pthread_mutex_lock(&lista_de_io->mutex_lista);
			_proceso_desbloqueado->datos_io->instancia_utilizada->pid = -1;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			pasar_pcb_blocked_a_ready(_proceso_desbloqueado);

			break;

		case PROCESO_SUSPENDIDO_DESBLOQUEADO:

			recibir_paquete(socket_cliente_io, paquete);

			uint8_t pid_desbloqueado_susp = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "%d finalizó IO y pasa a READY", pid_desbloqueado_susp);

			t_pcb* _proceso_desbloqueado_suspendido = _sacar_pcb_de_cola(pid_desbloqueado_susp, estado_blocked_aux);
			
			pthread_mutex_lock(&lista_de_io->mutex_lista);
			_proceso_desbloqueado_suspendido->datos_io->instancia_utilizada->pid = -1;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			
			pasar_pcb_suspblocked_a_suspready(_proceso_desbloqueado_suspendido);

			break;
	
		case -1:

			log_error(logger_kernel, "el cliente [%d] se desconecto", socket_cliente_io);
			eliminar_paquete(paquete);
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			eliminar_paquete(paquete);
			break;
		}
	}

	pthread_exit(NULL);
	close(socket_cliente_io);
	return EXIT_SUCCESS;
}