#!/bin/bash


#FORMATO: sala_XX_fecha@hora.res 
#EJEMPLO: sala_20_2023-02-11@17:04.4K

#Vamos a declarar una "constante" que usaremos luego
DEFAULT_IFS=$' \t\n'

#Como se pide que se muestre un mensaje de uso si no se introducen los parámetros correctos, definimos uno:
uso="Uso: $0 <directorio_origen> <directorio_destino>\n
Ejemplo: $0 /ruta/al/origen /ruta/al/destino"

#Comprobamos que el número de parámetros sea 2. $# nos devuelve el número de parámetros (sin contar el nombre del script)
if [ "$#" -ne 2 ]; then
    echo "Se requieren exactamente 2 parámetros."
    echo -e "$uso"
    exit 1
fi

#Comprobamos que el primer parámetro ($1) y el segundo ($2) sean directorios (-d). Negamos lo anterior (!) para que se ejecute el if si no es un directorio.
#Adicionalmente, obtenemos los valores de los parámetros entre comillas, asegurándonos de tratar cada uno como una sola unidad
if ([ ! -d "$1" ] || [ ! -d "$2" ]); then
    echo "Uno de los parámetros no es un directorio."
    echo -e "$uso"
    exit 1
fi

#De forma similar a lo anterior, comprobamos que tenga permisos de lectura.
if [ ! -r "$1" ]; then
    echo "El directorio de origen no tiene permisos de lectura."
    echo -e "$uso"
    exit 1
fi

#Comprobamos que el directorio de destino tenga permisos de escritura.
if [ ! -w "$2" ]; then 
    echo "El directorio de destino no tiene permisos de escritura."
    echo -e "$uso"
    exit 1
fi

#Recuperamos la ruta absoluta del segundo directorio
ruta_abs=$(realpath "$2")

#Buscamos subdirectorios que coincidan con el formato especificado en el enunciado.

#NOTA: a la hora de realizar el ejercicio, se crea un directorio con el formato y dentro se guarda el archivo con nombre modificado. De esta forma, yo
#entiendo que cada archivo tiene su carpeta, por lo que se hace la comprobación CON LA RUTA ENTERA para evitar que se sobreescriban.
#Con esto quiero justificar que se busque la ruta completa y no solo parte de ella. sala23/2023-02-11/4K se encuentra, sala23/2023-02-11/; no. 

#La búsqueda la hacemos mediante un find, el cual busca en la ruta absoluta del segundo directorio, especificándole el tipo de archivo (directorio) y
#la expresión regular a cunmplir. La expresión regular se compone de los siguientes elementos:
#   - .*/sala(2[0-9]|3[0-9]|4[0-9]|50)/[0-9]{4}-[0-9]{2}-[0-9]{2}/(HD|Full HD|4K|8K)
#    se hacen que las salas sean desde la 2 hasta la 4, pero añadiéndole los 9 posibles segundos dígitos (2[0-9]|3[0-9]|4[0-9]) y la sala 50.
#    Para la fecha, se especifica que es una combinación de 4, 2 y 2 dígitos, tomando valores de 0 a 9. (Se asumen fechas normales)
#    Por último, se especifican las 4 calidades de vídeo posibles mediante una colección de valores.
#Luego, el output de find se pasa a un grep silencioso (-q), el cual busca si hay alguna coincidencia (cualquiera, por el .). Si la encuentra, se imprime 
#un mensaje y se listan los subdirectorios
#Es importante especificar el tipo de expresión regular a utilizar, ya que por defecto, find utiliza la expresión regular básica, la cual
#no soporta ciertas funcionalidades como los grupos ()

if find "$ruta_abs" -type d -regextype posix-extended -regex ".*/sala(2[0-9]|3[0-9]|4[0-9]|50)/[0-9]{4}-[0-9]{2}-[0-9]{2}/(HD|Full HD|4K|8K)" | grep -q .; then
    echo "Se encontraron subdirectorios con la estructura requerida. Debe eliminarlos o almacenar una copia antes de ejecutar el programa."
    
    #Listar los subdirectorios encontrados
    echo "Subdirectorios encontrados:"
    find "$ruta_abs" -type d -regextype posix-extended -regex ".*/sala(2[0-9]|3[0-9]|4[0-9]|50)/[0-9]{4}-[0-9]{2}-[0-9]{2}/(HD|Full HD|4K|8K)"
    exit 1
fi

#NOTA: Ahora el enunciado me pide dos cosas:
#Crear todos lo directorios de salida, 20 . . . 50 y los subdirectorios con las distintas fechas, aunque estén vacı́os.
#Las fechas se deben obtener a partir de la lista de ficheros de entrada.
#Si tengo que crear archivos según las fechas de los archivos de entrada, no tendré archivos vacíos, ya que haré los justos. Esto contradice a lo que
#se menciona. Por lo que, tras pensar y debatir, tomaré las fechas independientemente de las salas y cada sala tendrá un subdirectorio
#por cada una de las fechas presentes, a pesar que en esa fecha, la sala no se usase. Además, también se crearán los formatos posibles, ya que de esta
#forma, tomo las fechas de los archivos (cumpliendo el 2do requisito) y quedarán algunas carpetas (las de formatos y fechas) vacías (1er requisito).

