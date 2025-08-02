#!/bin/bash

# Verificación de argumentos
if [ "$#" -ne 5 ]; then
  echo "Uso: $0 <IP_MEMORIA> <PUERTO_MEMORIA> <DISPATCH> <INTERRUPT> <IO>"
  exit 1
fi

# Nuevos valores desde argumentos
NUEVA_IP_MEMORIA=$1
NUEVO_PUERTO_MEMORIA=$2
NUEVO_PUERTO_DISPATCH=$3
NUEVO_PUERTO_INTERRUPT=$4
NUEVO_PUERTO_IO=$5

# Directorios a procesar (podés agregar más si tenés)
carpetas=(
  "prueba_cache"
  "prueba_cortoplazo"
  "prueba_general"
  "prueba_medianolargoplazo"
  "prueba_swap"
  "prueba_tlb"
)

# Iterar por cada carpeta y modificar el archivo kernel.config
for carpeta in "${carpetas[@]}"; do
  CONFIG_FILE="$carpeta/kernel.config"

  if [ -f "$CONFIG_FILE" ]; then
    echo "Modificando $CONFIG_FILE..."

    sed -i "s/^IP_MEMORIA=.*/IP_MEMORIA=$NUEVA_IP_MEMORIA/" "$CONFIG_FILE"
    sed -i "s/^PUERTO_MEMORIA=.*/PUERTO_MEMORIA=$NUEVO_PUERTO_MEMORIA/" "$CONFIG_FILE"
    sed -i "s/^PUERTO_ESCUCHA_DISPATCH=.*/PUERTO_ESCUCHA_DISPATCH=$NUEVO_PUERTO_DISPATCH/" "$CONFIG_FILE"
    sed -i "s/^PUERTO_ESCUCHA_INTERRUPT=.*/PUERTO_ESCUCHA_INTERRUPT=$NUEVO_PUERTO_INTERRUPT/" "$CONFIG_FILE"
    sed -i "s/^PUERTO_ESCUCHA_IO=.*/PUERTO_ESCUCHA_IO=$NUEVO_PUERTO_IO/" "$CONFIG_FILE"

    echo "Archivo $CONFIG_FILE modificado."
  else
    echo "Archivo $CONFIG_FILE no encontrado. Saltando..."
  fi
done

echo "Modificación completada en todas las carpetas."
