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

	while (1) {
		t_paquete* paquete = crear_paquete();
		paquete->codigo_operacion = recibir_cod_operacion(socket_cliente_io);
		switch (paquete->codigo_operacion) {
		case MENSAJE:
			recibir_mensaje(socket_cliente_io, logger_kernel);
			free(paquete);
			break;

		case PROCESO_DESBLOQUEADO:

			recibir_pid(socket_cliente_io, paquete);

			uint8_t pid_desbloqueado = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "%d finalizó IO y pasa a READY", pid_desbloqueado);
			
			t_pcb* _proceso_desbloqueado = _sacar_pcb_de_cola(pid_desbloqueado, estado_blocked_aux);

			pthread_mutex_lock(&lista_de_io->mutex_lista);
			_proceso_desbloqueado->datos_io->instancia_utilizada->pid = -1;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			pasar_pcb_blocked_a_ready(_proceso_desbloqueado);
			eliminar_paquete(paquete);
			break;

		case PROCESO_SUSPENDIDO_DESBLOQUEADO:

			
			recibir_pid(socket_cliente_io, paquete);
			uint8_t pid_desbloqueado_susp = _recibir_proceso_bloqueado(paquete->buffer);
			log_info(logger_kernel, "%d finalizó IO y pasa a READY", pid_desbloqueado_susp);

			t_pcb* _proceso_desbloqueado_suspendido = _sacar_pcb_de_cola(pid_desbloqueado_susp, estado_blocked_aux);
			
			pthread_mutex_lock(&lista_de_io->mutex_lista);
			_proceso_desbloqueado_suspendido->datos_io->instancia_utilizada->pid = -1;
			pthread_mutex_unlock(&lista_de_io->mutex_lista);

			pasar_pcb_suspblocked_a_suspready(_proceso_desbloqueado_suspendido);
			eliminar_paquete(paquete);
			break;
	
		case -1:

			pthread_mutex_lock(&(lista_de_io->mutex_lista));
			t_io* interfaz_de_instancia_finalizada = buscar_io_en_lista(lista_de_io->lista_ios, socket_cliente_io);
			pthread_mutex_unlock(&(lista_de_io->mutex_lista));

			pthread_mutex_lock(&(lista_de_io->mutex_lista));
			t_instancia_io* instancia_a_eliminar = eliminar_y_devolver_instancia(interfaz_de_instancia_finalizada->instancias, socket_cliente_io);
			pthread_mutex_unlock(&(lista_de_io->mutex_lista));

			if(instancia_a_eliminar->pid != -1) {

				log_error(logger_kernel, "Se elimino una instancia de [%s]", interfaz_de_instancia_finalizada->nombre);

				t_pcb* proceso_a_eliminar = _sacar_pcb_de_cola(instancia_a_eliminar->pid, estado_blocked_aux);

				pthread_mutex_lock(&(lista_de_io->mutex_lista));
				eliminar_proceso_de_io(interfaz_de_instancia_finalizada->procesos, proceso_a_eliminar->pid);
				pthread_mutex_unlock(&(lista_de_io->mutex_lista));
				
				pasar_pcb_blocked_a_exit(proceso_a_eliminar);

				pthread_mutex_lock(&(lista_de_io->mutex_lista));
				free(instancia_a_eliminar);
				pthread_mutex_unlock(&(lista_de_io->mutex_lista));

				if(list_is_empty(interfaz_de_instancia_finalizada->instancias)) {
				
					for (int i = 0; i < list_size(interfaz_de_instancia_finalizada->procesos); i++) 
					{
						pthread_mutex_lock(&(lista_de_io->mutex_lista));
						proceso_a_eliminar = list_get(interfaz_de_instancia_finalizada->procesos, i);
						pthread_mutex_unlock(&(lista_de_io->mutex_lista));

						_sacar_pcb_de_cola(proceso_a_eliminar->pid, estado_blocked_aux);

						pthread_mutex_lock(&(lista_de_io->mutex_lista));
						eliminar_proceso_de_io(interfaz_de_instancia_finalizada->procesos, proceso_a_eliminar->pid);
						pthread_mutex_unlock(&(lista_de_io->mutex_lista));

						pasar_pcb_blocked_a_exit(proceso_a_eliminar);
					}

					pthread_mutex_lock(&(lista_de_io->mutex_lista));
					interfaz_de_instancia_finalizada->socket = -1;
					pthread_mutex_unlock(&(lista_de_io->mutex_lista));

					pthread_mutex_lock(&(lista_de_io->mutex_lista));
					eliminar_interfaz(interfaz_de_instancia_finalizada);
					pthread_mutex_unlock(&(lista_de_io->mutex_lista));
				} 

			} else {

				log_error(logger_kernel, "La interfaz que se elimino no tenia ningun proceso...");

				pthread_mutex_lock(&(lista_de_io->mutex_lista));
				eliminar_interfaz(interfaz_de_instancia_finalizada);
				pthread_mutex_unlock(&(lista_de_io->mutex_lista));
			}

			sem_post(&bin_eliminar_procesos_en_interfaces);

			eliminar_paquete(paquete);
			return EXIT_FAILURE;
		default:
			log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
			eliminar_paquete(paquete);
			break;
		}
		
	}

	pthread_exit(NULL);
	free(nombre_io);
	free(stream);
	close(socket_cliente_io);
	return EXIT_SUCCESS;
}