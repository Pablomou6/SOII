#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#define N 8             // Tamaño del buffer
#define ITERACIONES 60  // Número de iteraciones

void produce_item(char* elemento, char* arrayLocal) { 
    // Generamos una letra aleatoriamente
    *elemento = 'A' + rand() % 26;
    // Introducimos la letra en el array local de char's
    arrayLocal[strlen(arrayLocal)] = *elemento;
}

void insert_item(char elemento, void* arrayCompartido, int* nElementosBuffer) {
    // Convertimos el arrayCompartido al tipo adecuado
    char* buffer = (char*)arrayCompartido;
    // Añadir caracter generado al buffer compartido
    buffer[*nElementosBuffer] = elemento;
    (*nElementosBuffer)++;
}

int main(int argc, char *argv[]) {
    // Array local de char para almacenar los elementos generados
    char* arrayLocal = (char *)malloc(100 * sizeof(char));
    // Caracter que se va a generar
    char elemento;

    // Semáforos
    sem_t *vacias, *llenas, *mutex;

    // Eliminar semáforos existentes (buena práctica)
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    // Crear e inicializar semáforos
    vacias = sem_open("VACIAS", O_CREAT, 0700, N);
    llenas = sem_open("LLENAS", O_CREAT, 0700, 0);
    mutex = sem_open("MUTEX", O_CREAT, 0700, 1);

    if (vacias == SEM_FAILED || llenas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Error al crear los semáforos");
        exit(EXIT_FAILURE);
    }

    // Abrir archivo para lectura y escritura
    int fd = open("archivoCompartido.txt", O_RDWR | O_CREAT, 0666); 
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // Truncar el archivo al tamaño del buffer
    if (ftruncate(fd, N * sizeof(char) + sizeof(int)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Mapear el archivo en memoria compartida
    void* map = mmap(NULL, N * sizeof(char) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Inicializar nElementosBuffer en la memoria compartida
    int* nElementosBuffer = (int*)map;
    *nElementosBuffer = 0;
    char* buffer = (char*)(nElementosBuffer + 1);

    // Productor: iterar 60 veces
    srand(time(NULL));
    for (int i = 0; i < ITERACIONES; i++) {
        // Producir un elemento
        produce_item(&elemento, arrayLocal);
        printf("(prod) Elemento generado: %c\n", elemento);

        // Esperar a que haya espacio en el buffer
        sem_wait(vacias);
        sem_wait(mutex);

        // Insertar el elemento en el buffer
        insert_item(elemento, buffer, nElementosBuffer);
        printf("(prod) Elemento insertado en el buffer\n");

        sem_post(mutex);
        sem_post(llenas);

        // Simular tiempo de producción
        sleep(rand() % 4);
    }

    // Liberar recursos
    munmap(map, N * sizeof(char) + sizeof(int));
    close(fd);
    free(arrayLocal);

    // Cerrar y eliminar semáforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    return 0;
}