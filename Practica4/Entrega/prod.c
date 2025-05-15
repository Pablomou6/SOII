// Pablo Mouriño Lorenzo
// Lucía Pérez González

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mqueue.h>     
#include <string.h>

#define MAX_BUFFER 5
#define MAX_TAM 50

mqd_t almacen1;         // Cola de entrada de mensajes para el productor
mqd_t almacen2;         // Cola de entrada de mensajes àra el consumidor
struct mq_attr attr;    // Atributos de los buzones

int T;                              //Variable de tiempo pasada por línea de comandos
char fichero_lectura[MAX_TAM];      //Nombre del fichero de lectura


/* Función de lectura de un caracter del fichero indicado */
char generarItem(FILE *fp) {
    char item= getc(fp);
    if(item == EOF) return '*';
    if (item == '*') return '\0';
    return item;
}

/* Función que comprende el proceso de producción de items y paso de mensajes */
void productor(){
    FILE *fp;
    char aviso, item= '0';
    unsigned int prioridad = 0; 
    // Se abre en modo lectura el fichero pasado por línea de comandos
    if((fp=fopen(fichero_lectura,"r"))== NULL){
        perror("Error: no se pudo abrir el archivo indicado en modo lectura");
        exit(EXIT_FAILURE);
    }

    // Se espera a que el consumidor confirme que está listo, llenando 'almacen2' de items vacíos
    // En cuanto el productor reciba el primer item vacío, comenzará con la producción
    while(aviso !=' '){
        mq_receive(almacen2, &aviso, sizeof(char), &prioridad);
    }

    // Se inicia el bucle de producción, el cual se detendrá al llegar al final del fichero de lectura
    // y produzca un asterisco '*'
    while(item!= '*'){
        // Se duerme el proceso de 0 a T microsegundos
        usleep(rand()%(T));

        // Se lee un caracter del fichero y se genera con él un item
        item = generarItem(fp);

        // Se comprueba que el item devuelto sea un caracter alfanumérico o un asterico de finalización
        if((item>=48 && item<=57) || (item>=65 && item<=90) || (item>=97 && item<=122) || item=='*'){
            printf("Item producido: %c\n", item);
            
            // Se tiene que implementar una cola FIFO para el buzón 'almacen1'. Para ello se emplea la prioridad:
            // Si todos los mensajes tienen la misma prioridad (será este caso), se consumirá de más antiguo 
            // a más reciente. Si tienen prioridad distinta, se consumirán primero los de mayor prioridad, 
            // independientemente de su antigüedad en el buzón.
            // Para ello se insertan en el buzón 'almacen1' con prioridad 0, la prioridad más baja.
            prioridad= 0;
            mq_send(almacen1, &item, sizeof(char), prioridad);

            // Consume un item vacío del buzón 'almacén2'
            mq_receive(almacen2, &aviso, sizeof(char), &prioridad);
        }
    }
    // Se cierra el fichero de lectura
    fclose(fp);
}

int main(int argc, char** argv){
    // Paso 1: Se comprueba que se introduce el número de argumentos adecuado y de formato adecuado por línea de comandos
    if(argc != 3){
        perror("Error: no ha insertado el número requerido de argumentos. Formato aceptado: ./exe <fichero_lectura> <espera_microsegundos> ");
        exit(EXIT_FAILURE);
    }else{
        if((T=atoi(argv[2]))==0){
            if(strcmp(argv[2], "0")==0){
                perror("Error: el tiempo de espera debe de ser mayor que 0. Formato aceptado: ./exe <fichero_lectura> <espera_microsegundos> ");
            }else{
                perror("Error: el segundo parámetro no es un número entero. Formato aceptado: ./exe <fichero_lectura> <espera_microsegundos> ");
            }
            exit(EXIT_FAILURE);
        }
        if(T < 0){
            perror("Error: el tiempo de espera debe de ser positivo. Formato aceptado: ./exe <fichero_lectura> <espera_microsegundos> ");
            exit(EXIT_FAILURE);
        }
    }
    strcpy(fichero_lectura, argv[1]);

    // Paso 2: Se inicializan los atributos de los buzones, se eliminan los buzones antes de ser creados y abiertos, para evitar errores por
    // si se mantuvieron abiertos tras una ejecucion previa
    attr.mq_maxmsg= MAX_BUFFER;
    attr.mq_msgsize= sizeof(char);
    
    // Borrado de los buffers de entrada por si existían de una ejecución previa
    mq_unlink("/ALMACEN1");
    mq_unlink("/ALMACEN2");

    // Creación y apertura de los buffers
    almacen1= mq_open("/ALMACEN1", O_CREAT|O_WRONLY, 0777, &attr);
    almacen2= mq_open("/ALMACEN2", O_CREAT|O_RDONLY, 0777, &attr);
    if((almacen1 == -1) || (almacen2 == -1)){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // Paso 3: Se inicia el proceso de producción y paso de mensajes
    printf("Buzones preparados. Esperando al consumidor...\n");
    productor();

    // Paso 4: Se cierran y eliminan los buzones
    mq_close(almacen1);
    mq_close(almacen2);

    mq_unlink("/ALMACEN1");
    mq_unlink("/ALMACEN2");

    exit(EXIT_SUCCESS);
}
