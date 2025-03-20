/*
    Proyectamos el archivo en memoria (lectura y escritura, compartida)
    void *mapped = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mimir.h"

#define N 8     //tamaño del buffer

int nElementosBuffer = 0;   

void produce_item(char* elemento, char* arrayLocal){ 
    //Generamos una letra aleatoriamente
    *elemento = 'A' + rand() % 26;
    //Introducimos la letra en el array local de char's
    arrayLocal[strlen(arrayLocal)] = *elemento;
}

void insert_item(char elemento, void* arrayCompartido){
    //convertimos el arrayCopartido al tipo adecuado
    char* buffer = (char*)arrayCompartido;
    //añadir caracter generado al buffer compartido
    buffer[nElementosBuffer] = elemento;
}



int main(int argc, char *arg[]) {

    //arraylocal de char para almacenar los elementos generados
    char* arrayLocal = (char *)malloc(100 * sizeof(char));
    //caracter que se va a generar
    char elemento;

    //abrimos el archivo para lectura y escritura usando open
    int fd = open("archivoCompartido.txt", O_RDWR); 
    if(fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    //Como vamos a mapear el archivo, necesitamos saber su tamaño, por lo que lo truncamos
    if(ftruncate(fd, N*sizeof(char)) == -1) {
        //ftruncate recibe el descriptor de archivo y el tamaño al que queremos truncar
        perror("Error al truncar el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //proyectamos el archivo en memoria y almacenamos la dirección de memoria devuelta
    //map es puntero a void con el que accederemos a la memoria proyectada
    void* map = mmap(NULL, N*sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(;;) {   //bucle infinito

        produce_item(&elemento, arrayLocal);        //generamos un elemento
        printf("(prod) elemento generado: %c\n", elemento);
        
        if(nElementosBuffer == N){
            printf("(prod) Buffer lleno, voy mimir\n");
            sleep_();         //si el buffer está lleno, dormimos el proceso
        }
           
        insert_item(elemento, map); 
        //incrementamos el número de elementos en el buffer
        nElementosBuffer = nElementosBuffer + 1;                 //insertamos el elemento en el buffer
        printf("(prod) elemento insertado en el buffer\n");
        
        if(nElementosBuffer == 1){
            printf("\n(prod) Buffer vacío, despertamos al consumidor\n");
            wakeup_();        //si el buffer estaba vacío, despertamos al consumidor 
        }

    }

    

    return 0;
}
/*
void prod(){
    int item;
    while(1){
        item=produce_item();
        if(count==N) sleep();
        insert_item(item);
        count=count+1;
        if(count==1) wakeup(cons);
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