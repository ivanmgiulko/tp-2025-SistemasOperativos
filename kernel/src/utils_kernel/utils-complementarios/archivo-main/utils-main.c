#include "utils-main.h"
void _chequear_segundo_argumento_es_numero(char* numero_en_string) 
{
    int longitud_string = string_length(numero_en_string);

    int num_prueba = atoi(numero_en_string);
    char* num_prueba_en_string = string_itoa(num_prueba);
    int num_prueba_en_string_length = string_length(num_prueba_en_string);
    free(num_prueba_en_string);
    
    if(!(longitud_string == num_prueba_en_string_length)) { 
        fprintf(stderr, "Bro realmente puso mal el tamanio en memoria \n");
        abort();
    }
}
