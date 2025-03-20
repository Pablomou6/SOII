/*
Las funciones sleep() y wakeup() las puedes implementar con señales o con semáforos o susti-
tuirlas por espera activa.
*/

//utilizando señales
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//wake up debe ser de forma que el 
//productor pueda despertar al consumidor y viceversa

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void sleep_() {
    //esta función debe ser llamada por el productor y el consumidor
    //para dormirse hasta que el otro los despierte
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

void wakeup_() {
    //esta función debe ser llamada por el productor y el consumidor
    //para despertar al otro
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}   
