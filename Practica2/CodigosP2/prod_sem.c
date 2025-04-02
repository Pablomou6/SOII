#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

//Declaramos unas constantes
#define N 8             
#define ITERACIONES 60  

//Declaramos la estructura que será compartida por ambos procesos
typedef struct {
    char array[N];       
    int numElementos;    
} sharedData;

//Función que para generar un elemento aleatorio
void produce_item(char* elemento, char* arrayLocal) { 
    *elemento = 'A' + rand() % 26; 
    arrayLocal[strlen(arrayLocal)] = *elemento; 
}

//Función que, dado un elemento, lo inserta en el buffer
void insert_item(sharedData* info, char elemento) {
    info->array[info->numElementos] = elemento; 
    info->numElementos++; 
}

int main(int argc, char *argv[]) {
    //Array local para almacenar los elementos generados
    char* arrayLocal = (char *)malloc(1000 * sizeof(char));
    char elemento;

    //Declamos los semáforos
    sem_t *vacias, *llenas, *mutex;

    //Como se menciona en el enunciado, es buena práctica eliminar los semáforos antes de crearlos
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    //Crear e inicializar semáforos
    vacias = sem_open("VACIAS", O_CREAT, 0700, N);
    llenas = sem_open("LLENAS", O_CREAT, 0700, 0);
    mutex = sem_open("MUTEX", O_CREAT, 0700, 1);

    if (vacias == SEM_FAILED || llenas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Error al crear los semáforos");
        exit(EXIT_FAILURE);
    }

    //Abrimos un archivo que será mapeado
    int fd = open("archivoCompartido.txt", O_RDWR | O_CREAT, 0666); 
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    //Truncamos el archivo al tamaño de la estructura compartida
    if (ftruncate(fd, sizeof(sharedData)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Mapeamos el archivo en memoria compartida
    void* map = mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Asignamos el casteo de la memoria compartida a la estructura
    sharedData* info = (sharedData*)map;
    info->numElementos = 0; 

    //Inicializamos la semilla para la generación de números aleatorios
    srand(time(NULL));
    for (int i = 0; i < ITERACIONES; i++) {
        //Producimos un elemento
        produce_item(&elemento, arrayLocal);
        printf("(prod) Elemento generado: %c\n", elemento);

        //Esperamos a que haya espacio en el buffer
        sem_wait(vacias);
        sem_wait(mutex);

        //Insertamos el elemento generado en el buffer
        insert_item(info, elemento);
        printf("(prod) Elemento insertado en el buffer\n");

        //Desbloqueamos los semáforos, permitiendo el acceso al otro proceso
        sem_post(mutex);
        sem_post(llenas);

        //Simulamos el tiempo de producción
        sleep(rand() % 4);
    }

    //Imprimimos el buffer local
    printf("Se han producido las letras (buffer local): ");
    for (int i = 0; i < strlen(arrayLocal); i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El productor ha terminado.\n");

    //Liberamos recursos
    munmap(map, sizeof(sharedData));
    close(fd);
    free(arrayLocal);

    //Cerramos y eliminamos los semáforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    return 0;
}