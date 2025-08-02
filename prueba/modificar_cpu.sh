#!/bin/bash

# Verificación de argumentos
if [ "$#" -ne 5 ]; then
  echo "Uso: $0 <IP_MEMORIA> <PUERTO_MEMORIA> <IP_KERNEL> <PUERTO_DISPATCH> <PUERTO_INTERRUPT>"
  exit 1
fi

# Nuevos valores desde argumentos
IP_MEMORIA=$1
PUERTO_MEMORIA=$2
IP_KERNEL=$3
PUERTO_DISPATCH=$4
PUERTO_INTERRUPT=$5

# Directorios a procesar
carpetas=(
  "prueba_cache"
  "prueba_cortoplazo"
  "prueba_general"
  "prueba_medianolargoplazo"
  "prueba_swap"
  "prueba_tlb"
)

# Archivos CPU posibles
archivos_cpu=("cpu1.config" "cpu2.config" "cpu3.config" "cpu4.config")

# Iterar por cada carpeta
for carpeta in "${carpetas[@]}"; do
  for archivo_cpu in "${archivos_cpu[@]}"; do
    CONFIG_FILE="$carpeta/$archivo_cpu"

    if [ -f "$CONFIG_FILE" ]; then
      echo "Modificando $CONFIG_FILE..."

      sed -i "s/^IP_MEMORIA=.*/IP_MEMORIA=$IP_MEMORIA/" "$CONFIG_FILE"
      sed -i "s/^PUERTO_MEMORIA=.*/PUERTO_MEMORIA=$PUERTO_MEMORIA/" "$CONFIG_FILE"
      sed -i "s/^IP_KERNEL=.*/IP_KERNEL=$IP_KERNEL/" "$CONFIG_FILE"
      sed -i "s/^PUERTO_KERNEL_DISPATCH=.*/PUERTO_KERNEL_DISPATCH=$PUERTO_DISPATCH/" "$CONFIG_FILE"
      sed -i "s/^PUERTO_KERNEL_INTERRUPT=.*/PUERTO_KERNEL_INTERRUPT=$PUERTO_INTERRUPT/" "$CONFIG_FILE"

      echo "Archivo $CONFIG_FILE modificado."
    else
      echo "Archivo $CONFIG_FILE no encontrado. Saltando..."
    fi
  done
done

echo "Modificación completada en los archivos cpuX.config existentes."
