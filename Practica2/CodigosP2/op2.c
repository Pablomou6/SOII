#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>

#define N 8             // tamaño del buffer
#define ITERACIONES 60  // numero de iteraciones por productor

// memoria compartida
char* buffer;
int* nElementosBuffer;

// semaforos
sem_t *vacias, *llenas, *mutex;

// funcion para producir un elemento
void produce_item(char* elemento) {
    *elemento = 'A' + rand() % 26; // generar letra aleatoria
}

// funcion para insertar un elemento en el buffer
void insert_item(char elemento) {
    buffer[*nElementosBuffer] = elemento;
    (*nElementosBuffer)++;
}

// funcion para retirar un elemento del buffer
char remove_item() {
    (*nElementosBuffer)--;
    return buffer[*nElementosBuffer];
}

// funcion del proceso productor
void productor(int id) {
    char elemento;

    srand(time(NULL) + id); // semilla aleatoria unica para cada proceso

    for (int i = 0; i < ITERACIONES; i++) {
        // producir un elemento
        produce_item(&elemento);
        printf("(prod %d) Elemento generado: %c\n", id, elemento);

        // esperar a que haya espacio en el buffer
        sem_wait(vacias);
        sem_wait(mutex);

        // insertar el elemento en el buffer
        insert_item(elemento);
        printf("(prod %d) Elemento insertado en el buffer\n", id);

        sem_post(mutex);
        sem_post(llenas);

        // simular tiempo de produccion
        sleep(rand() % 4);
    }

    exit(0);
}

// funcion del proceso consumidor
void consumidor(int id) {
    char elemento;

    for (int i = 0; i < ITERACIONES; i++) {
        // esperar a que haya elementos en el buffer
        sem_wait(llenas);
        sem_wait(mutex);

        // retirar un elemento del buffer
        elemento = remove_item();
        printf("(cons %d) Elemento retirado del buffer: %c\n", id, elemento);

        sem_post(mutex);
        sem_post(vacias);

        // simular tiempo de consumo
        sleep(rand() % 4);
    }

    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <num_productores> <num_consumidores>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int numProductores = atoi(argv[1]);
    int numConsumidores = atoi(argv[2]);

    if (numProductores <= 0 || numConsumidores <= 0) {
        fprintf(stderr, "El numero de productores y consumidores debe ser mayor que 0.\n");
        exit(EXIT_FAILURE);
    }

    // abrir archivo para lectura y escritura
    int fd = open("archivoCompartido.txt", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // truncar el archivo al tamaño del buffer
    if (ftruncate(fd, N * sizeof(char) + sizeof(int)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // mapear el archivo en memoria compartida
    void* mem_compartida = mmap(NULL, N * sizeof(char) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem_compartida == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    buffer = (char*)mem_compartida;
    nElementosBuffer = (int*)(mem_compartida + N * sizeof(char));
    *nElementosBuffer = 0;

    // inicializar semaforos
    vacias = sem_open("/vacias", O_CREAT, 0666, N);  // espacios vacios en el buffer
    llenas = sem_open("/llenas", O_CREAT, 0666, 0);  // elementos llenos en el buffer
    mutex = sem_open("/mutex", O_CREAT, 0666, 1);    // exclusion mutua

    if (vacias == SEM_FAILED || llenas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Error al crear los semaforos");
        exit(EXIT_FAILURE);
    }

    // crear procesos productores
    for (int i = 0; i < numProductores; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            productor(i + 1);
        } else if (pid < 0) {
            perror("Error al crear el proceso productor");
            exit(EXIT_FAILURE);
        }
    }

    // crear procesos consumidores
    for (int i = 0; i < numConsumidores; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            consumidor(i + 1);
        } else if (pid < 0) {
            perror("Error al crear el proceso consumidor");
            exit(EXIT_FAILURE);
        }
    }

    // esperar a que todos los procesos terminen
    for (int i = 0; i < numProductores + numConsumidores; i++) {
        wait(NULL);
    }

    // destruir semaforos
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);
    sem_unlink("/vacias");
    sem_unlink("/llenas");
    sem_unlink("/mutex");

    // liberar memoria compartida
    munmap(mem_compartida, N * sizeof(char) + sizeof(int));
    close(fd);

    return 0;
}