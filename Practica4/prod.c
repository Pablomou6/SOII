#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mqueue.h>

#define MAX_BUFFER 5
#define MAX_TAM 30

mqd_t almacen1; // Cola de entrada de mensajes para el productor
mqt_t almacen2; // Cola de entrada de mensajes àra el consumidor
int T;
char fichero_lectura[MAX_TAM];

void productor(){
    // Se abre en modo lectura el fichero de items del hilo productor
    FILE *fp;
    if((fp=fopen(fichero_lectura,"r"))== NULL){
        perror("Error: no se pudo abrir el archivo indicado en modo lectura");
        exit(EXIT_FAILURE);
    }
    // Se espera a que el consumidor confirme que está listo
    char aviso='';
    while(aviso==''){
        mq_receive(almacen2, &aviso, sizeof(char), NULL);
    }
    aviso='';

    char item= getc(fp);
    while((item= getc(fp)) != EOF)){
        //sleep(rand()%(T));
        
        mq_send(almecen1, item, sizeof(char), NULL);

    }

}

int main(int argc, char** argv){
    // Paso 1: Se comprueba que se introduce el número de argumentos adecuado y de formato adecuado por línea de comandos
    int numCons;
    if(argc != 3){
        perror("Error: no ha insertado el número requerido de argumentos. Formato aceptado: ./exe <fichero_lectura> <espera_segundos> ");
        exit(EXIT_FAILURE);
    }else{
        if((T=atoi(argv[2]))==0){
            if(strcmp(argv[2], "0")==0){
                perror("Error: el tiempo de espera debe de ser mayor que 0. Formato aceptado: ./exe <fichero_lectura> <espera_segundos> ");
            }else{
                perror("Error: el segundo parámetro no es un número entero. Formato aceptado: ./exe <fichero_lectura> <espera_segundos> ");
            }
            exit(EXIT_FAILURE);
        }
        if(T < 0){
            perror("Error: el tiempo de espera debe de ser positivo. Formato aceptado: ./exe <fichero_lectura> <espera_segundos> ");
            exit(EXIT_FAILURE);
        }
    }

    // Paso 2:
    // Atributos de la cola
    struct mq_attr attr;
    attr.mq_maxmsg= MAX_BUFFER;
    attr.mq_msgsize= sizeof(char);

    // Borrado de los buffers de entrada por si existían de una ejecución previa
    mq_unlink("/ALMACEN1");
    mq_unlink("/ALMACEN2");

    // Apertura de los buffers
    almacen1= mq_open("/ALMACEN1", O_CREAT|O_WRONLY, 077, &attr);
    almacen2= mq_open("/ALMACEN2", O_CREAT|O_RDONLY, 077, &attr);
    if((almacen1 == -1) || (almacen2 == -1)){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    productor();

    mq_close(almacen1);
    mq_close(almacen2);

    exit(EXIT_SUCCESS);
}
