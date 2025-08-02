#include "kernel-gestor.h"

#include <utils_kernel/funciones-thread-safe/busqueda-de-struct/busqueda-de-structs.h>
#include <utils_kernel/funciones-thread-safe/cambio-de-estado/cambio-estado-proceso.h>
#include <utils_kernel/kernel-de-serializaciones/conexion-con-io/modulo-io.h>
#include <utils_kernel/manejar-conexiones/modulo-io/manejar-conexion-io.h>
#include <utils_kernel/utils-complementarios/conexion-con-io/utils-kernel-io.h>

t_lista_io* lista_de_io = NULL;

uint8_t* pop_pid_esperando_io(t_io* io) {
    pthread_mutex_lock(&(io->mutex_lista));
    uint8_t* pid = list_remove(io->procesos, 0);
    pthread_mutex_unlock(&(io->mutex_lista));
    return pid;
}

void enviar_proceso_en_cola_io(t_io* io_a_encolar_instancia, t_instancia_io* instancia_io) {
	pthread_mutex_lock(&instancia_io->mutex_instancia);

	pthread_mutex_lock(&io_a_encolar_instancia->mutex_lista);
	int cantidad_procesos_en_espera = list_size(io_a_encolar_instancia->procesos);
	pthread_mutex_unlock(&io_a_encolar_instancia->mutex_lista);

	if(cantidad_procesos_en_espera != 0) {
		
		uint8_t* pid_proceso_en_cola = pop_pid_esperando_io(io_a_encolar_instancia);
		
		t_pcb* proceso_en_cola = buscar_proceso_en_cola(estado_blocked, *pid_proceso_en_cola);

		if(proceso_en_cola == NULL) proceso_en_cola = buscar_proceso_en_cola(estado_susp_blocked, *pid_proceso_en_cola);
		
		instancia_io->pid = *pid_proceso_en_cola;
		proceso_en_cola->datos_io->instancia_utilizada = instancia_io;
		enviar_proceso_a_io_para_bloqueo(*pid_proceso_en_cola,proceso_en_cola->datos_io->tiempo, instancia_io->socket_io);
		
		log_info(logger_kernel, "Se envio el proceso [%d] a la interfaz [%s]", *pid_proceso_en_cola, io_a_encolar_instancia->nombre);
	}

	pthread_mutex_unlock(&instancia_io->mutex_instancia);
}

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

	// Busco si existe IO del mismo tipo en la lista de IOs
	
	t_io* io_a_encolar_instancia = buscar_io(nombre_io);
	
	if(io_a_encolar_instancia == NULL) io_a_encolar_instancia = inicializar_io(nombre_io, socket_cliente_io);

	t_instancia_io* instancia_io = insertar_nueva_instancia_io(io_a_encolar_instancia, socket_cliente_io);

	
	enviar_proceso_en_cola_io(io_a_encolar_instancia, instancia_io);

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
		
			t_pcb* _proceso_desbloqueado = _sacar_pcb_de_cola(pid_desbloqueado, estado_blocked);
			
			if(_proceso_desbloqueado == NULL) {
				_proceso_desbloqueado = _sacar_pcb_de_cola(pid_desbloqueado,estado_susp_blocked);
				pasar_pcb_suspblocked_a_suspready(_proceso_desbloqueado);
			}else{
				log_info(logger_kernel, "%d finalizó IO y pasa a READY", pid_desbloqueado);
				pasar_pcb_blocked_a_ready(_proceso_desbloqueado);
			}
			
			pthread_mutex_lock(&_proceso_desbloqueado->datos_io->instancia_utilizada->mutex_instancia);
			_proceso_desbloqueado->datos_io->instancia_utilizada->pid = -1;
			pthread_mutex_unlock(&_proceso_desbloqueado->datos_io->instancia_utilizada->mutex_instancia);

			enviar_proceso_en_cola_io(io_a_encolar_instancia, instancia_io);
			eliminar_paquete(paquete);
			break;

		case -1:
			// Desconexion de instancia IO
			// Tomamos la interfaz que contiene a la instancia desconectada
			t_io* interfaz_de_instancia_finalizada = buscar_io_en_lista(socket_cliente_io);
	
			// Tomamos la instancia desconectada y que debemos eliminar
			t_instancia_io* instancia_a_eliminar = eliminar_y_devolver_instancia(interfaz_de_instancia_finalizada, socket_cliente_io);
	
			// Chequeamos si la instancia a eliminar tenia un proceso ejecutando en ella
			if(instancia_a_eliminar->pid != -1) {
				// Si tenia un proceso ejecutando lo pasamos a EXIT
				
				t_pcb* proceso_a_eliminar = buscar_proceso_en_cola(estado_blocked, instancia_a_eliminar->pid);

				if(proceso_a_eliminar == NULL){
					proceso_a_eliminar = buscar_proceso_en_cola(estado_susp_blocked, instancia_a_eliminar->pid);	
					_sacar_pcb_de_cola(instancia_a_eliminar->pid,estado_susp_blocked);
				}else{
					_sacar_pcb_de_cola(instancia_a_eliminar->pid,estado_blocked);
				}
				
				pasar_pcb_blocked_a_exit(proceso_a_eliminar);
			}
	
			pthread_mutex_lock(&(interfaz_de_instancia_finalizada->mutex_lista));
			free(instancia_a_eliminar);
			bool era_ultima_instancia = list_is_empty(interfaz_de_instancia_finalizada->instancias);
			pthread_mutex_unlock(&(interfaz_de_instancia_finalizada->mutex_lista));

			// Ahora chequeamos si la instancia que eliminamos era la ultima de la interfaz
			if(era_ultima_instancia) {

				pthread_mutex_lock(&(lista_de_io->mutex_lista));
				list_remove_element(lista_de_io->lista_ios, interfaz_de_instancia_finalizada);
				pthread_mutex_unlock(&(lista_de_io->mutex_lista));

				// Si era la ultima entonces agarramos todos los procesos que estaban esperando para usar la interfaz y los pasamos a exit
				pthread_mutex_lock(&(interfaz_de_instancia_finalizada->mutex_lista));
				int cantidad_procesos_en_espera = list_size(interfaz_de_instancia_finalizada->procesos);
				pthread_mutex_unlock(&(interfaz_de_instancia_finalizada->mutex_lista));
				
				for (int i = 0; i < cantidad_procesos_en_espera; i++) {
					uint8_t* pid_proceso_en_cola = pop_pid_esperando_io(interfaz_de_instancia_finalizada);
		
					t_pcb* proceso_en_cola = buscar_proceso_en_cola(estado_blocked, *pid_proceso_en_cola);
					
					if(proceso_en_cola == NULL){
						proceso_en_cola = buscar_proceso_en_cola(estado_susp_blocked, instancia_a_eliminar->pid);	
						_sacar_pcb_de_cola(instancia_a_eliminar->pid,estado_susp_blocked);
					}else{
						_sacar_pcb_de_cola(instancia_a_eliminar->pid,estado_blocked);
					}
				
					eliminar_proceso_de_io(interfaz_de_instancia_finalizada, proceso_en_cola->pid);
					pasar_pcb_blocked_a_exit(proceso_en_cola);
				}
				// Finalmente eliminamos la interfaz de IO
				eliminar_interfaz(interfaz_de_instancia_finalizada);
			} 
			 
			eliminar_paquete(paquete);
		
			return EXIT_FAILURE;
			break;
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
