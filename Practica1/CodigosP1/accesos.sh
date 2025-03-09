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
        #con -r para que se imprima por terminal de forma decreciente
        cut -d '"' -f3 "$2" | cut -d ' ' -f2 | sort | uniq -c  | sort -r
        ;;

    -t)
        # Add your code for option -t here
        echo "hola"
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
