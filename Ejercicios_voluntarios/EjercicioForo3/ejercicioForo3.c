#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main() {
    //Semilla para los números aleatorios
    srand(time(NULL));

    double x, y, z;
    for(int i = 0; i < 10000; i++) {
        //Generamos aleatorios entre el rango [-10, 10]
        x = ((double)rand() / RAND_MAX) * 20 - 10;
        y = ((double)rand() / RAND_MAX) * 20 - 10;
        z = ((double)rand() / RAND_MAX) * 20 - 10;

        //Hacemos las operaciones
        double t = x+y;
        double resultado1 = t+z;

        t = y+z;
        double resultado2 = x+t;

        //Comprobamos si la diferencia de resultados es mayor que 1e-16, en caso afirmativo, mostramos los valores
        if(fabs(resultado1-resultado2) > 1e-15) {
            printf("El el intento %d, con los valores:\n", i);
            printf("x = %.17g\n", x);
            printf("y = %.17g\n", y);
            printf("z = %.17g\n", z);
            printf("Obtenemos el resultado 1 %.17g\n", resultado1);
            printf("Obtenemos el resultado 2 %.17g\n", resultado2);
            printf("Obtenemos la diferencia %.17g\n", fabs(resultado1-resultado2));
            printf("\n");
            return EXIT_SUCCESS;
        }
    }
    
    return EXIT_SUCCESS;
}