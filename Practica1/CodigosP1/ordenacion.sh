#!/bin/bash

uso="Uso: $0 -flag <directorio>\n
Ejemplo: $0 -a /ruta/al/directorio\n
\t-a: Ordena por número de caracteres del nombre.\n
\t-b: Ordena alfabéticamente pero con el nombre escrito al revés.\n
\t-c: Ordena por los últimos 4 dígitos de su inode.\n
\t-d: Ordena por tamaño y agrupa en 8 categorías según permisos del propietario.\n
\t-e: Ordena por tamaño y agrupa por el mes de último acceso.\n"

#Verificamos que se pasen dos parámetros (flag y directorio)
if [ "$#" -ne 2 ]; then
    echo -e "Se requieren exactamente 2 parámetros.\n$uso"
    exit 1
fi

#Verificamos que el segundo parámetro sea un directorio
if [ ! -d "$2" ]; then
    echo "El directorio especificado no existe o no es válido."
    exit 1
fi

#Utilizamos realpath para obtener la ruta absoluta del directorio
directorio=$(realpath "$2")

case "$1" in
    -a)
        #Vamos a ordenar por el número de carácteres que tenga el nombre de cada archivo
        echo "Ordenando por número de caracteres en el nombre..."
        #Hacemos un find de los archivos que hay en el directorio y recorremos cada uno de ellos con un while read. Como siempre, -r evita que \ sea escapado.
        find "$directorio" -type f -o -type d| while read -r archivo; do
            #Con basename extraemos el nombre del archivo de la ruta completa, ya que find nos devuelve la ruta completa.
            nombre=$(basename "$archivo")
            #Hacemos un echo del nombre (teniendo en cuenta \n) y le contamos los carácteres con wc -c. Hacemos echo de este resultado y el nombre del archivo
            echo "$(echo -n "$nombre" | wc -c) $nombre"
        #Una vez que el while read termina, pasamos el resultado de contar los carácteres a sort para que ordene los nombres por el número de carácteres.
        #Como el primer elemento de cada línea es el número de carácteres, ya los ordena directamente. El -n es para que ordene numéricamente.
        done | sort -n
        ;;

    -b)
        echo "Ordenando alfabéticamente con nombres al revés..."
        #Hacemos un find de los archivos que hay en el directorio y recorremos cada uno de ellos con un while read. Como siempre, -r evita que \ sea escapado.
        find "$directorio" -type f | while read -r archivo; do
            #Con basename extraemos el nombre del archivo de la ruta completa, ya que find nos devuelve la ruta completa.
            nombre=$(basename "$archivo")
            #Se hace un echo en el while read para que se ejecute el comando rev y se invierta el nombre en todos los archivos. Después, mediante un pipe
            #se redirige la salida del bucle al sort para que ordene alfabéticamente.
            echo "$nombre" | rev
        done | sort
        ;;

    -c)
        echo "Ordenando por los últimos 4 dígitos del inode..."
        #Al hcer el find, ponemos la opción -printf "%i %p\n" para que nos devuelva el inode y la ruta del archivo.
        #Después, con un while read -r leemos el inode y la ruta de cada archivo.
        find "$directorio" -type f -printf "%i %p\n" | while read -r inode archivo; do
            #Almacenamos en una variable el resultado de un echo de los inodes, pero antes pasando el inode por un sed -E (para las regex) 
            #de forma que el sed va asustituír (s/). Con el .* le indicamos que tome todos los carcteres, pero con (.{4})$ también le indicamos
            #que los 4 últimos caracteres se capturen, ya que el paréntesis hace un grupo de captura. 
            #Estos 4 últimos caracteres son los que se almacenen en la variable, ya que \1 hace que toda la línea se sustituya por estos.
            ultimos4=$(echo "$inode" | sed -E 's/.*(.{4})$/\1/')
            echo "$ultimos4 $(basename $archivo)"
        #De la mima forma, pasamos el resultado del while read al sort para que ordene por los 4 últimos dígitos.
        done | sort -n
        ;;

    -d)
        echo "Ordenando por tamaño y agrupando en 8 categorías según permisos..."
        #De forma similar, hcemos un find que nos devuelve los permisos del archivo en formato de cadena (%M), el tamaño en bytes (%s) y la ruta (%p).
        #Con un while read -r leemos los permisos, el tamaño y la ruta de cada archivo.
        find "$directorio" -type f -printf "%M %s %p\n" | while read -r permisos tam archivo; do
            #De cada archivo, sacamos los permisos del propietario con una expansión. Obtenemos los tres carácteres siguientes de permisos del mismo, 
            #empezando en el 1 (el iíndice empieza en 0)
            grupo=${permisos:1:3}
            #Hacemos un echo de los permisos, el tamaño y la ruta del archivo.
            echo "$grupo $tam $archivo"
        #Pasamos el resultado del while read a un sort, el cual ordena según el primer parámetro (-k1,1) (el 1 de después de la coma es para que ordene
        #solamente según el primer campo). En caso de empate, se ordena numéricamente por el segundo campo (-k2,2n). Es lo deseado, primero se agruoan y 
        #luego, internamente en cada grupo, se ordena según tamaño
        done | sort -k1,1 -k2,2n
        ;;

    -e)
        echo "Ordenando por tamaño y agrupando por mes de último acceso..."
        #Hacemos un find que nos devuelve el mes del último acceso (%Am), el tamaño en bytes (%s) y la ruta (%p).
        find "$directorio" -type f -printf "%Am %s %p\n" | while read -r mes tam archivo; do
            echo "$mes $tam $archivo"
        #De forma similar, pasamos el resultado del while y lo ordenamos como en el apartado anterior. Ordena según los meses y, en caso de empate,
        #ordena numéricamente por el tamaño. Esto es lo que buscamos, primero los agrupamos según los meses y, luego, los ordenamos por tamaño.
        done | sort -k1,1n -k2,2n
        ;;

    *)
        echo -e "Opción no válida.\n$uso"
        exit 1
        ;;
esac
