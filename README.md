# Trabajo Práctico Sistemas Operativos: Episodio III: Revenge of the Cth

Este proyecto, realizado por Iván Giulkovich, Luciano Lobo, Leandro Calviño, Facundo Vinitzca y Thiago Santero, busca simular un sistema operativo distribuido, desarrollado en C.
Fue realizado el primer cuatrimestre del año 2025 para la materia: "Sistemas Operativos" de la carrera de Ingeniería en Sistemas de Información en la UTN.


## Objetivo del TP

Los objetivos del proyecto eran los siguientes:

- Entender como funciona un sistema operativo por dentro.
- Aplicar conceptos como planificación de procesos, sincronización(semaforos) y algoritmos de reemplazo.
- Dominar el lenguaje C
- Adquirir experiencia en la programación en entorno LINUX.


## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un archivo makefile. Para compilar un módulo, es necesario ejecutar el comando make desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta bin del módulo. Ejemplo:
```
sh cd kernel make ./bin/kernel
```


## Enunciado

Si querés profundizar más en los requerimientos y requisitos del TP, podés acceder al [enunciado](https://docs.google.com/document/d/1HC9Zi-kpn8jI_egJGEZe77wUCbSkwSw9Ygqqs7m_-is/edit?tab=t.0) donde se explica cada componente y las funcionalidades pedidas de los mismos.


## Tecnologías Utilizadas

+ Sockets para las comunicaciones entre los diferentes módulos.
+ Algoritmos de planificacion.
+ Tests para verificar el comportamiento del sistema.
+ Concurrencia y sincronización.
+ Archivos, logs, scripts.
+ Linux - Maquina virtual.


## Consultas o Contactos
Por cualquier consulta, por favor contactar a cualquiera de los integrantes:

Luciano Lobo: [Github](https://github.com/LoboLuciano) - [Linkedin](https://www.linkedin.com/in/luciano-lobo-81b0b4212/) (Kernel & IO)
Facundo Vinitzca: [Github](https://github.com/ViniBitCode) - [Linkedin](https://www.linkedin.com/in/facundo-hern%C3%A1n-vinitzca-a2bb38284/) (Kernel & IO)
Leandro Calviño: [Github](https://github.com/LeandroCalvinoGITutn) - [Linkedin](https://www.linkedin.com/in/leandro-calvino/) (Memoria & CPU)
Iván Giulkovich: [Github](https://github.com/ivanmgiulko) - [Linkedin](https://www.linkedin.com/in/ivan-giulkovich-a561b9306/) (CPU & Memoria)
Thiago Santero: [Github](https://github.com/tsantero22) - [Linkedin](https://www.linkedin.com/in/thiago-santero-94bb6b1b6/) (Memoria & CPU)


