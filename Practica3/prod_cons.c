#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Variables globales
char *buffer;
int inicio, final; // LLena si ambas variables coinciden
int nAsteriscos = 0;
int P, C, N; 

pthread_mutex_t mutex;
pthread_cond_t condc, condp;

void* consumidor(void* i){
    int id = *(int *)i;
    char nombre[50];
    int T = 0;
    sprintf(nombre, "salida%d.txt", id);    // Guarda en nombre el string formateado

    FILE* salida = fopen(nombre, "w");
    if(salida == NULL){
        printf("Error al crear el archivo de salida %s\n", nombre);
        pthread_exit(NULL);
    }

    printf("Introduce el retardo T entre cada iteración: ");
    scanf(" %d\n", &T);

    while(nAsteriscos != P || inicio != final){
        
    }
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

    printf("Introduce el nombre del archivo del que se leerá el texto: ");
    scanf(" %[^\n\r]", archivo);

    FILE* doc = fopen(archivo, "r");
    if(doc == NULL) {
        printf("Error al abrir el archivo %s\n", archivo);
        pthread_exit(NULL);
    }

    printf("Introduce el retardo máximo deseado para las esperas: ");
    scanf(" %d\n", &T);
    
    while((elemento = fgetc(doc)) != EOF){
        if(esAlfanumerico(elemento)) {

        }
    }

    fclose(doc);
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
    pthread_t productores[P], consumidores[C], indices[C];

    // Creación del mutex y variables de condición
    pthread_mutex_init(&mutex, 0);
    
    // Creación de los hilos
    for(int i = 0; i < P; i++){
        pthread_create(&productores[i], NULL, productor, NULL);
    }

    for(int i = 0; i < C; i++){
        indices[i] = i; 
        pthread_create(&consumidores[i], NULL, consumidor, (void *)&indices[i]);
    }

    // Esperar a que terminen todos los hilos
    for(int i = 0; i < P; i++){
        pthread_join(productores[i], NULL);
    }

    for(int i = 0; i < C; i++){
        pthread_join(consumidores[i], NULL);
    }

    return EXIT_SUCCESS;
}
