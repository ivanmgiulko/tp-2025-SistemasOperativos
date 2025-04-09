#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include<stdlib.h>
#include<stdio.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
void saludar(char* quien);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);


#endif
