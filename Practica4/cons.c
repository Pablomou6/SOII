#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>

//Definimos el tamño del buffer
#define N 5 

//Definimos los buffers con los que vamos a trabajar
mqd_t almacen1; //Buffer para entrada del consumidor
mqd_t almacen2; //Buffer para entrada del productor
FILE* doc; //Archivo de salida
int T; //Tiempo de espera

//Con send podes setter a priorida ddos mensajes, esa forma, si simepre es null
//y establecemos en receive que reciba el más antiguo, el primero que se envió, lo recibe

void consumidor() {
    char elemento;

    //Abrimos el archivo de salida
    if(fopen(doc, "w") == NULL) {
        printf("Error al abrir el archivo %s\n", doc);
        exit(EXIT_FAILURE);
    }
    
    while(1) {
        //Mandamos el mensaje al productor para que sepa que puede enviar
        mq_send(almacen2, ' ', sizeof(char), NULL);

        //Recibimos el mensaje del productor, que por defecto es el más antiguo y prioritario
        mq_receive(almacen1, &elemento, sizeof(char), NULL);
    }
    

}

int main(int argc, char* argv[]) {
    //Se comprueba que se pasen los argumentos correctos
    if(argc != 3) {
        printf("Usage: %s <file_name> <delay>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Almacenamos los argumentos en variables
    FILE* doc = argv[1];
    int T = atoi(argv[2]);

    //Abrimos los buffers, ya que se encarga el productor de crearlos
    almacen1 = mq_open("/ALMACEN1", O_RDONLY);
    if(almacen1 == -1) {
        printf("Error al abrir el buzón de entrada del produtor\n");
        exit(EXIT_FAILURE);
    }

    almacen2 = mq_open("/ALMACEN2", O_WRONLY);
    if(almacen2 == -1) {
        printf("Error al abrir el buzón de salida del consumidor\n");
        exit(EXIT_FAILURE);
    }

    //Llamamos a la función que hará el trabajo del consumidor
    consumidor();


}