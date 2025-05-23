#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

//Definimos el tamño del buffer
#define N 5 

//Definimos los buffers con los que vamos a trabajar
mqd_t almacen1; //Buffer para entrada del consumidor
mqd_t almacen2; //Buffer para entrada del productor
FILE* doc; //Archivo de salida
int T; //Tiempo de espera
char* file_name;

//Funcion que recibe el mensaje del productor
void recibirMensaje(char* elemento) {
    //recibimos el mensaje del productor, que por defecto es el más antiguo y prioritario
    if(mq_receive(almacen1, elemento, sizeof(char), 0) == -1) {
        printf("Error al recibir el mensaje: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

//Con send podes setter a priorida ddos mensajes, esa forma, si simepre es null
//y establecemos en receive que reciba el más antiguo, el primero que se envió, lo recibe

void consumidor() {
    char elemento;

    //Abrimos el archivo de salida
    doc = fopen(file_name, "w");
    if(doc == NULL) {
        printf("Error al abrir el archivo %s\n", file_name);
        exit(EXIT_FAILURE);
    }
    
    //Mandamos el mensaje al productor para que sepa que puede enviar
    char espacio = ' ';
    printf("Voy a enviar el mensaje de inicio al productor\n");
    for(int i = 0; i < N; i++) {
        mq_send(almacen2, &espacio, sizeof(char), 0);
        printf("Mensaje de inicio enviado al productor\n");
    }

    //Esperamos a que el productor se llene
    sleep(2);

    while(1) {
        //Esperamos los milisegundos T
        usleep(rand()%(T));

        //Llamamos a la funcion que recibe el mensaje
        recibirMensaje(&elemento);
        printf("Recibido: %c\n", elemento);

        //Escribimos el mensaje en el archivo de salida
        fprintf(doc, "%c\n", elemento);
        printf("Escrito en el archivo de salida\n");

        //Comprobamos que el mensaje recibidos es * o no
        if(elemento == '*') {
            printf("Fin de la ejecución\n");
            break;
        }

        mq_send(almacen2, &espacio, sizeof(char), 0);
        printf("Mensaje de recepción enviado al productor\n");
    }

    //Cerramos el archivo de salida
    if(fclose(doc) == EOF) {
        printf("Error al cerrar el archivo %s\n", file_name);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    //Se comprueba que se pasen los argumentos correctos
    if(argc != 3) {
        printf("Usage: %s <file_name> <delay>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Almacenamos los argumentos en variables
    file_name = argv[1];
    T = atoi(argv[2]);

    //Abrimos los buffers, ya que se encarga el productor de crearlos
    almacen1 = mq_open("/ALMACEN1", O_CREAT|O_RDONLY);
    if(almacen1 == -1) {
        printf("Error al abrir el buzón de entrada del productor: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    almacen2 = mq_open("/ALMACEN2", O_CREAT|O_WRONLY);
    if(almacen2 == -1) {
        printf("Error al abrir el buzón de salida del consumidor\n");
        exit(EXIT_FAILURE);
    }

    //Llamamos a la función que hará el trabajo del consumidor
    consumidor();

}