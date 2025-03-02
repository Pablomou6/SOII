#!/bin/bash

esBisiesto() {
    if [ $(($1%4)) -eq 0 -a $(($1%100)) -ne 0 -o $(($1%400)) -eq 0 ]; then
        return 0 #return 0 indica que es bisiesto, ya que el 0 es el "True"
    else
        return 1
    fi
}

dias_meses=(31 28 31 30 31 30 31 31 30 31 30 31)

fecha="$1"

echo "La fecha introducida es: $fecha. Tiene el formato: dd/mm/yyyy ? [s/n]"
read respuesta
if [ "$respuesta" != "s" ] && [ "$respuesta" != "S" ]; then
    echo "El formato de la fecha no es correcto. Debe ser dd/mm/yyyy"
    exit 1
fi

#Recuperamos el día haciendo un cut en el string. El delimitador es el slash (se indica con -d) y el campo a recuperar es el primero (se indica con -f)
dia=$(echo $fecha | cut -d '/' -f 1)
mes=$(echo $fecha | cut -d '/' -f 2)
anho=$(echo $fecha | cut -d '/' -f 3)

#Comprobamos si la fecha es correcta: Comparamos los rangos posibles de día, mes y año.
if [ $dia -gt 0 -a $dia -lt 32 -a $mes -gt 0 -a $mes -lt 13 -a $anho -gt 0 ]; then
    if [ $mes -eq 2 -a $dia -eq 29 ]; then
        #Si el mes es febrero y el día es 29, comprobamos si el año es bisiesto.
        #Comprobamos si el año es bisiesto; si es divisible entre cuatro y no entre cien, ó es divisible entre 400, es un año bisiesto. 
        #Para usar la función como parámetro, no se necesita los corchetes y se debe pasar el valor de la variable.
        if esBisiesto $anho; then
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
#IMPORTATNE!!!!! Dado que recuperamos el día actual, este será empleado en las operaciones, por lo que no será contado como transcurrido.
#es decir, si por ejemplo estamos en 28/2/2025 e introducimos 2/2/2025, el resultado será 26 días; desde el 2 (incluído) hasta el 27 (incluído)
fecha_actual=$(date +%d/%m/%Y)
dia_actual=$(echo $fecha_actual | cut -d '/' -f 1)
mes_actual=$(echo $fecha_actual | cut -d '/' -f 2)
anho_actual=$(echo $fecha_actual | cut -d '/' -f 3)

#Comprobamos si la fecha es anterior a la actual: (para usar el operador && debemos poner los corchetes de la condición en cada comparación y los pareńtesis)
if [ $anho -gt $anho_actual ] || ([ $anho -eq $anho_actual ] && [ $mes -gt $mes_actual ]) || 
   ([ $anho -eq $anho_actual ] && [ $mes -eq $mes_actual ] && [ $dia -gt $dia_actual ])
then
    echo "La fecha introducida $fecha es posterior a la actual. No es válida."
    exit 1
fi  

#Declaramos una variable para llevar la cuenta de días de diferencia, luego haremos las conversiones
dias_totales=0

#El primer caso es si el año ingresado es el mismo que el actual
if [ "$anho" -eq "$anho_actual" ]; then
    #Comprobamos también si el mes ingresado es el mismo que el actual
    if [ "$mes" -eq "$mes_actual" ]; then
        #En caso de serlo, sumamos la diferencia de días
        dias_totales=$((dia_actual - dia)) 
    else
        #Si no es el mismo mes, sumamos los días restantes del mes del usuario
        dias_totales=$((dias_meses[mes-1] - dia))
        
        #Una vez sumamos lo días restantes del propio mes, sumamos los días de los meses restantes en el año
        #Como el programa, respecto a lo "humano", lleva el índice de los meses desde 0, restamos 1 al mes actual. Además, no es necesario
        #Sumar 1 cuando asignamos el valor a i, ya que nuestro i-ésimo mes es el i+1-ésimo mes del usuario 
        for ((i=mes; i < mes_actual-1; i++)); do
            dias_totales=$((dias_totales + dias_meses[i]))
        done
        
        #Por último, agregamos los días que han pasado en el mes actual
        dias_totales=$((dias_totales + dia_actual))
    fi

