#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mimir.h"

#define N 8

int nElementosBuffer = 0;
int indiceLibre = 0;

void remove_item(void *arrayCompartido, char *elemento){
    //retirar del buffer el caracter que corresponda
    char* buffer = (char*)arrayCompartido;
    *elemento = buffer[nElementosBuffer];
}

void consume_item(char *arrayLocal, char elemento){
    //añadir el caracter retirado del buffer a un string local
    arrayLocal[indiceLibre] = elemento;
    indiceLibre++; 
}

int main(int argc, char** argv){

    //array local donde se almacenarán los caracteres retirados del buffer
    char* arrayLocal = malloc(100 * sizeof(char));
    char elemento; 

    //se abre el archivo compartido
    int fd = open("archivoCompartido.txt", O_RDWR);
    if(fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    //se trunca el archivo a N bytes
    if(ftruncate(fd, N*sizeof(char)) == -1) {
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //se mapea el archivo a memoria compartida para que ambos procesos puedan acceder a él
    void* map = mmap(NULL, N*sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(;;){

        if(nElementosBuffer == 0) sleep_();             //si count==0 duerme el proceso
        remove_item(map, &elemento);                    //se retira del buffer el caracter que corresponda

        nElementosBuffer = nElementosBuffer - 1;        
        
        if(nElementosBuffer == N-1) wakeup_();          //si count==N-1 despierta al productor
        consume_item(arrayLocal, elemento);             //añade el caracter retirado del buffer a un string local
    }

    return 0;
}


//Pseudocodigo consumidor
/*
void cons(){
    int item;
    while(1){
        if(count==0) sleep();
        item=remove_item();
        count=count-1;
        if(count==N-1) wakeup(prod);
        consume_item(item);
    }
}
*/

/*
Programa el productor y el consumidor para comprobar que se pueden presentar carreras crı́ticas.
Puedes aumentar la probabilidad de que ocurran haciendo que la región crı́tica dure más
tiempo, por ejemplo, con llamadas a la función sleep() oportunamente situadas. Añade al código
salidas que muestren como evolucionan el productor y el consumidor, y usa comentarios para indicar
donde deben situarse los sleep() para que se produzcan carreras crı́ticas con alta probabilidad.
Los códigos del productor y el consumidor deben ser dos programas diferentes llamados prod.c y
cons.c que se ejecuten en terminales distintos. Ten en cuenta lo siguiente:
◦ Dado que el consumidor y el productor serán dos procesos diferentes (no hilos), es necesario
definir zonas de memoria donde almacenar variables compartidas usando, por ejemplo, mmap().
◦ El buffer debe ser de tipo char, y funcionar como una cola LIFO (Last In First Out).
◦ El tamaño del buffer definido como una consttante de los códigos, debe ser N=8.
◦ Las funciones sleep() y wakeup() las puedes implementar con señales o con semáforos o susti-
tuirlas por espera activa.
◦ La función produce item() debe generar una letra mayúscula aleatoria. Deberá, además, ir
añadiendola a un string local que contendrá finalmente todos esos caracteres.
◦ La función insert item() debe colocar el carácter generado en el buffer.
◦ La función remove item() debe retirar del buffer el carácter que corresponda.
◦ La función consume item() debe añadir el carácter retirado del buffer a un string local donde
los irá almacenando a medida que se leen.
*/