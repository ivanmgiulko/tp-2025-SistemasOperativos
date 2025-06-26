#include "cpu.h"
int main(int argc, char* argv[]){
	if(argc<2){
		fprintf(stderr, "Falta ID del CPU\n");
		return EXIT_FAILURE;
	}
	char* cpu_id = argv[1];

	t_config* config_cpu = iniciar_config("./cpu.config");
	char* puerto_kernel_interrupt = config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
	char* puerto_kernel_dispatch = config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH");
    char* ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
	char* puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    char* ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
	uint32_t entradas_cache = atoi(config_get_string_value(config_cpu,"ENTRADAS_CACHE"));
	uint32_t retardo_cache = atoi(config_get_string_value(config_cpu, "RETARDO_CACHE"));
    uint32_t maximas_entradas_tlb = atoi(config_get_string_value(config_cpu, "ENTRADAS_TLB"));
	char* algoritmo_reemplazo_tlb = config_get_string_value(config_cpu, "REEMPLAZO_TLB");
	algoritmo = algoritmo_from_string(algoritmo_reemplazo_tlb);

	/* ---------------- LOGGING ---------------- */
	char *directorioLogger = string_new(); // Gloria eterna al creador de las commons
	string_append(&directorioLogger,"cpu-");
	string_append(&directorioLogger, cpu_id);
		string_append(&directorioLogger,".log");

	logger_cpu = log_create(directorioLogger, "log", true, LOG_LEVEL_TRACE);
	log_info(logger_cpu, "%s", directorioLogger);
    // log_info(logger_cpu, "Primer log");
	free(directorioLogger);
	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	log_info(logger_cpu, "ID CPU: %s", cpu_id);
	log_info(logger_cpu, "IP Kernel: %s", ip_kernel);
	log_info(logger_cpu, "Puerto Kernel Interrupt: %s",puerto_kernel_interrupt);
	log_info(logger_cpu, "Puerto Kernel Dispatch: %s", puerto_kernel_dispatch);
	log_info(logger_cpu, "IP Memoria: %s", ip_memoria);
	log_info(logger_cpu, "Puerto Memoria: %s", puerto_memoria);
	mmu = inicializar_mmu();	
	memoria_cache = inicializar_cache(entradas_cache, mmu->tamanio_pagina);
	tlb = inicializar_tlb(maximas_entradas_tlb);

	//	IP_MEMORIA=127.0.0.4
	//	PUERTO_MEMORIA=40074
    // CREAMOS CONEXIONES DE DISPATCH E INTERRUPT HACIA EL KERNEL y asia memoria
	
	_crear_conexion_kernel_interrupt(ip_kernel, puerto_kernel_interrupt, cpu_id);
	
	_crear_conexion_kernel_dispatch(ip_kernel, puerto_kernel_dispatch, cpu_id);

	_crear_conexion_cpu_memoria(ip_memoria, puerto_memoria);

	pcb_actual = malloc(sizeof(t_peticion_instruccion));
	pcb_actual->pid = 0;
	pcb_actual->pc = 0;

	//este post tendria que estar cuando se recibe un proceso y cada vez que se tenga que pedir otra instruccion a memoria.
	sem_init(&sem_cpu,0,0);
	sem_init(&sem_cpu_kernel,0,1);
	sem_init(&sem_read,0,0);
	sem_init(&sem_write,0,0);
	pthread_mutex_init(&mutex_cpu, NULL);
	//ahora esta asi para que solo se ejecute una vez y no afecte en la ejecucion del resto.
	
	pthread_t hilo_cliente_cpuInt_akernel;
    pthread_create(&hilo_cliente_cpuInt_akernel, NULL, (void*)manejar_conexion_kernel_interrupt, NULL);
    pthread_detach(hilo_cliente_cpuInt_akernel);
	
	pthread_t hilo_cliente_cputDispatch_akernel;
    pthread_create(&hilo_cliente_cputDispatch_akernel, NULL, (void*)manejar_conexion_kernel_dispatch, NULL);
    pthread_detach(hilo_cliente_cputDispatch_akernel);
	
	pthread_t hilo_cliente_cpu_amemoria;
    pthread_create(&hilo_cliente_cpu_amemoria, NULL, (void*)manejar_conexion_memoria, NULL);
	pthread_join(hilo_cliente_cpu_amemoria, NULL);



	//Libera config y logger,y otros
	destruir_tlb(tlb);
	destruir_cache(memoria_cache, mmu->tamanio_pagina);
	destruir_mmu(mmu);
	config_destroy(config_cpu);
	log_destroy(logger_cpu);

    return 0;
}
