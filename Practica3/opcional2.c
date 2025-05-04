#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

//Output en un mismo archivo (xuraría que solo escribe 2 asteriscos con 3 Productores, ns se nn o escribe ou como, pero detéctao)

// Variables globales
char *buffer;
int inicio = 0, final = 0, numElementos = 0; // LLena si ambas variables coinciden (final apunta al último elemento)
int nAsteriscos = 0;
int P, C, N; 

pthread_mutex_t mutex, scanner;
pthread_barrier_t barrier;


void insertarElemento(char elemento, int T) {
    // Insertamos el elemento en el buffer por el final
    buffer[final] = elemento;
    // Recalculamos el índice del final
    final = (final + 1) % N;
    // Aumentamos el contador de elementos
    numElementos += 1;
}

char eliminarElemento(int T){
    // Recuperamos el elemento del buffer por el inicio
    char elemento = buffer[inicio];
    // Recalculamos el índice del inicio
    inicio = (inicio + 1) % N;
    // Disminuimos el contador de elementos
    numElementos -= 1;
    // Devolvemos el elemento recuperado
    return elemento;
}

// Función que se asocia a los hilos consumidores
void* consumidor(void* i){
    int T;
    int id = *(int *)i;
    char nombre[50];
    sprintf(nombre, "salida%d.txt", id);    // Guarda en nombre el string formateado

    // Abrimos el archivo de salida
    FILE* salida = fopen(nombre, "w");
    if(salida == NULL){
        printf("Error al crear el archivo de salida %s\n", nombre);
        exit(3);    // Si falla la lectura de algún archivo, se detiene el proceso
    }

    // Solicitamos el retardo al consumidor
    pthread_mutex_lock(&scanner);
    printf("(Cons %d) Introduce el retardo T entre cada iteración: ", id);
    scanf("%d", &T);
    pthread_mutex_unlock(&scanner);

    // Esperamos a todos los hilos con la barrera
    pthread_barrier_wait(&barrier);
    printf("BARRERA CONSUMIDOR\n");

    char elemento;
    while(1){
        // Comprobamos si el buffer está vacío. En caso de estarlo, espera al productor
        // Tenemos que comprobarlo dentro del mutex para evitar carreras críticas
        do{
            pthread_mutex_lock(&mutex);
            if(numElementos != 0 || (numElementos == 0 && nAsteriscos == P)){
                break;
            }
            pthread_mutex_unlock(&mutex);
            sched_yield();      // Cede la CPU por si otro hilo quisiese intentar ejecutarse, para dejar tiempo hasta que se cumpla la condición
        }while(1);
        
        // Comprobamos si estamos en el caso  de que se hayan leído todos los caracteres del archivo
        if((nAsteriscos == P) && numElementos == 0) {
            // En caso de terminar la ejecución, se desbloquea el mutex y se sale del bucle
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Si hay elementos en el buffer, eliminamos uno
        elemento = eliminarElemento(T);        

        // Si el elemento a insertar en el archivo es un asterisco
        if(elemento == '*') {

            // Aumentamos el contador de asteriscos
            nAsteriscos++;

            // Como ya no hay variables de condición, sino espera activa, no hace falta el broadcast
        }
        // Desbloqueamos el mutex
        pthread_mutex_unlock(&mutex);

        printf("(Cons %d) Elimina el elemento %c\n", id, elemento);

        // Escribir el elemento en el archivo
        fputc(elemento, salida);

        // Esperamos un tiempo aleatorio entre 0 y T para simular el retardo
        sleep(rand() % T);
    }
    // Cerramos el archivo de salida y termina el hilo
    fclose(salida);
    pthread_exit(0);
}

// Función que comrueba si un carácter es alfanumérico
int esAlfanumerico(char c) {
    int a = (int)c;
    if((a >= 48 && a <= 57) || (a >= 65 && a <= 90) || (a >= 97 && a <= 122)) return 1;
    
    return 0;
}

void* productor(void* i){
    int id = *(int*)i, T;
    char archivo[20], elemento;

    // Solicitamos el nombre del archivo para leer
    pthread_mutex_lock(&scanner);
    printf("(Prod %d): Introduce el nombre del archivo del que se leerá el texto: ", id);
    scanf(" %[^\n\r]", archivo);
    printf("\n");
    pthread_mutex_unlock(&scanner);

    // Intentamos abrir el archivo
    FILE* doc = fopen(archivo, "r");
    if(doc == NULL) {
        printf("Error al abrir el archivo %s\n", archivo);
        exit(4);    // Si falla la lectura de algún archivo, se detiene el proceso
    }

    // Solicitamos el retardo para el productor
    pthread_mutex_lock(&scanner);
    printf("(Prod %d): Introduce el retardo máximo deseado para las esperas: ", id);
    scanf(" %d", &T);
    printf("\n");
    pthread_mutex_unlock(&scanner);

    // Esperamos al resto de hilos usando la barrera
    pthread_barrier_wait(&barrier);
    printf("BARRERA PRODUCTOR\n");
    
    // Mientras no se alcanza el final del archivo, leemos el siguiente elemento
    while((elemento = fgetc(doc)) != EOF){
        // Comprobamos si el elemento es alfanumérico. En caso de no serlo, se ignora
        if(esAlfanumerico(elemento)) {
            // Comprobamos si el buffer está lleno. En caso de estarlo, espera al consumidor
            // Tenemos que comprobarlo dentro del mutex para evitar carreras críticas
            do{
                pthread_mutex_lock(&mutex);
                if(numElementos < N){
                    break;
                }
                pthread_mutex_unlock(&mutex);
                sched_yield();  // Cede la CPU por si otro hilo quisiese intentar ejecutarse, para dejar tiempo hasta que se cumpla la condición
            }while(1);

            // Si el buffer tiene hueco, se inserta el elemento
            insertarElemento(elemento, T);

            // Desbloqueamos el mutex
            pthread_mutex_unlock(&mutex);

            printf("(Prod %d) Inserta el elemento %c\n", id, elemento);

            // Tiempo de espera aleatorio entre 0 y T-1
            sleep(rand() % T);
        }
    }

    // Comprobamos si el buffer está lleno para insertar el asterisco
    do{
        pthread_mutex_lock(&mutex);
        if(numElementos != N){
            break;
        }
        pthread_mutex_unlock(&mutex);
        sched_yield();  // Cede la CPU por si otro hilo quisiese intentar ejecutarse, para dejar tiempo hasta que se cumpla la condición
    }while(1);

    // Si el buffer tiene hueco, se inserta un asterisco
    insertarElemento('*', T);
    
    // Desbloqueamos el mutex
    pthread_mutex_unlock(&mutex);

    // Cerramos el archivo y terminamos el hilo
    fclose(doc);
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    
    // Comprobar los parámetros por línea de comandos: P C N
    if(argc != 4){
        printf("Número de argumentos incorrecto. Debe ser: P C N\n");
        exit(1);
    }

    P = atoi(argv[1]);  // Número de hilos productores
    C = atoi(argv[2]);  // Número de hilos consumidores
    N = atoi(argv[3]);  // Número de posiciones del buffer

    
    
    // Reserva de memoria para el buffer compartido
    buffer = (char *)malloc(N*sizeof(char));
    
    // Variables para almacenar los identificadores de los hilos
    pthread_t productores[P], consumidores[C];
    int indicesProd[P], indicesCons[C];

    // Creación del mutex y variables de condición
    pthread_mutex_init(&mutex, 0);
    pthread_mutex_init(&scanner, 0);
    
    // Creación de la barrera
    if(pthread_barrier_init(&barrier, NULL, P+C) != 0){
        printf("Error al crear la barrera\n");
        exit(2);
    }

    // Creación de los hilos
    for(int i = 0; i < P; i++){
        indicesProd[i] = i;
        pthread_create(&productores[i], NULL, productor, (void *)&indicesProd[i]);
    }

    for(int i = 0; i < C; i++){
        indicesCons[i] = i; 
        pthread_create(&consumidores[i], NULL, consumidor, (void *)&indicesCons[i]);
    }

    // Esperar a que terminen todos los hilos
    for(int i = 0; i < P; i++){
        pthread_join(productores[i], NULL);
        printf("Termina el productor %d\n", i);
    }

    for(int i = 0; i < C; i++){
        pthread_join(consumidores[i], NULL);
        printf("Termina el consumidor %d\n", i);
    }

    // Liberamos los mutex creados y el buffer compartido
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&scanner);
    free(buffer);

    return EXIT_SUCCESS;
}
