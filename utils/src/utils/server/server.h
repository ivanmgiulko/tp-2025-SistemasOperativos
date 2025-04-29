#ifndef SERVER_H_
#define SERVER_H_

#include "./utils/utils.h"

int iniciar_servidor(char*, t_log*);
int esperar_cliente(int, t_log*);
void iterator(char*, t_log*);

#endif // SERVER_H_