#Vamos a obtener las diferentes fechas en las que se usaron las salas
#find "$(realpath "$1")" -type f -name "sala*" | cut -d '_' -f 3 | cut -d '@' -f 1 | sort -u

#Inicializamos un array, en el que almacenaremos las fechas (no duplicadas). 
#Para esto, con declare -A se declara un array asociativo, que nos permite asociar un valor a una clave. En este caso, la clave será la fecha
declare -A fechas

for archivo in $(find "$(realpath "$1")" -type f -name "sala*"); do
    #Dado que find nos proporciona la ruta entera, extraemos el nombre del archivo
    nombre=$(basename "$archivo")
    
    #Extraemos la fecha (que está en la tercera parte del nombre, separado por '_')
    fecha=$(echo "$nombre" | cut -d '_' -f 3 | cut -d '@' -f 1)

    #Comprobamos si la fecha tiene el formato correcto (YYYY-MM-DD).
    #La comprobación se compone de diferentes componentes:
    #    Primero, tomamos el valor que se almacena en fecha
    #    Con el símbolo =~ comprobamos si el valor cumple la regex, ya que es el símbolo que se emplea
    #    Marcamos el inicio de la cadena con ^ y, dentro de esta, comprobamos que tenga 4 números de 0 al 9, seguidos de un guión, para tener un par
    #    de dígitos (otra vez del 0 al 9) y otro guión, acabando en otro par. Indicamos el final de la cadena con $
    if [[ "$fecha" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
        #Como es un array asocaitivo, cada fecha la almacenamos con ella misma como clave. Si ya existe, no se sobreescribe
        #Almacenamos un 1 pq la clave es suficiente para saber qué fecha es, además, como el valor no nos importa, simplemente marcamos un 1 como "presente"
        fechas["$fecha"]=1
    fi
done

#Con find ../material_P1/conference/ -type f -name '*2023-09-1[^3,4,5]*' podemos ver que solo hay 3 fechas distintas, pero solo en este caso para verificar

#A continuación, crearemos los directorios de salida:
#Primero, creamos las salas, que es lo más "externo"
for i in {20..50}; do
    #Luego, para cada una de las fechas:
    #El array lo recorremos de la forma "${!array[@]}", que nos devuelve las claves del array. Esto se consigue ya que {...} es una expansión, de
    #forma que expandimos todos los elementos del array "fechas" (indicamos la totalidad de elementos con [@]) y, con !, indicamos que queremos las claves
    for fecha in "${!fechas[@]}"; do
        for formato in "HD" "Full HD" "4K" "8K"; do
            #Creamos los directorios con la estructura requerida
            #Empleamos la ruta absoluta del directorio destino, con los valores correspondientes de la sala i-ésima, la fecha y el formato.
            #Con la flag -p de mkdir, se crean los directorios intermedios si no existen
            mkdir -p "$ruta_abs/sala$i/$fecha/$formato"
        done
    done
done

#El for, en las cadenas toma los espacios en blanco como divisores (cosa que pasa también arriba, al exttraer la fecha, pero no lo sabía y se añadió 
#la comprobación de formato). Para evitar esto, modificaremos temporalmente el IFS (Internal Field Separator) para que 
#no tome los espacios en blanco como divisores
IFS=$'\n'

#Ahora, volveremos a leer los nombres de los archivos en el directorio y los moveremos a la carpeta correspondiente
for archivo in $(find "$(realpath "$1")" -type f -name "sala*"); do
    #Extraemos el nombre del archivo, ya que ahora mismo tenemos la ruta absoluta
    nombre=$(basename "$archivo")
    num_sala=$(echo "$nombre" | cut -d '_' -f 2)
    fecha=$(echo "$nombre" | cut -d '_' -f 3 | cut -d '@' -f 1)
    hora=$(echo "$nombre" | cut -d '@' -f 2 | cut -d '.' -f 1)
    #Extraemos la resolución, pero, como cut tiene de delimitador el punto, al tomar Full HD, solo toma Full ya que al detectar el espacio, se
    #queda con "la primera palabra". Para eso, cuando le indicamos que queremos el campo 2, debemos poner el guión después de -f2 para así
    #indicar que queremos un campo (-f), concretamente el 2 (-f2), pero hasta el final de la cadena (-f2-)
    resolucion=$(echo "$nombre" | cut -d '.' -f2-)

    #Una vez extraemos los parámetros necesarios para almacenarlo y modificarle el nombre, lo copiamos a su carpeta
    cp "$(realpath "$1")/sala_${num_sala}_$fecha@$hora.$resolucion" "$ruta_abs/sala$num_sala/$fecha/$resolucion/charla_$hora"
done

#Restauramos el IFS a su valor original (En muchos foros dicen que 'unset IFS' es lo correcto, incluso el propio ChatGPT. Sin embargo, en la 
#documentación no se menciona, como muchos otros usuarios lo dicen. Es por eso que, sabiendo que el valor por defecto del IFS es que separe cuando
#hay espacios, tabuladores y saltos de línea, lo volvemos a poner en ese valor)
IFS=$DEFAULT_IFS




