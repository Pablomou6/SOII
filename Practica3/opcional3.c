#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

//Output en un mismo archivo (xuraría que solo escribe 2 asteriscos con 3 Productores, ns se nn o escribe ou como, pero detéctao)

// Variables globales
char *buffer, *bufferFinal;
int inicio = 0, final = 0, numElementos = 0; // LLena si ambas variables coinciden (final apunta al último elemento)
int inicioFinal = 0, finalFinal = 0, numElementosFinal = 0;
int nAsteriscos = 0;
int P, C, D, N, M;

int suma = 0;

pthread_mutex_t mutex, scanner, nAsterisco, mutexFinal;
pthread_cond_t condc, condp, condcFinal, condpFinal;
pthread_barrier_t barrier;


void insertarElemento(char **b, char elemento) {
    // Insertamos el elemento en el buffer por el final
    (*b)[final] = elemento;
    // Recalculamos el índice del final
    numElementos += 1;
    // Aumentamos el contador de elementos
    final = (final + 1) % N;
}

void insertarElementoFinal(char **b, char elemento) {
    // Insertamos el elemento en el buffer por el final
    (*b)[finalFinal] = elemento;
    // Recalculamos el índice del final
    numElementosFinal += 1;
    // Aumentamos el contador de elementos
    finalFinal = (finalFinal + 1) % M;
}

char eliminarElemento(char **b){
    // Recuperamos el elemento del buffer por el inicio
    char elemento = (*b)[inicio];
    // Recalculamos el índice del inicio
    inicio = (inicio + 1) % N;
    // Disminuimos el contador de elementos
    numElementos -= 1;
    // Devolvemos el elemento recuperado
    return elemento;
}

char eliminarElementoFinal(char **b){
    // Recuperamos el elemento del buffer por el inicio
    char elemento = (*b)[inicioFinal];
    // Recalculamos el índice del inicio
    inicioFinal = (inicioFinal + 1) % M;
    // Disminuimos el contador de elementos
    numElementosFinal -= 1;
    // Devolvemos el elemento recuperado
    return elemento;
}

// Función que comrueba si un carácter es alfanumérico
int esNumerico(char c) {
    int a = (int)c;
    if(a >= 48 && a <= 57) return 1;
    
    return 0;
}

