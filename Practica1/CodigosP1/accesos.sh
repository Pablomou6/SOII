#!/bin/bash



#Vamos a declarar un mensaje de uso, en caso de que el usuario ejecute incorrectamente el script
uso="Uso: $0 <opción> <archivo>\n
    Ejemplo: $0 -c archivo.txt\n
    Opciones:\n
        -c: Muestra los diferentes códigos de respuesta sin repetición.\n 
        -t: Muestra el número de dı́as para los que no hay ningún acceso al servidor.\n
        GET: Cuenta el total de accesos con una petición tipo GET, con respuesta 200.\n
        POST: Cuenta el total de accesos con una petición tipo POST, con respuesta 200.\n
        -s: Resume el total de Datos enviados en KiB por cada mes.\n
        -o: Ordena las lı́neas del fichero access.log en orde decreciente del número de bytes enviados.\n"

#Vamos a comprobar primero que los argumentos son correctos
if [ "$#" -ne 2 ]; then
    echo "Has introducido un número incorrecto de argumentos"
    echo -e $uso
    exit 1
fi

#Comprobamos ahora que el archivo tenga permisos de lectura (-r) y que sea un archivo (-f)
if ([ ! -r "$2" ] || [ ! -f "$2" ]); then
    echo "Se debe pasar un archivo regular y con permisos de lectura"
    echo -e $uso
    exit 1
fi

