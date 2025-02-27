#!/bin/bash

touch ~/passwd_copia.txt
if [ $? -eq 0 ]
then
    echo "Se ha creado el archivo passwd_copia.txt"
else
    echo "Error al crear el archivo passwd_copia.txt"
fi

cat /etc/passwd > $HOME/passwd_copia.txt
if [ $? -eq 0 ]
then
    echo "Se ha copiado el contenido del archivo "/etc/passwd" al archivo passwd_copia.txt"
else
    echo "Error al copiar el contenido del archivo "/etc/passwd" al archivo passwd_copia.txt"
fi

