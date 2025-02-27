#!/bin/bash

#Contamos las líneas previas a la ejecución
lineas_antes=$(wc -l $HOME/passwd_copia.txt)

#Con el comando 'sort', ordenamos el archivo alfabéticamente. El output, lo pasamos con un pipe para que sea un input de uniq, 
sort $HOME/passwd_copia.txt | uniq > /tmp/passwd_orignal.txt

#Contamos las líneas después de la ejecución
lineas_despues=$(wc -l /tmp/passwd_orignal.txt)

echo "Lineas antes de copiar: $lineas_antes"
echo "Lineas despues de copiar: $lineas_despues"

#Ordenamos el archivo original y lo comparamos con el creado en este script. El guión se pone para ocupar el segundo parámetro, esto indica que se 
#comparará con el input del pipe
sort /etc/passwd | diff /tmp/passwd_orignal.txt -