#!/bin/bash

uso="Uso: $0 -flag <archivo>\n
Ejemplo: $0 -c telefonos.txt\n
\t-c: Crear una nueva agenda\n
\t-a: Añadir un nuevo contacto\n
\t-bN: Buscar un contacto por nombre\n
\t-bT: Buscar un contacto por teléfono\n
\t-m: Modificar un contacto\n
\t-e: Eliminar un contacto\n"

#Verificamos el número de parámetros
if [ "$#" -ne 2 ]; then
    echo -e "Se requieren exactamente 2 parámetros.\n$uso"
    exit 1
fi

#Compruebamos que el archivo existe
if [ ! -f "$2" ]; then
    echo -e "El archivo no existe.\n$uso"
    exit 1
fi

#Función que valida el número de teléfono
validar_telefono() {
    #Recupera el primer parámetro y comprueba que coincide con una regex (=~) de forma que tiene  9 dígitos y el primero no es 0.
    if [[ ! "$1" =~ ^[1-9][0-9]{8}$ ]]; then
        echo "El teléfono introducido no es válido."
        exit 1
    fi
}

#Función para buscar un contacto por nombre o teléfono
buscar_contacto() {
    #Recupera el primer parámetro, que indica si se busca por un nombre o por un teléfono. El segundo parámetro es el valor a buscar y el tercero el archivo.
    #Las variables se declaran locales ya que es una buena práctica para evitar que se sobreescriban por otras funciones o valores fuera de la función.
    local tipo="$1"
    local valor="$2"
    local archivo="$3"

    #Como hemos dicho, se filtra por el tipo y, una vez filtrado, se hace un grep insensible a mayúsculas (-i). Si se busca por nombre, se indica que 
    #debe tener el inicio de línea justo antes (^) y si se busca por teéfono, primero se valida y luego indicamos que tenga @ antes y fin de línea ($).
    if [[ "$tipo" == "nombre" ]]; then
        grep -i "^$valor" "$archivo"
    else
        validar_telefono "$valor"
        grep "@$valor$" "$archivo"
    fi
}

#Para confirmar una acción, como borrar un contacto, mostramos primero el contacto (para saber si es ese o no) y luego preguntamos si se confirma.
confirmar_accion() {
    local contacto="$1"
    echo "El contacto encontrado es: $contacto. ¿Confirmar? (s/n)"
    read confirmacion
    if [ "$confirmacion" != "s" ]; then
        echo "Operación cancelada."
        exit 1
    fi
}

case "$1" in
    -c) 
        #Hacemos un echo -n (no imprime el salto de línea) para vaciar el archivo.
        echo -n > "$2"
        echo "Se ha creado una nueva agenda en el archivo."
        ;;

    -a)
        #Se pide el nombre y el teléfono del contacto y se valida el teléfono.
        echo -n "Introduce el nombre completo del contacto: "
        read nombre
        echo -n "Introduce el teléfono del contacto: "
        read telefono
        validar_telefono "$telefono"
        #Se añade el contacto al archivo con el formato nombre@teléfono y por el final del archivo.
        echo "$nombre@$telefono" >> "$2"
        echo "Contacto añadido correctamente."
        ;;

    -bN)
        #Se pide al usuario el nombre
        echo -n "Introduce el nombre del contacto a buscar: "
        read nombre
        #Se busca el contacto por nombre con la función anterior
        contacto=$(buscar_contacto "nombre" "$nombre" "$2")
        #Ahora, en caso de no no encontrar el contacto, la variable no contendrá nada, por lo que se verifica cn -z. Si se encontró, la expresión ya no se
        #cumple y se imprime el contacto. Si no se encontró, la primera condición se dará verdadera, por lo que el echo se imprime y se sale del script.
        [ -z "$contacto" ] && echo "No se encontró el contacto." && exit 1
        echo "$contacto"
        ;;

    -bT)
        #Se pide al usuario el teléfono
        echo -n "Introduce el teléfono del contacto a buscar: "
        read telefono
        #Buscamos el contacto con la función y lo almacenamos en la variable
        contacto=$(buscar_contacto "telefono" "$telefono" "$2")
        #Misma lógica que el caso anterior, si no se encuentra, -z se cumple por lo que la expresión entera se ejecuta.
        [ -z "$contacto" ] && echo "No se encontró el contacto." && exit 1
        echo "$contacto"
        ;;

    -m)
        #Pedimos la información del contacto a modificar y validamos el teléfono
        echo -n "Introduce el nombre del contacto a modificar: "
        read nombre
        echo -n "Introduce el teléfono del contacto a modificar: "
        read telefono
        validar_telefono "$telefono"

        #Buscamos el contacto y lo almacenamos en la variable. Como se trata de una modificación, se necesita la máxima precisión posible, por eso
        #el usuario debe recordar el nombre completo. Sino, no se encuentra.
        contacto=$(grep -i "^$nombre@$telefono$" "$2")
        #Si no se encuentra, se imprime un mensaje y se sale del script.
        [ -z "$contacto" ] && echo "No se encontró el contacto." && exit 1
        #Como vamos a modificar el contacto, se pide confirmación
        confirmar_accion "$contacto"

        #Pedimos la nueva información del contacto y, si no se introduce nada, se mantiene la información anterior.
        echo -n "Introduce el nuevo nombre (deja vacío para no cambiarlo): "
        read nuevo_nombre
        echo -n "Introduce el nuevo teléfono (deja vacío para no cambiarlo): "
        read nuevo_telefono

        #Al no introducir nada, queremos mantener uno de los campos (o ambos), por lo que, de forma similar a como hicimos antes, ponemos la condición
        #de si la variable está vacía. En caso de estar vacía, asignamos el valor anterior.
        [[ -z "$nuevo_nombre" ]] && nuevo_nombre="$nombre"
        [[ -z "$nuevo_telefono" ]] && nuevo_telefono="$telefono"

        #Antes de modificar nada, debemos comprobar que el número de teléfono es válido.
        validar_telefono "$nuevo_telefono"
        #Eliminamos el contacto anterior. Esto lo hacemos con sed, que le indicamos que el cambio será en el mismo archivo (-i) y que la expresión
        #será la que coincida con contacto (de esta forma, no aseguramos que sed tenga el formato correcto nombre@tlf y coincidan las mayúsculas). 
        #Con el "d" al final le indicamos que la línea que coincida, se elimina.
        sed -i "/$contacto/d" "$2"
        #Añadimos el nuevo contacto al archivo.
        echo "$nuevo_nombre@$nuevo_telefono" >> "$2"
        echo "Contacto modificado correctamente."
        ;;

    -e)
        #Pedimos la información del contacto a eliminar y validamos el teléfono
        echo -n "Introduce el nombre del contacto a eliminar: "
        read nombre
        echo -n "Introduce el teléfono del contacto a eliminar: "
        read telefono
        validar_telefono "$telefono"

        #Buscamos el contacto y lo almacenamos en la variable. Como se trata de un borrado, se necesita la máxima precisión posible, por eso
        #el usuario debe recordar el nombre completo. Sino, no se encuentra.
        contacto=$(grep -i "^$nombre@$telefono$" "$2")
        [ -z "$contacto" ] && echo "No se encontró el contacto." && exit 1
        confirmar_accion "$contacto"

        #Reutilizamos el código de sed para la eliminación de la línea.
        sed -i "/$contacto/d" "$2"
        echo "Contacto eliminado correctamente."
        ;;

    *)
        echo -e "Opción no válida.\n$uso"
        exit 1
        ;;
esac
