#Optimizaciones
OPTIMIZACIONES=('-O0' '-O3')

#Valores de M y T
M_VALORES=(100 1000 10000)
T_VALORES=(2 4 8 16)

#Número de repeticiones por combinación
REPETICIONES=10000

#Archivo fuente y ejecutable
SRC="sumaThreads_Foro1.c"
EXEC="suma_threads"

#Compilar y ejecutar para cada optimización
for OPT in "${OPTIMIZACIONES[@]}"; do
    echo "Compilando con $OPT..."
    gcc $OPT -lpthread $SRC -o $EXEC


    #Ejecutar combinaciones posibles de M y T
    for M in "${M_VALORES[@]}"; do
        for T in "${T_VALORES[@]}"; do
            #Reiniciamos el contador de errores para cada combinación
            ERRORES=0  
            #Reiniciamos contador de ejecuciones
            EJECUCIONES=0  

            echo "Ejecutando con M=$M, T=$T ($REPETICIONES veces)..."

            for ((i = 0; i < REPETICIONES; i++)); do
                OUTPUT=$(./$EXEC $M $T)

                #Detectamos si se ha sumado correctamente
                if echo "$OUTPUT" | grep -q "La suma no ha sido calculada correctamente"; then
                    ((ERRORES++))
                fi
                ((EJECUCIONES++))
            done

            #Calculamos el porcentaje de error para esta combinación
            ERROR_PERCENTAGE=$(echo "scale=4; ($ERRORES / $EJECUCIONES) * 100" | bc -l)
            echo "M=$M, T=$T -> Errores: $ERRORES / $EJECUCIONES -> $ERROR_PERCENTAGE% de error"
            echo "------------------------------------------------------"
        done
    done
done