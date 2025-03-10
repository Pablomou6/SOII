#!/bin/bash



#Vamos a declarar un mensaje de uso, en caso de que el usuario ejecute incorrectamente el script
uso="Uso: $0 <opción> <archivo>
    Ejemplo: $0 -c archivo.txt
    Opciones:
        -c: Muestra los diferentes códigos de respuesta sin repetición. 
        -t: Muestra el número de dı́as para los que no hay ningún acceso al servidor.
        GET: Cuenta el total de accesos con una petición tipo GET, con respuesta 200.
        POST: Cuenta el total de accesos con una petición tipo POST, con respuesta 200.
        -s: Resume el total de Datos enviados en KiB por cada mes.
        -o: Ordena las lı́neas del fichero access.log en orde decreciente del número de bytes enviados."

#Vamos a comprobar primero que los argumentos son correctos
if [ "$#" -ne 2 ]; then
    echo "Has introducido un número incorrecto de argumentos"
    echo $uso
    exit 1
fi

#Comprobamos ahora que el archivo tenga permisos de lectura (-r) y que sea un archivo (-f)
if ([ ! -r "$2" ] || [ ! -f "$2" ]); then
    echo "Se debe pasar un archivo regular y con permisos de lectura"
    echo $uso
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
        # Add your code for option GET here
        continue
        ;;

    POST)
        # Add your code for option POST here
        continue
        ;;

    -s)
        # Add your code for option -s here
        continue
        ;;

    -o)
        # Add your code for option -o here
        continue
        ;;

    *)
        echo "Opción no válida"
        echo $uso
        exit 1
        ;;
esac
