//Diego Pérez Álvarez, Pablo Mouriño Lorenzo

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>

//Constantes para el tamaño del nombre del archivo
#define MAX 1000
#define MAX_FILE_NAME 100

//Estructura para datos compartidos
typedef struct {
    char* buffer; //Buffer compartido
    int inicio, final, numElementos; //Indices para el buffer
    int finProductores; //Contador de productores que han terminado
    int N, P, C; //Variables para el tamaño del buffer y el número de productores y consumidores
    int contadorBarrera; //Contador para la barrera
    //Mutex, para la sincronización de los procesos
    pthread_mutex_t mutex;
    pthread_mutex_t io_mutex;
    //Variables de condición para la sincronización
    pthread_cond_t condp;
    pthread_cond_t condc;
    //Variable de condición para la barrera
    pthread_cond_t condBarrera;
} SharedData;

//Función que comrueba si un carácter es alfanumérico
int esAlfanumerico(char c) {
    int a = (int)c;
    if((a >= 48 && a <= 57) || (a >= 65 && a <= 90) || (a >= 97 && a <= 122)) return 1;
    
    return 0;
}

//Función que implementa la barrera
void barrera(SharedData *data, int totalProcesos) {
    //Bloqueamos el mutex para que todos los procesos puedan acceder a la barrera
    pthread_mutex_lock(&data->mutex);

    //Aumentamos el contador de la barrera a medida que los procesos llegan
    data->contadorBarrera++;

    //Si el contador de la barrera es menor que el número total de procesos, esperamos a que todos lleguen
    if(data->contadorBarrera < totalProcesos) {
        //Esperar a que todos los procesos lleguen a la barrera
        pthread_cond_wait(&data->condBarrera, &data->mutex);
    } 
    else {
        //Último proceso libera a todos
        data->contadorBarrera = 0;
        pthread_cond_broadcast(&data->condBarrera);
    }

    //Desbloqueamos el mutex 
    pthread_mutex_unlock(&data->mutex);
}

//Función que inserta un elemento en el buffer
void insertarElemento(SharedData *data, char elemento) {
    //Insertamos el elemento en el buffer
    data->buffer[data->final] = elemento;
    //Aumentamos el índice del final del buffer y el número de elementos
    data->final = (data->final + 1) % data->N;
    data->numElementos++;
}

//Función que elimina un elemento del buffer
char eliminarElemento(SharedData *data) {
    //Recuperamos el elemento del buffer
    char elemento = data->buffer[data->inicio];
    //Aumentamos el índice del inicio del buffer y disminuimos el número de elementos
    data->inicio = (data->inicio + 1) % data->N;
    data->numElementos--;
    //Devolvemos el elemento
    return elemento;
}

