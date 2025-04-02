#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

//Declaramos constantes
#define N 8
#define ITERACIONES 60

//Declaramos la estructura que será compartuda 
typedef struct {
    char array[N];       
    int numElementos;    
} sharedData;

//Función que retira un elemento del buffer
char remove_item(sharedData* info) {
    if (info->numElementos > 0) {
        char elemento = info->array[info->numElementos - 1];
        info->numElementos--; 
        return elemento;
    } else {
        fprintf(stderr, "Error: Intento de retirar un elemento de un buffer vacío.\n");
        exit(EXIT_FAILURE);
    }
}

//Función para consumir un elemento y almacenarlo en el array local
void consume_item(char* arrayLocal, char elemento, int* indiceLibre) {
    arrayLocal[*indiceLibre] = elemento;
    (*indiceLibre)++;
}

int main(int argc, char** argv) {
    
    char* arrayLocal = malloc(1000 * sizeof(char));
    char elemento;
    int indiceLibre = 0;

    //Declaramos los semáforos
    sem_t *vacias, *llenas, *mutex;

    //Se abren los semáforos existentes
    vacias = sem_open("VACIAS", 0);
    llenas = sem_open("LLENAS", 0);
    mutex = sem_open("MUTEX", 0);

    if (vacias == SEM_FAILED || llenas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Error al abrir los semáforos");
        exit(EXIT_FAILURE);
    }

    //Abrimos un archivo que será compartido
    int fd = open("archivoCompartido.txt", O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    //Mapeamos el archivo a memoria compartida
    void* map = mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Asignamos el casteo de la memoria compartida a la estructura
    sharedData* info = (sharedData*)map;


    //Inicializamos la semilla de los números aleatorios
    srand(time(NULL));
    for (int i = 0; i < ITERACIONES; i++) {
        //Esperamos a que haya elementos en el buffer
        sem_wait(llenas);
        sem_wait(mutex);

        //Retiramos un elemento del buffer
        elemento = remove_item(info);

        //Debloqueamos el semáforo para el otro proceso
        sem_post(mutex);
        sem_post(vacias);

        //Consumimos el elemento
        consume_item(arrayLocal, elemento, &indiceLibre);
        printf("(cons) Elemento consumido: %c\n", elemento);

        //Simulamos un tiempo de consumo
        sleep(rand() % 4);
    }

    //Imprimimos el buffer local
    printf("Se han consumido las letras (buffer local): ");
    for (int i = 0; i < indiceLibre; i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El consumidor ha terminado.\n");

    //Liberamos recursos
    munmap(map, sizeof(sharedData));
    close(fd);
    free(arrayLocal);

    //Cerramos semáforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);

    return 0;
}