#La segunda opción es si el año ingresado es anterior al actual
elif [ "$anho" -lt "$anho_actual" ]; then
    #Sumamos primero los días restantes del mes indicado por el usuario
    dias_totales=$((dias_meses[mes-1] - dia))
    
    #Sumamos los días de los meses restantes del año indicado por el usuario
    for ((i=mes; i < 12; i++)); do
        dias_totales=$((dias_totales + dias_meses[i]))
    done

    #Sumamos los días de los años inetrmedios
    for ((i=anho+1; i < anho_actual; i++)); do
        if esBisiesto $i; then
            dias_totales=$((dias_totales + 366))
        else
            dias_totales=$((dias_totales + 365))
        fi
    done

    #Sumamos los días trnascurrido en el año actual, de meses intermedios(tenemos en cuenta si el año es bisiesto, por si ejecutamos el código en uno)
    for ((i=0; i < mes_actual-1; i++)); do
        if esBisiesto $anho_actual && [ $i -eq 1 ]; then
            dias_totales=$((dias_totales + dias_meses[i] + 1))
        else 
            dias_totales=$((dias_totales + dias_meses[i]))
        fi
    done

    #Sumamos los días transcurridos del mes actual
    dias_totales=$((dias_totales + dia_actual))
fi


#Vamos ahora a considerar el cambio de calendario Juliano al Gregoriano.
#La primera opción es que sea la fecha el 15/10/1582 o posterior (no se debe hacer nada, ya que se ha producido el cambio)
#Este caso, como no debemos hacerle nada, no lo consideramos en el código

#La segunda opción es que sea la fecha el 4/10/1582 o anterior
if [ "$anho" -lt 1582 ] || ([ "$anho" -eq 1582 ] && [ "$mes" -lt 10 ]) || ([ "$anho" -eq 1582 ] && [ "$mes" -eq 10 ] && [ "$dia" -le 4 ]); then
    #Si la fecha es anterior al cambio, debemos restar los días que se han eliminado del calendario
    #Para ello, restamos los días que se han eliminado del calendario (10 días)
    dias_totales=$((dias_totales - 10))

elif [ "$anho" -eq 1582 ] && [ "$mes" -eq 10 ] && ([ "$dia" -ge 5 ] && [ "$dia" -le 14 ]); then
    #Si la fecha es válida (primer if del código, que comprueba el formato), pero está en el intervalo de días eliminados del calendario
    #Lo informaremos al usuario y terminaremos el programa
    echo "La fecha $fecha está en el intervalo de días eliminados por el cambio de calendario [5/10/1582 - 14/10/1582]"
    exit 1

fi

#A pesar de que el enunciado limita el número de años a 2025, anteriormente, en la suma de los días de diferencia, se comprueba si
#el año es bisiesto, por lo que, aunque se ejecutase en un año bisiesto, se tendría en cuenta. Habría que actualizar el máximo dependiendo del año
#en el que se ejecute el código.

#Debemos tener en cuenta también la hora del día actual, para el cálculo de los minutos en el propio día
minutos_hoy=$(( $(date +%H) * 60 + $(date +%M) ))

#Ahora, vamos a convertir los días en minutos, días y años
#Convertimos días en minutos totales
minutos_totales=$((dias_totales * 24 * 60 + minutos_hoy))

#Hacemos el módulo de los minutos para obtener los minutos que pasaron en el último día (máximo 1440)
minutos_diferencia=$((minutos_totales % 1440))
if [ "$minutos_diferencia" -gt 1440 ]; then
    minutos_diferencia=1440
fi
#Debemos restar los minutos al total
minutos_totales=$((minutos_totales - minutos_diferencia))

#Calculamos los sías dentro del último año (máximo 365)
dias_diferencia=$((dias_totales % 365))
if [ "$dias_diferencia" -gt 365 ]; then
    dias_diferencia=365
fi

#Años totales (máximo 2025)
anhos_diferencia=$((dias_totales / 365))
if [ "$anhos_diferencia" -gt 2025 ]; then
    anhos_diferencia=2025
fi

#Los if's se aseguran de que no se sobrepase el máximo de años, días y minutos, ya que el enunciado lo limita

echo "Desde la fecha $fecha, hasta la fecha actual; $(date +%d/%m/%Y)"
echo "Pasaron:"
echo "  $anhos_diferencia años."
echo "  $dias_diferencia días."
echo "  $minutos_diferencia minutos."
echo "Este cálculo respeta el cambio de calendario al Gregoriano, por lo que cuenta 10 días menos si el la fecha introducida 
es anterior o igual al 4/10/1582. Se puede verificar con una página web, en la que tienen 10 días más (no cuentan el cambio)."