//Función que implementa el productor
void productor(SharedData *data) {
    char file_name[MAX_FILE_NAME];
    int T_local;

    //Bloqueamos el mutex para que el productor pueda acceder a la entrada/salida
    pthread_mutex_lock(&data->io_mutex);
    printf("[Productor %d] Introduzca el nombre del archivo de entrada: ", getpid());
    scanf("%s", file_name);
    printf("[Productor %d] Introduzca el retardo T (en segundos): ", getpid());
    scanf("%d", &T_local);
    //Desbloqueamos el mutex para que otros procesos puedan acceder a la entrada/salida
    pthread_mutex_unlock(&data->io_mutex);

    //Abrimos el archivo de entrada
    FILE *fp = fopen(file_name, "r");
    if(!fp) {
        perror("Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Esperamos a que todos los procesos lleguen a la barrera
    barrera(data, data->P + data->C);

    srand(getpid());
    char c;
    while((c = fgetc(fp)) != EOF) {
        if(!esAlfanumerico(c)) {
            //Si el carácter no es alfanumérico, lo ignoramos
            continue;
        }

        //Bloqueamos el mutex para que el productor pueda acceder al buffer
        pthread_mutex_lock(&data->mutex);

        //Esperamos a que haya espacio en el buffer
        while(data->numElementos == data->N) {
            pthread_cond_wait(&data->condp, &data->mutex);
        }

        //Insertamos el elemento en el buffer
        insertarElemento(data, c);
        
        //Desbloqueamos el mutex y notificamos al consumidor
        pthread_cond_signal(&data->condc);
        pthread_mutex_unlock(&data->mutex);
        printf("[Productor %d] Inserta el elemento %c\n", getpid(), c);

        //Esperamos un tiempo aleatorio antes de insertar el siguiente elemento
        sleep(rand() % (T_local));
    }

    //Ceramos el archivo
    fclose(fp);

    //Bloqueamos el mutex para que el productor pueda acceder al buffer
    pthread_mutex_lock(&data->mutex);

    //Esperamos a que haya espacio en el buffer
    while(data->numElementos == data->N) {
        pthread_cond_wait(&data->condp, &data->mutex);
    }

    //Insertamos el elemento '*' en el buffer para indicar el fin del productor
    insertarElemento(data, '*');

    //Desbloqueamos el mutex y notificamos al consumidor
    pthread_cond_signal(&data->condc);
    pthread_mutex_unlock(&data->mutex);

    exit(0);
}

void consumidor(SharedData *data, int id) {
    char file_name[MAX_FILE_NAME];
    int T_local;

    //Bloqueamos el mutex para que el consumidor pueda acceder a la entrada/salida
    pthread_mutex_lock(&data->io_mutex);
    printf("[Consumidor %d] Introduzca el retardo T (en segundos): ", getpid());
    scanf("%d", &T_local);
    //Desbloqueamos el mutex para que otros procesos puedan acceder a la entrada/salida
    pthread_mutex_unlock(&data->io_mutex);

    //Generamos el nombre del archivo de salida
    sprintf(file_name, "salida%d.txt", id);
    //Abrimos el archivo de salida
    FILE *fp = fopen(file_name, "w");
    if(!fp) {
        perror("Error creando el archivo");
        exit(EXIT_FAILURE);
    }

    //Esperamos a que todos los procesos lleguen a la barrera
    barrera(data, data->P + data->C);

    srand(getpid());

    while (1) {
        //Bloqueamos el mutex para que el consumidor pueda acceder al buffer
        pthread_mutex_lock(&data->mutex);

        //Esperamos a que haya elementos en el buffer y además que no se hayan terminado todos los productores
        while(data->numElementos == 0 && !(data->finProductores == data->P)) {
            pthread_cond_wait(&data->condc, &data->mutex);
        }

        //Volvemos a comprobar ya que, si algún consumidor se queda bloqueado en el bloque superior, ejecutaría de nuevo el resto del código
        if((data->finProductores == data->P) && data->numElementos == 0) {
            //En caso de terminar la ejecución, se desbloquea el mutex y se sale del bucle
            pthread_mutex_unlock(&data->mutex);
            break;
        }
        
        char c;
        //Eliminamos el elemento del buffer
        c = eliminarElemento(data);

        //Desbloqueamos el mutex y notificamos al productor
        pthread_cond_signal(&data->condp);
        pthread_mutex_unlock(&data->mutex);
        printf("[Consumidor %d] Elimina el elemento %c\n", id, c);

        //Escribimos el elemento en el archivo de salida
        fputc(c, fp);

        //Si el elemento es '*', significa que el productor ha terminado
        if(c == '*') {
            //Bloqueamos el mutex 
            pthread_mutex_lock(&data->mutex);
            //Aumentamos el contador de productores que han terminado
            data->finProductores++;

            //Si todos los productores han terminado, notificamos a los consumidores
            if(data->finProductores == data->P) {
                pthread_cond_broadcast(&data->condc);
            }

            //Desbloqueamos el mutex
            pthread_mutex_unlock(&data->mutex);
        }

        //Esperamos un tiempo aleatorio antes de eliminar el siguiente elemento
        sleep(rand() % (T_local));
    }

    //Cerramos el archivo de salida
    fclose(fp);
    exit(0);
}

int main(int argc, char *argv[]) {

    //Comprobamos que se han introducido los argumentos necesarios
    if(argc != 4) {
        fprintf(stderr, "Uso: %s <num_productores> <num_consumidores> <tam_buffer>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Convertimos los argumentos a enteros
    int P = atoi(argv[1]);
    int C = atoi(argv[2]);
    int N = atoi(argv[3]);

    //Mapeamos la memoria compartida para escribir y leer, especificando que es anónima y compartida
    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //Inicializamos los valores de la estructura de datos compartidos
    data->inicio = data->final = data->numElementos = 0;
    data->finProductores = 0;
    data->N = N;
    data->P = P;
    data->C = C;
    data->contadorBarrera = 0;

    //Reservamos memoria para el buffer compartido
    data->buffer = mmap(NULL, N * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(data->buffer == MAP_FAILED) {
        perror("Error reservando memoria para el buffer.\n");
        exit(EXIT_FAILURE);
    }

    //Variables para los atributos de los mutex y las variables de condición
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    //Establecemos loas atributos para que se puedan compartir entre procesos los mutex y las variables de condición
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    //Inicializamos los mutex y las variables de condición con los atributos establecidos
    pthread_mutex_init(&data->mutex, &mattr);
    pthread_mutex_init(&data->io_mutex, &mattr);
    pthread_cond_init(&data->condp, &cattr);
    pthread_cond_init(&data->condc, &cattr);
    pthread_cond_init(&data->condBarrera, &cattr);

    //Creamos los procesos productores
    for(int i = 0; i < P; i++) {
        pid_t pid = fork();
        if(pid == 0) {
            productor(data);
        }
    }

    //Creamos los procesos consumidores
    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            consumidor(data, i);
        }
    }

    //Esperamos a que todos los procesos terminen
    for (int i = 0; i < P + C; i++) {
        wait(NULL);
    }

    //Liberamos los recursos
    pthread_mutex_destroy(&data->mutex);
    pthread_mutex_destroy(&data->io_mutex);
    pthread_cond_destroy(&data->condp);
    pthread_cond_destroy(&data->condc);
    pthread_cond_destroy(&data->condBarrera);
    munmap(data->buffer, N * sizeof(char));
    munmap(data, sizeof(SharedData));

    return 0;
}
