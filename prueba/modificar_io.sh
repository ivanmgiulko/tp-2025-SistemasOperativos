#!/bin/bash

# Verificación de argumentos
if [ "$#" -ne 2 ]; then
  echo "Uso: $0 <IP_KERNEL> <PUERTO_KERNEL>"
  exit 1
fi

# Nuevos valores desde argumentos
NUEVA_IP_KERNEL=$1
NUEVO_PUERTO_KERNEL=$2

# Directorios a procesar
carpetas=(
  "prueba_cache"
  "prueba_cortoplazo"
  "prueba_general"
  "prueba_medianolargoplazo"
  "prueba_swap"
  "prueba_tlb"
)

# Iterar por cada carpeta y modificar el archivo io.config
for carpeta in "${carpetas[@]}"; do
  CONFIG_FILE="$carpeta/io.config"

  if [ -f "$CONFIG_FILE" ]; then
    echo "Modificando $CONFIG_FILE..."

    sed -i "s/^IP_KERNEL=.*/IP_KERNEL=$NUEVA_IP_KERNEL/" "$CONFIG_FILE"
    sed -i "s/^PUERTO_KERNEL=.*/PUERTO_KERNEL=$NUEVO_PUERTO_KERNEL/" "$CONFIG_FILE"

    echo "Archivo $CONFIG_FILE modificado."
  else
    echo "Archivo $CONFIG_FILE no encontrado. Saltando..."
  fi
done

echo "Modificación completada en todas las carpetas."
