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

void remove_item(void *arrayCompartido, char *elemento, int* nElementosBuffer) {
    char* buffer = (char*)arrayCompartido;
    if (*nElementosBuffer > 0) {
        *elemento = buffer[*nElementosBuffer - 1];
        (*nElementosBuffer)--;
    } else {
        fprintf(stderr, "Error: Intento de retirar un elemento de un buffer vacío.\n");
        exit(EXIT_FAILURE);
    }
}

void consume_item(char *arrayLocal, char elemento, int* indiceLibre) {
    // Añadir el caracter retirado del buffer a un string local
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
    void* map = mmap(NULL, N * sizeof(char) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Obtener punteros a la memoria compartida
    int* nElementosBuffer = (int*)map;
    char* buffer = (char*)(nElementosBuffer + 1);

    // Consumidor: iterar 60 veces
    srand(time(NULL));
    for (int i = 0; i < ITERACIONES; i++) {
        // Esperar a que haya elementos en el buffer
        sem_wait(llenas);
        sem_wait(mutex);

        // Retirar elemento del buffer
        remove_item(buffer, &elemento, nElementosBuffer);

        sem_post(mutex);
        sem_post(vacias);

        // Consumir el elemento
        consume_item(arrayLocal, elemento, &indiceLibre);
        printf("(cons) Elemento consumido: %c\n", elemento);

        // Simular tiempo de consumo
        sleep(rand() % 4);
    }

    // Liberar recursos
    munmap(map, N * sizeof(char) + sizeof(int));
    close(fd);
    free(arrayLocal);

    // Cerrar semáforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);

    return 0;
}