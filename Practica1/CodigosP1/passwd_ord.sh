#!/bin/bash

#Contamos las líneas previas a la ejecución
lineas_antes=$(wc -l $HOME/passwd_copia.txt)

#Con el comando 'sort', ordenamos el archivo alfabéticamente. Añadiéndole la flag -u, eliminamos las líneas duplicadas.
sort -u $HOME/passwd_copia.txt > /tmp/passwd_orignal.txt

#Contamos las líneas después de la ejecución
lineas_despues=$(wc -l /tmp/passwd_orignal.txt)

echo "Lineas antes de copiar: $lineas_antes"
echo "Lineas despues de copiar: $lineas_despues"

#Calculamos la diferencia de líneas, pero debemos quitar el nombre del archivo que devuelve el comando wc
lineas_antes=$(echo $lineas_antes | cut -d ' ' -f 1)
lineas_despues=$(echo $lineas_despues | cut -d ' ' -f 1)
diferencia=$(($lineas_antes-$lineas_despues))
echo "Diferencia de líneas: $diferencia"

#Ordenamos el archivo original y lo comparamos con el creado en este script. El guión se pone para ocupar el segundo parámetro, esto indica que se 
#comparará con el input del pipe
sort /etc/passwd | diff /tmp/passwd_orignal.txt -