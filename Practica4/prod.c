#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mqueue.h>
#include <string.h>

#define MAX_BUFFER 5
#define MAX_TAM 30

mqd_t almacen1;         // Cola de entrada de mensajes para el productor
mqd_t almacen2;         // Cola de entrada de mensajes àra el consumidor
struct mq_attr attr;    // Atributos de los buzones
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
    char aviso;
    while(aviso !=' '){
        mq_receive(almacen2, &aviso, sizeof(char), NULL);
    }

    char item= '0';
    while(item!= '*'){
        //sleep(rand()%(T));

        //Lectura del fichero de entrada
        item= getc(fp);
        if(item == EOF) item= '*';
        if (item == '*') item= '\0';

        // Se comprueba que el item devuelto sea un caracter alfanumérico o un asterico de finalización
        if((item>=48 && item<=57) || (item>=65 && item<=90) || (item>=97 && item<=122) || item=='*'){
            mq_getattr(almacen1, &attr);
            printf("Num mensajes: %ld\n", attr.mq_curmsgs);

            printf("Item producido: %c\n", item);
            // Cola FIFO, se insertan con prioridad 0, la prioridad más baja.
            // Al tener todos los items, la misma prioridad, se rebirán de más antigua a más reciente.
            mq_send(almacen1, &item, sizeof(char), 0);
            mq_receive(almacen2, &aviso, sizeof(char), 0);
        }
        
        //if(aviso=='f'){
        //    mq_send(almacen1, 'y', siezeof(char), NULL);
        //}
    }
    fclose(fp);
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
    strcpy(fichero_lectura, argv[1]);
    printf("%s\n\n", fichero_lectura);

    // Paso 2:
    // Atributos de la cola
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
