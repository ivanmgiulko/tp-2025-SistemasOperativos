#ifndef CAMBIO_ESTADO_PROCESO_H_
#define CAMBIO_ESTADO_PROCESO_H_

    #include <./utils_kernel/kernel-structs-enums.h>

   /**
	* @brief pasa el proceso que le pasemos por parametro del estado NEW a READY
    * @param t_pcb* proceso que va a ser pasado a READY
	*/
    void pasar_pcb_new_a_ready(t_pcb* pcb);

    /**
	* @brief pasa el proceso que le pasemos por parametro del estado READY a EXEC
    * @param t_pcb* proceso que va a ser pasado a EXEC
	*/
    void pasar_pcb_ready_a_exec(t_pcb* pcb);

    void pasar_pcb_blocked_a_ready(t_pcb* pcb);

    void pasar_pcb_exec_a_ready(t_pcb* pcb);

    void pasar_pcb_susp_ready_a_ready(t_pcb* pcb);

    void pasar_pcb_blocked_a_suspblocked(t_pcb* pcb);

    void pasar_pcb_suspblocked_a_suspready(t_pcb* pcb);

    void pasar_pcb_blocked_a_exit(t_pcb* pcb);

    void pasar_pcb_suspblocked_a_exit(t_pcb* pcb);

    void pasar_de_exec_a_blocked(t_pcb* pcb);
    
    void pasar_de_exec_a_exit(t_pcb* pcb);

    t_pcb* _sacar_pcb_de_cola(uint8_t pid, t_estado* estado);

    void _enviar_a_finalizar_proceso(t_pcb* proceso_a_finalizar);

    uint64_t calcular_estimacion_actual(int64_t rafagas_hechas, uint64_t estimacion_anterior);

#endif // CAMBIO_ESTADO_PROCESO_H_