case "$1" in
    -c)
        #Para recuperar los códigos de respuesta, haremos un cut indicando que debe separar la línea en las comillas y coger el tercer campo. De esta forma,
        #especificándole el archivo, se lo hace a todas las líneas. Una vez termina el primer cut, tendremos el número de respuesta y el de bytes, por
        #lo que repetimos un cut, para recuperar solo la respuesta. Una vez tenemos esto, ordenamos con un sort (para poner todas las lineas repetidas una
        #seguida de la otra), a lo que le hacemos un uniq, para quedarnos con 1 copia. Usamos -c para contar cuántas veces aparecía. Volvemos a ordenar
        #con -r para que se imprima por terminal de forma decrecienteopen .
        cut -d '"' -f3 "$2" | cut -d ' ' -f2 | sort | uniq -c  | sort -r
        ;;

    -t) 
        #En esta opción se asumen 2 cosas: Los accesos al servidor se guardaron cronológicamente, es decir, en orden desde el más antiguo al más reciente.
        #Además, se asume que las fechas son posteriores al 1 de enero de 1970, ya que se usan los segundos desde esa fecha para calcular la diferencia.
        #En caso de no ser así, se deberá modificar el script en sus respectivos lugares.

        #Dado que es un registro de accesos a un servidor, se asumen su orden cronológico para almacenarlo en un array.
        declare -a lista_fechas
        #Hacemos un cut para quedarnos con las fechas del archivo. Con ambos cut's, nos quedamos solo con la fecha, mes y año. Ordenamos luego con sort -r
        #y, por último, hacemos uniq para dejar solo una copia de cada fecha. El resultado de esta operación lo almacenamos en un array con 
        #"process substitution" [< <(comando)] para que el output del comando se almacene en la variable. Esta variables es un array que, con mapfile,
        #leemos las líneas output del comando y las almacenamos en el array, ya que cada línea será un elemento. 
        #La flag -t se usa para que no se incluyan los saltos de línea.
        mapfile -t lista_fechas < <(cut -d '[' -f2 "$2" | cut -d ':' -f1 | sort | uniq)

        #Vamos ahora a hacer un array para almacenar las fechas en segundos desde 1970 y así poder ordenarlas.
        #Para ello, vamos a hacer un bucle que recorra el array de fechas y, para cada fecha, la convierta a segundos.
        declare -a fechas_segundos
        for fecha in "${lista_fechas[@]}"; do
            #Almacenamos las fechas en el array con "+=" para que se concatenen por el final. Para convertir la fecha a segundos, necesitamos formatear
            #la fecha, ya que tiene formato dd/Mon/yyyy en el archivo. Entonces, usamos un date -d para convertir la fecha a segundos, pero la fecha que
            #queremos convertir es la resultado de un echo, con pipe a un sed, función que formatea un string con un patrón dado. En este caso, el patrón
            #es sustituir las barras por espacios. Por último, le decimos a date que queremos los segundos desde 1970.
            #NOTA: El patrón consiste de; s#/# #g, donde s le indica que se realizará una substitución, # es el delimitador (en el patrón), / es el
            #carácter deseado a sustituír, ' ' indica que se sustituirá por un espacio y g indica que se hará global, en todas las ocurrencias de /.
            fechas_segundos+=($(date -d "$(echo "$fecha" | sed 's#/# #g')" +%s))
        done
        
        #Ahora, vamos a ordenar el array
        declare -a fechas_ordenadas
        #Para ordenar el array, primero lo convertimos a un string con "${fechas_segundos[@]}" (lo convertimos en su totalidad debido a [@] y la expansión{}), 
        #luego lo pasamos como input de tr, el cual TRADUCE los espacios ' ' por saltos de línea '\n' y, por último, se ordena con sort -n para que 
        #se ordene numéricamente a pesar de ser un string. El resultado de sort se almacena en un nuevo array.
        fechas_ordenadas=($(echo "${fechas_segundos[@]}" | tr ' ' '\n' | sort -n))
        
        #Ahora, como tenemos las fechas ordenadas, vamos a hacer un bucle que recorra las fechas y calcule la diferencia entre la i y la i+1.
        dias_sin_acceso=0
        for ((i=0; i<${#fechas_ordenadas[@]}-1; i++)); do
            #Recuperamos la fecha actual y la fecha siguiente
            fecha_actual=${fechas_ordenadas[i]}
            fecha_siguiente=${fechas_ordenadas[i+1]}
            #Calculamos la diferencia entre ambas fechas y la dividimos entre 86400 para obtener los días.
            diferencia=$(( (fecha_siguiente - fecha_actual) / 86400 ))
            #Si la diferencia es mayor a 1, incrementamos el contador
            if (( diferencia > 1 )); then
                ((dias_sin_acceso++))
            fi
        done
        echo "Fecha del primer acceso: $(date -d "@${fechas_ordenadas[0]}" '+%d/%m/%Y')"
        echo "Fecha del último acceso: $(date -d "@${fechas_ordenadas[-1]}" '+%d/%m/%Y')"
        echo "Número de días sin acceso al servidor: $dias_sin_acceso"
        ;;

    GET)
        #Hacemos un grep para quedarnos con las líneas que tienen GET y hacemos un cut, delimitado por espacios, para quedarnos con el noveno campo.
        #Esto es posible a que conocemos el formato del archivo. Una vez tenemos todos los campos de respuesta de los GET, hacemos un grep para quedarnos
        #con los que tienen respuesta 200 y, gracias a la flag -c, contamos cuántas veces aparece.
        num_GET=$(grep "GET" "$2" | cut -d' ' -f9 | grep -c "200") 
        echo "$(date '+%b %d %H:%M:%S') - Número de accesos GET con respuesta 200: $num_GET"
        ;;

    POST)
        #Hacemos un grep para quedarnos con las líneas que tienen POST y hacemos un cut, delimitado por espacios, para quedarnos con el noveno campo.
        #Esto es posible a que conocemos el formato del archivo. Una vez tenemos todos los campos de respuesta de los POST, hacemos un grep para quedarnos
        #con los que tienen respuesta 200 y, gracias a la flag -c, contamos cuántas veces aparece.
        num_POST=$(grep "POST" "$2" | cut -d' ' -f9 | grep -c "200") 
        echo "$(date '+%b %d %H:%M:%S') - Número de accesos POST con respuesta 200: $num_POST"
        #Ambos casos se podrían hacer con wc -l, pero como no se asegura tener un \n en la última línea, se podría omitir el último resultado.
        ;;

    -s)
        #En el vector ponemos los meses que hay en el log, pero se podría generalizar
        meses=("Dec" "Jan" "Feb")

        #Hacemos un for para recorrer el array de los meses
        for mes in "${meses[@]}"; do

            total_datos=0 
            contador_accesos=0

            #Ahora, hacemos un while read -r (con la flag, no interpreta los \ como escape) el cual recibe como process substitution las líneas
            #que tienen el mes deseado. este mes se consigue haciendo un grep del mes para obtener las líneas. Después, se hace un cut, delimitado por 
            #espacios, y se coge el campo número 10, que será el último (posible debido a conocer el formato). Por último, se hace un grep que con las flags
            #se posibilita las regex extendidas y, con -o, se consigue que se impriman las líneas que coinciden con la regex. La que usamos, 
            #'[0-9]+', indica que hay uno o más (+) números del cero al nueve [0-9]
            while read -r dato; do
                #Una vez recuperamos el número de Bytes, comprobamos qu sea un número. Se indica el inicio de la expresión con ^, donde puede
                #haber uno o más dígitos del 0 al 9. La expresión termina con el $.
                if [[ "$dato" =~ ^[0-9]+$ ]]; then
                    ((total_datos += dato))
                    ((contador_accesos++))
                fi
            done < <(grep "$mes" "$2" | cut -d' ' -f10 | grep -oE '[0-9]+')

            #Convertimos a KiB dividiendo entre 1024
            total_datos_kib=$((total_datos / 1024))
            echo "$total_datos_kib KiB enviados en $mes en $contador_accesos accesos."
        done
        ;;

    -o)
        #Hacemos cat del archivo para que se muestre, pero lo pasamos mediante un pipe a un sort. Este sort tiene indicado que el espacio es separador
        #(-t ' ') y que el campo que se toma como referencia para ordenar, es el 10 (-k10). Por último, se ordena de forma numérica y de forma decreciente
        #(-nr) y se redirige la salida a un archivo. (Esta forma mezcla los 0's con los -'s. Se puede sustituír con un sed -E 's/([0-9]{3}) -$/\1 0/', pero
        #es muy lento. Otra opción es usar perl, pero se necesita tenerlo instalado) Como 0 y - se pueden entender como el mismo valor, se deja así.
        #NOTA: -k10 ordena tomando en cuenta desde el campo 10 hasta el final, se podría limitar, pero no es necesario.
        cat "$2" | sort -t' ' -k10 -nr > $HOME/access_ord.log   
        ;;

    *)
        echo "Opción no válida"
        echo $uso
        exit 1
        ;;
esac
