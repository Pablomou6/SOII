#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int M, T;
double suma = 0.0;

void* sum(void* arg) {
    int idHilo = *(int*)arg;
    for(int i = idHilo; i <= M; i += T) {
        suma += i;
    }
    pthread_exit(NULL);
}

double suma_correcta() {
    return (M*(M+1))/2.0;
}

int main(int argc, char* argv[]) {
    //Recuperamos los valores introducidos como args
    M = atoi(argv[1]);
    T = atoi(argv[2]);

    //Declaración de variables
    pthread_t Threads[T];
    int idHilos[T];

    //Comprobamos que se ingresen los argumentos correctos
    if (argc != 3) {
        printf("Error: Debe ingresar dos argumentos enteros\n");
        return -1;
    }

    //Creamos los hilos queejecutan la función sum
    for(int i = 0; i < T; i++) {
        idHilos[i] = i;
        pthread_create(&Threads[i], NULL, sum, &idHilos[i]);
    }

    //Nos aseguramos de que todos los hilos terminen
    for(int i = 0; i < T; i++) {
        pthread_join(Threads[i], NULL);
    }

    double suma_esperada = suma_correcta();

    if(suma_esperada != suma) {
        printf("La suma no ha sido calculada correctamente\n");
    }

    return EXIT_SUCCESS;
}