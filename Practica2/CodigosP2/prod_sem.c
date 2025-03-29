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

typedef struct {
    char array[N];       // Buffer compartido
    int numElementos;    // Número de elementos en el buffer
} sharedData;

// Generar un elemento aleatorio
void produce_item(char* elemento, char* arrayLocal) { 
    *elemento = 'A' + rand() % 26; // Generar una letra aleatoria
    arrayLocal[strlen(arrayLocal)] = *elemento; // Guardar en el array local
}

// Insertar un elemento en el buffer compartido
void insert_item(sharedData* info, char elemento) {
    info->array[info->numElementos] = elemento; // Insertar en el buffer
    info->numElementos++; // Incrementar el contador
}

int main(int argc, char *argv[]) {
    // Array local para almacenar los elementos generados
    char* arrayLocal = (char *)malloc(100 * sizeof(char));
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

    // Truncar el archivo al tamaño de la estructura compartida
    if (ftruncate(fd, sizeof(sharedData)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Mapear el archivo en memoria compartida
    void* map = mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Asignar la memoria compartida a la estructura
    sharedData* info = (sharedData*)map;
    info->numElementos = 0; // Inicializar el contador de elementos

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
        insert_item(info, elemento);
        printf("(prod) Elemento insertado en el buffer\n");

        sem_post(mutex);
        sem_post(llenas);

        // Simular tiempo de producción
        sleep(rand() % 4);
    }

    // Imprimir el buffer local
    printf("Se han producido las letras (buffer local): ");
    for (int i = 0; i < strlen(arrayLocal); i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El productor ha terminado.\n");

    // Liberar recursos
    munmap(map, sizeof(sharedData));
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