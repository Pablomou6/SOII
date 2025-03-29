#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdbool.h>

#define N 8 

bool boolean = true;
bool sigint_received = false; // Variable global para indicar que se recibió SIGINT

//Compartiremos entre los procesos la estructura sharedData
/*
    Esta estructura contiene varios datos:
    - array: Es el buffer compartido entre los procesos. Es un array de char's de tamaño N.
    - numElementos: Es el número de elementos que hay en el buffer.
    - pidProd: Es el PID del productor.
    - pidCons: Es el PID del consumidor.
*/
typedef struct {
    char array[N];
    int numElementos;
    int pidProd, pidCons;
} sharedData;

//Esta función se encarga de eliminar el elemento tope del buffer
char remove_item(sharedData* info) {
    //Devolvemos el elemento que está en la última posición ocupada. (Dado que almacenamos el número de elementos, cuando tengamos 5 elementos,
    //las posiciones ocupadas serán de 0 a 4, por lo que debemos devolver numElementos - 1)
    return info->array[info->numElementos - 1];
}

//Esta función se encarga de almacenar el item consumido en un array local
void consume_item(char item, char arrayLocal[], int* indexLocal) {
    arrayLocal[*indexLocal] = item;
    (*indexLocal)++;
}

void signal_handler(int senhal) {
    //No tiene que tener nada
}

void sigusr2_handler(int senhal) {
    if (senhal == SIGUSR2) {
        printf("Consumidor: Recibida señal SIGUSR2. Terminando...\n");
        boolean = false;
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if(argc != 2) {
        printf("Debe introducir un int para el número de segundos a dormir\n");
        return EXIT_FAILURE;
    }

    int fd = 0, indexLocal = 0;
    sharedData* info;
    char arrayLocal[1000];
    char item;
    int segundos = atoi(argv[1]);

    //Abrimos el archivo compartido. En caso de no existir, lo crea.
    fd = open("archivoCompartido.txt", O_RDWR | O_CREAT, 0666);
    if(fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    //Truncamos el archivo al tamaño de la estructura
    if(ftruncate(fd, sizeof(sharedData)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //Mapeamos el archivo, pero casteándolo a la estructura sharedData
    /*
        Lo que estamos haciendo es crear un espacio de memoria compartida entre los procesos al mapear el archivo. 
        Sin embargo, al hacer un cast a la estructura, lo que hace realmente es interpretar a la estructura como si fuese el archivo,
        quedando así l contenido de la estructura en el archvio.
    */
    info = (sharedData*)mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(info == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //Cerramos el archivo
    close(fd);

    //Una vez tenemos la memoria compartida creada y ambos procesos accedieron a ella, debemos conocer los PIDs de ambos procesos.
    info->pidCons = getpid();
    printf("PID del consumidor almacenado: %d\n", info->pidCons);
    


    //Configuramos el manejador de señales
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, sigusr2_handler);


    while(boolean) {
        //Si el buffer está vacío, se hace espera activa
        while(info->numElementos == 0) {
            
        }

        //Insertamos un sleep para forzar la carrera crítica. (Se debe a que, si el productor está en proceso de escribir un item, 
        //la actualización del tope de la pila no se verá reflejada en el consumidor hasta que el productor termine de escribir)
        //sleep(segundos);

        //Sacamos el elemento tope del buffer (no se actualiza el tope en la función)
        item = remove_item(info);

        //Añadimos un sleep para forzar la carrera crítica (Se debe a que, se ha consumido el item, pero no se ha actualizado el tope del buffer; por lo
        //que el productor puede leer el buffer antes de que el consumidor lo haya actualizado)
        sleep(segundos);

        //Decrementamos el número de elementos
        info->numElementos--;
        printf("Consumidor: Saca %c, numElementos (ahora): %d\n", item, info->numElementos);

        if(info->numElementos == N - 1) {
            //Enviar señal al productor si el buffer estaba lleno, interrumpiendo la espera activa
            kill(info->pidProd, SIGUSR1); 
        }

        //Consumimos el item
        consume_item(item, arrayLocal, &indexLocal);
    }

    //Una vez se recibe la señal SIGINT, se imprime el array local
    printf("Se han consumido las letras (array local): ");
    for (int i = 0; i < indexLocal; i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El consumidor ha terminado.\n");
    
    //Desmapeamos el archivo
    if(munmap(info, sizeof(sharedData)) == -1) {
        perror("Error al desmapear el archivo");
        exit(EXIT_FAILURE);
    }

    return 0;
}