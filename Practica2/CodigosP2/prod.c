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
sharedData* info;

//Esta función genera una letra aleatoria de la A a la Z
char produce_item() {
    return 'A' + (rand() % 26);
}

//Función que introduce el elemento en el array compartido
void insert_item(sharedData* info, char item) {
    //Introducimos el item en la posición superior del buffer. Dado que el número de elementos puede ser desde 0 a 8, pero solo entramos en la
    //función cuando el número de elementos es menor que 8, no es necesario comprobar si el buffer está lleno y se introduceen la posición numElementos.
    info->array[info->numElementos] = item;
}

void signal_handler(int senhal) {
    //No tiene que tener nada
}

void sigint_handler(int senhal) {
    if(senhal == SIGINT) {
        printf("Recibida señal SIGINT. Se cambia el valor del bool.\n");
        boolean = false;

        //Enviar señal SIGUSR2 al consumidor para que termine
        kill(info->pidCons, SIGUSR2);
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if(argc != 2) {
        printf("Debe introducir un int para el número de segundos a dormir\n");
        return EXIT_FAILURE;
    }

    int fd = 0, indexLocal = 0;
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
    info->pidProd = getpid();
    printf("PID del productor almacenado: %d\n", info->pidProd);

    info->numElementos = 0;
    info->pidCons = 0;

    //Configuramos el manejador de señales
    signal(SIGUSR1, signal_handler);
    signal(SIGINT, sigint_handler);

    while(boolean) {
        //Generamos un item aleatorio
        item = produce_item();

        //Comprobamos si el tope es 8, por lo que el buffer está lleno (0 a 7) y hacemos una espera activa
        while(info->numElementos == N) {
            
        }

        //Introducimos un sleep para forzar la carrera crítica (Forzamos la carrera crítica porque se aumenta la probabilidad de que
        //el consumidor lea el buffer antes de que el productor lo escriba)
        sleep(segundos);

        //Insertamos el item en el buffer
        insert_item(info, item);

        //Añadimos un sleep para forzar la carrera crítica (Se debe a que, si tarda en actualizar el tope, el consumidor leerá 
        //el buffer antes de que el productor actualice la posición libre)
        sleep(segundos);

        //Aumentamos la posición tope
        info->numElementos++;
        printf("Productor: Letra %c, Contador de buffer (ahora): %d\n", item, info->numElementos);

        //Insertamos el item en el buffer local
        arrayLocal[indexLocal] = item;
        indexLocal++;

        //Ahora, si el buffer está vacío/tiene espacio, se avisa al consumidor
        if(info->numElementos == 1) {
            if(info->pidCons != 0) {
                //Enviar señal al consumidor para que deje de hacer espera activa
                kill(info->pidCons, SIGUSR1);
            }
        }
    }

    //Una vez se recibe la señal SIGINT, se imprime el buffer local
    printf("Se han producido las letras (buffer local): ");
    for(int i = 0; i < indexLocal; i++) {
        printf("%c ", arrayLocal[i]);
    }
    printf("\n");
    printf("El productor ha terminado.\n");

    //Desmapeamos el archivo
    if(munmap(info, sizeof(sharedData)) == -1) {
        perror("Error al desmapear el archivo");
        exit(EXIT_FAILURE);
    }

    return 0;
}