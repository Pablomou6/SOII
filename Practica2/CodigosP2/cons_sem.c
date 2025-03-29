#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#define N 8
#define ITERACIONES 60

typedef struct {
    char array[N];       // Buffer compartido
    int numElementos;    // Número de elementos en el buffer
} sharedData;

// Retirar un elemento del buffer compartido
char remove_item(sharedData* info) {
    if (info->numElementos > 0) {
        char elemento = info->array[info->numElementos - 1];
        info->numElementos--; // Decrementar el contador
        return elemento;
    } else {
        fprintf(stderr, "Error: Intento de retirar un elemento de un buffer vacío.\n");
        exit(EXIT_FAILURE);
    }
}

// Consumir un elemento y almacenarlo en el array local
void consume_item(char* arrayLocal, char elemento, int* indiceLibre) {
    arrayLocal[*indiceLibre] = elemento;
    (*indiceLibre)++;
}

int main(int argc, char** argv) {
    // Inicialización de variables
    char* arrayLocal = malloc(100 * sizeof(char));
    char elemento;
    int indiceLibre = 0;

    // Semáforos
    sem_t *vacias, *llenas, *mutex;

    // Abrir semáforos existentes
    vacias = sem_open("VACIAS", 0);
    llenas = sem_open("LLENAS", 0);
    mutex = sem_open("MUTEX", 0);

    if (vacias == SEM_FAILED || llenas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Error al abrir los semáforos");
        exit(EXIT_FAILURE);
    }

    // Abrir archivo compartido
    int fd = open("archivoCompartido.txt", O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // Mapear archivo a memoria compartida
    void* map = mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Asignar la memoria compartida a la estructura
    sharedData* info = (sharedData*)map;


    // Consumidor: iterar 60 veces
    srand(time(NULL));
    for (int i = 0; i < ITERACIONES; i++) {
        // Esperar a que haya elementos en el buffer
        sem_wait(llenas);
        sem_wait(mutex);

        // Retirar elemento del buffer
        elemento = remove_item(info);

        sem_post(mutex);
        sem_post(vacias);

        // Consumir el elemento
        consume_item(arrayLocal, elemento, &indiceLibre);
        printf("(cons) Elemento consumido: %c\n", elemento);

        // Simular tiempo de consumo
        sleep(rand() % 4);
    }

    // Imprimir el buffer local
    printf("Se han consumido las letras (buffer local): ");
    for (int i = 0; i < indiceLibre; i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El consumidor ha terminado.\n");

    // Liberar recursos
    munmap(map, sizeof(sharedData));
    close(fd);
    free(arrayLocal);

    // Cerrar semáforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);

    return 0;
}