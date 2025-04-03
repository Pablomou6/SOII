#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define N 8             // tamaño del buffer
#define ITERACIONES 60  // numero de iteraciones

// buffer compartido y variables globales
char buffer[N];
int nElementosBuffer = 0; // numero de elementos en el buffer

// semaforos
sem_t vacias, llenas, mutex;

// funcion para producir un elemento
void produce_item(char* elemento, char* arrayLocal) {
    *elemento = 'A' + rand() % 26; // generar letra aleatoria
    arrayLocal[strlen(arrayLocal)] = *elemento;
    arrayLocal[strlen(arrayLocal) + 1] = '\0'; // asegurar fin de cadena
}

// funcion para insertar un elemento en el buffer
void insert_item(char elemento) {
    buffer[nElementosBuffer] = elemento;
    nElementosBuffer++;
}

// funcion para retirar un elemento del buffer
char remove_item() {
    nElementosBuffer--;
    return buffer[nElementosBuffer];
}

// funcion del hilo productor
void* productor(void* arg) {
    char* arrayLocal = (char*)malloc(100 * sizeof(char));
    char elemento;

    srand(time(NULL)); // inicializar semilla aleatoria

    for (int i = 0; i < ITERACIONES; i++) {
        // producir un elemento
        produce_item(&elemento, arrayLocal);
        printf("(prod) Elemento generado: %c\n", elemento);

        // esperar a que haya espacio en el buffer
        sem_wait(&vacias);
        sem_wait(&mutex);

        // insertar el elemento en el buffer
        insert_item(elemento);
        printf("(prod) Elemento insertado en el buffer\n");

        sem_post(&mutex);
        sem_post(&llenas);

        // simular tiempo de produccion
        sleep(rand() % 4);
    }

    free(arrayLocal);
    pthread_exit(NULL);
}

// funcion del hilo consumidor
void* consumidor(void* arg) {
    char* arrayLocal = (char*)malloc(100 * sizeof(char));
    char elemento;

    for (int i = 0; i < ITERACIONES; i++) {
        // esperar a que haya elementos en el buffer
        sem_wait(&llenas);
        sem_wait(&mutex);

        // retirar un elemento del buffer
        elemento = remove_item();
        printf("(cons) Elemento retirado del buffer: %c\n", elemento);

        // consumir el elemento
        arrayLocal[strlen(arrayLocal)] = elemento;
        arrayLocal[strlen(arrayLocal) + 1] = '\0';
        printf("(cons) Elemento consumido: %c\n", elemento);

        sem_post(&mutex);
        sem_post(&vacias);

        // simular tiempo de consumo
        sleep(rand() % 4);
    }

    free(arrayLocal);
    pthread_exit(NULL);
}

int main() {
    pthread_t hiloProductor, hiloConsumidor;

    // inicializar semaforos
    sem_init(&vacias, 0, N);  // espacios vacios en el buffer
    sem_init(&llenas, 0, 0);  // elementos llenos en el buffer
    sem_init(&mutex, 0, 1);   // exclusion mutua

    // crear hilos
    pthread_create(&hiloProductor, NULL, productor, NULL);
    pthread_create(&hiloConsumidor, NULL, consumidor, NULL);

    // esperar a que los hilos terminen
    pthread_join(hiloProductor, NULL);
    pthread_join(hiloConsumidor, NULL);

    // destruir semaforos
    sem_destroy(&vacias);
    sem_destroy(&llenas);
    sem_destroy(&mutex);

    return 0;
}