// Función que se asocia a los hilos consumidores FINALES
void* consumidorFinal(void* i){
    int id = *(int *)i;

    char elemento;
    while(1){
        // Bloqueamos el mutex para acceder a las variables compartidas
        pthread_mutex_lock(&mutexFinal);

        // Si no hay ningún elemento en el buffer y no se han leído todos los elementos, espera usando la variable de condiciónç
        // Solo se cuentan asteriscos una vez se han pasado al buffer final, así que la comprobación es correcta
        while(numElementosFinal == 0 && !(nAsteriscos == P)) {
            pthread_cond_wait(&condcFinal, &mutexFinal);
        }

        // Volvemos a comprobar ya que, si algún consumidor se queda bloqueado en el bloque superior, ejecutaría de nuevo el resto del código
        if((nAsteriscos == P) && numElementosFinal == 0) {
            // En caso de terminar la ejecución, se desbloquea el mutex y se sale del bucle
            pthread_mutex_unlock(&mutexFinal);
            break;
        }
        
        // Si hay elementos en el buffer, eliminamos uno
        elemento = eliminarElementoFinal(&bufferFinal);

        suma += atoi(&elemento);     // Como solo se accede dentro de la región crítica, no hay carreras críticas

        // Desbloqueamos el mutex y notificamos al productor
        pthread_cond_signal(&condpFinal);
        pthread_mutex_unlock(&mutexFinal);
        printf("(Cons Final %d) Elimina el elemento %c de FINAL\n", id, elemento);

    }
    // Termina el hilo
    pthread_exit(0);
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
        exit(3);
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
        // Bloqueamos el mutex para acceder a las variables compartidas
        pthread_mutex_lock(&mutex);

        // Si no hay ningún elemento en el buffer y no se han leído todos los elementos, espera usando la variable de condición
        while(numElementos == 0 && !(nAsteriscos == P)) {
            pthread_cond_wait(&condc, &mutex);
        }

        // Volvemos a comprobar ya que, si algún consumidor se queda bloqueado en el bloque superior, ejecutaría de nuevo el resto del código
        if((nAsteriscos == P) && numElementos == 0) {
            // En caso de terminar la ejecución, se desbloquea el mutex y se sale del bucle
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Si hay elementos en el buffer, eliminamos uno
        elemento = eliminarElemento(&buffer);

        // Desbloqueamos el mutex y notificamos al productor
        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&mutex);
        printf("(Cons %d) Elimina el elemento %c\n", id, elemento);
 
        // Proceso de productor, pero para el buffer de Salida
        // Comprobamos si el elemento es numérico. En caso de no serlo, se ignora
        if(esNumerico(elemento)) {
            // Bloqueamos el mutex para acceder a las variables compartidas
            pthread_mutex_lock(&mutexFinal);

            // Comprobamos si el buffer está lleno. En caso de estarlo, espera el productor
            while(numElementosFinal == M) {
                pthread_cond_wait(&condpFinal, &mutexFinal);
            }

            // Si el buffer tiene hueco, se inserta el elemento
            insertarElementoFinal(&bufferFinal, elemento);

            // Desbloqueamos el mutex y notificamos al consumidor
            pthread_cond_signal(&condcFinal);
            pthread_mutex_unlock(&mutexFinal);

            printf("(Cons %d) Inserta el elemento %c en FINAL\n", id, elemento);
        }

        // Escribir el elemento en el archivo
        fputc(elemento, salida);

        // Si el elemento a insertar en el archivo es un asterisco
        if(elemento == '*') {
            // Bloqueamos el mutex para acceder a la variable nAsteriscos
            pthread_mutex_lock(&nAsterisco);
            // Aumentamos el contador de asteriscos
            nAsteriscos++;

            // Si hemos recibido todos los asteriscos, notificamos a los consumidores
            // Para los consumidores finales también, ya que al llegar a este punto los número ya están todos insertamos en su buffer
            if(nAsteriscos == P) {
                pthread_cond_broadcast(&condc);
                pthread_cond_broadcast(&condcFinal);
            }
            // Desbloqueamos el mutex
            pthread_mutex_unlock(&nAsterisco);
        }
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
        exit(4);
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
            // Bloqueamos el mutex para acceder a las variables compartidas
            pthread_mutex_lock(&mutex);

            // Comprobamos si el buffer está lleno. En caso de estarlo, espera el productor
            while(numElementos == N) {
                pthread_cond_wait(&condp, &mutex);
            }

            // Si el buffer tiene hueco, se inserta el elemento
            insertarElemento(&buffer, elemento);

            // Desbloqueamos el mutex y notificamos al consumidor
            pthread_cond_signal(&condc);
            pthread_mutex_unlock(&mutex);

            printf("(Prod %d) Inserta el elemento %c\n", id, elemento);

            // Tiempo de espera aleatorio entre 0 y T-1
            sleep(rand() % T);
        }
    }

    // Al terminar la lectura del archivo, se bloquea el mutex
    pthread_mutex_lock(&mutex);

    // Se comprueba si el buffer está lleno. En caso de estarlo, espera el productor
    while(numElementos == N) {
        pthread_cond_wait(&condp, &mutex);
    }

    // Si el buffer tiene hueco, se inserta un asterisco
    insertarElemento(&buffer, '*');
    
    // Desbloqueamos el mutex y notificamos al consumidor
    pthread_cond_signal(&condc);
    pthread_mutex_unlock(&mutex);

    // Cerramos el archivo y terminamos el hilo
    fclose(doc);
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    
    // Comprobar los parámetros por línea de comandos: P C N
    if(argc != 6){
        printf("Número de argumentos incorrecto. Debe ser: P C D N M\n");
        exit(1);
    }

    P = atoi(argv[1]);  // Número de hilos productores
    C = atoi(argv[2]);  // Número de hilos consumidores
    D = atoi(argv[3]);  // Número de hilos consumidores finales
    N = atoi(argv[4]);  // Número de posiciones del buffer
    M = atoi(argv[5]);  // Número de posiciones del buffer Final
     
    
    // Reserva de memoria para el buffer compartido
    buffer = (char *)malloc(N*sizeof(char));

    // Reserva de memoria para el buffer final
    bufferFinal = (char *)malloc(M*sizeof(char));
    
    // Variables para almacenar los identificadores de los hilos
    pthread_t productores[P], consumidores[C], consumidoresFinales[D];
    int indicesProd[P], indicesCons[C], indicesConsFin[D];

    // Creación del mutex y variables de condición
    pthread_mutex_init(&mutex, 0);
    pthread_mutex_init(&scanner, 0);
    pthread_mutex_init(&nAsterisco, 0);
    pthread_mutex_init(&nAsterisco, 0);
    pthread_mutex_init(&mutexFinal, 0);
    
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

    for(int i = 0; i < D; i++){
        indicesConsFin[i] = i; 
        pthread_create(&consumidoresFinales[i], NULL, consumidorFinal, (void *)&indicesConsFin[i]);
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

    for(int i = 0; i < D; i++){
        pthread_join(consumidoresFinales[i], NULL);
        printf("Termina el consumidor final %d\n", i);
    }

    printf("La suma final de caracteres numéricos es: %d\n", suma);

    // Liberamos los mutex creados y el buffer compartido
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&scanner);
    pthread_mutex_destroy(&nAsterisco);
    pthread_mutex_destroy(&mutexFinal);
    pthread_cond_destroy(&condp);
    pthread_cond_destroy(&condc);
    pthread_cond_destroy(&condpFinal);
    pthread_cond_destroy(&condcFinal);
    free(buffer);
    free(bufferFinal);

    return EXIT_SUCCESS;
}
