#!/bin/bash

echo "Introduce una fecha (dd/mm/aaaa):"
read fecha

#Recuperamos el día haciendo un cut en el string. El delimitador es el slash (se indica con -d) y el campo a recuperar es el primero (se indica con -f)
dia=$(echo $fecha | cut -d '/' -f 1)
mes=$(echo $fecha | cut -d '/' -f 2)
anho=$(echo $fecha | cut -d '/' -f 3)

#Comprobamos si la fecha es correcta: Comparamos los rangos posibles de día, mes y año.
if [ $dia -gt 0 -a $dia -lt 32 -a $mes -gt 0 -a $mes -lt 13 -a $anho -gt 0 ]; then
    if [ $mes -eq 2 -a $dia -eq 29 ]; then
        #Si el mes es febrero y el día es 29, comprobamos si el año es bisiesto.
        #Comprobamos si el año es bisiesto; si es divisible entre cuatro y no entre cien, ó es divisible entre 400, es un año bisiesto. 
        if [ $(($anho%4)) -eq 0 -a $(($anho%100)) -ne 0 -o $(($anho%400)) -eq 0 ]; then
            echo "La fecha $fecha es correcta"
        else
            echo "La fecha $fecha es incorrecta"
            exit 1
        fi
    fi
else
    echo "La fecha $fecha es incorrecta"
    exit 1
fi

#Obtenemos la fecha actual. Utilizamos date con el formato +%d/%m/%Y para obtener la fecha en el formato deseado. El día, el mes y el año
#el año debe ser Y (mayúscula) para que se muestre en formato de 4 dígitos.
fecha_actual=$(date +%d/%m/%Y)
dia_actual=$(echo $fecha_actual | cut -d '/' -f 1)
mes_actual=$(echo $fecha_actual | cut -d '/' -f 2)
anho_actual=$(echo $fecha_actual | cut -d '/' -f 3)
#Recuperar la hora y minutos actuales
hora_actual=$(date +%H)
minuto_actual=$(date +%M)

#Comprobamos si la fecha es anterior a la actual: (para usar el operador && debemos poner los corchetes de la condición en cada comparación y los pareńtesis)
if [ $anho -gt $anho_actual ] || ([ $anho -eq $anho_actual ] && [ $mes -gt $mes_actual ]) || 
   ([ $anho -eq $anho_actual ] && [ $mes -eq $mes_actual ] && [ $dia -gt $dia_actual ])
then
    echo "La fecha introducida $fecha es posterior a la actual. No es válida."
    exit 1
fi  




