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
pthread_cond_t condc, condp;
pthread_barrier_t barrier;


void insertarElemento(char elemento, int T) {
    //sleep(rand() % T);

    buffer[final] = elemento;
    final = (final + 1) % N;
    numElementos += 1;
}

char eliminarElemento(int T){
    //sleep(rand() % T);

    char elemento = buffer[inicio];
    inicio = (inicio + 1) % N;
    numElementos -= 1;
    
    return elemento;
}

void* consumidor(void* i){
    int T;
    int id = *(int *)i;
    char nombre[50];
    sprintf(nombre, "salida%d.txt", id);    // Guarda en nombre el string formateado

    FILE* salida = fopen(nombre, "w");
    if(salida == NULL){
        printf("Error al crear el archivo de salida %s\n", nombre);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&scanner);
    printf("(Cons %d) Introduce el retardo T entre cada iteración: ", id);
    scanf("%d", &T);
    pthread_mutex_unlock(&scanner);

    pthread_barrier_wait(&barrier);
    printf("BARRERA CONSUMIDOR\n");

    char elemento;
    while(nAsteriscos != P){
        pthread_mutex_lock(&mutex);
        while(numElementos == 0){
            pthread_cond_wait(&condc, &mutex);
        }
        
        elemento = eliminarElemento(T);

        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&mutex);
        printf("(Cons %d) Elimina el elemento %c\n", id, elemento);

        // Escribir el elemento en el archivo
        fputc(elemento, salida);
        if(elemento == '*') {
            nAsteriscos++;
        }
    }

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

    pthread_mutex_lock(&scanner);
    printf("(Prod %d): Introduce el nombre del archivo del que se leerá el texto: ", id);
    scanf(" %[^\n\r]", archivo);
    printf("\n");
    pthread_mutex_unlock(&scanner);

    FILE* doc = fopen(archivo, "r");
    if(doc == NULL) {
        printf("Error al abrir el archivo %s\n", archivo);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&scanner);
    printf("(Prod %d): Introduce el retardo máximo deseado para las esperas: ", id);
    scanf(" %d", &T);
    printf("\n");
    pthread_mutex_unlock(&scanner);

    // Espera usando la barrera
    pthread_barrier_wait(&barrier);
    printf("BARRERA PRODUCTOR\n");
    
    while((elemento = fgetc(doc)) != EOF){
        if(esAlfanumerico(elemento)) {
            pthread_mutex_lock(&mutex);

            while(numElementos != 0) {
                pthread_cond_wait(&condp, &mutex);
            }

            insertarElemento(elemento, T);
            
            pthread_cond_signal(&condc);

            pthread_mutex_unlock(&mutex);
            printf("(Prod %d) Inserta el elemento %c\n", id, elemento);
        }
    }

    pthread_mutex_lock(&mutex);

    while(numElementos != 0) {
        pthread_cond_wait(&condp, &mutex);
    }

    insertarElemento('*', T);
    
    pthread_cond_signal(&condc);

    pthread_mutex_unlock(&mutex);

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
    pthread_t productores[P], consumidores[C], indicesProd[P], indicesCons[C];

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

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&scanner);
    pthread_cond_destroy(&condp);
    pthread_cond_destroy(&condc);
    free(buffer);

    return EXIT_SUCCESS;
}
