#!/bin/bash

# Verificación de argumentos
if [ "$#" -ne 1 ]; then
  echo "Uso: $0 <PUERTO_ESCUCHA>"
  exit 1
fi

# Nuevo valor para PUERTO_ESCUCHA
NUEVO_PUERTO_ESCUCHA=$1

# Directorios a procesar
carpetas=(
  "prueba_cache"
  "prueba_cortoplazo"
  "prueba_general"
  "prueba_medianolargoplazo"
  "prueba_swap"
  "prueba_tlb"
)

# Iterar por cada carpeta y modificar el archivo memoria.config
for carpeta in "${carpetas[@]}"; do
  CONFIG_FILE="$carpeta/memoria.config"

  if [ -f "$CONFIG_FILE" ]; then
    echo "Modificando $CONFIG_FILE..."

    sed -i "s/^PUERTO_ESCUCHA=.*/PUERTO_ESCUCHA=$NUEVO_PUERTO_ESCUCHA/" "$CONFIG_FILE"

    echo "Archivo $CONFIG_FILE modificado."
  else
    echo "Archivo $CONFIG_FILE no encontrado. Saltando..."
  fi
done

echo "Modificación completada en todas las carpetas."
