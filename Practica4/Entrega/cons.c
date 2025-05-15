// Pablo Mouriño Lorenzo
// Lucía Pérez González

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <string.h>

#define N 5 
#define MAX_TAM 50

mqd_t almacen1;         // Cola de entrada de mensajes para el productor
mqd_t almacen2;         // Cola de entrada de mensajes àra el consumidor
struct mq_attr attr;    // Atributos de los buzones

int T;                              //Variable de tiempo pasada por línea de comandos
char fichero_escritura[MAX_TAM];      //Nombre del fichero de lectura
unsigned int prioridad;               //Variable de prioridad de los mensajes recibidos/enviados

/* Función que recibe el mensaje del productor consumiendo un item del buzón 'almacen1' */
void recibirMensaje(char* item) {
    // Cola FIFO, se recibe del buzón 'almacen1' el item del productor. Si todos los mensajes 
    // tienen la misma prioridad (será este caso), se consumirá de más antiguo a más reciente. 
    // Si tienen prioridad distinta, se consumirán primero los de mayor prioridad, 
    // independientemente de su antigüedad en el buzón.
    // Por ello, ya que todos los mensajes tendrán la misma prioridad (responsabilidad del productor)
    // se recibirán los mensajes de más antiguos a más recientes
    if(mq_receive(almacen1, item, sizeof(char), &prioridad) == -1) {
        printf("Error al recibir el mensaje\n");
        exit(EXIT_FAILURE);
    }
}

/* Función que comprende el proceso de consumo de items y de paso de mensajes  */
void consumidor() {
    char item, espacio = ' ';

    // Se abre en modo escritura el fichero pasado por línea de comandos
    FILE *doc = fopen(fichero_escritura, "w+");
    if(doc == NULL) {
        printf("Error al abrir el archivo %s\n", fichero_escritura);
        exit(EXIT_FAILURE);
    }
    
    // Se procede a llenar el buzón 'almacen2' de items vacións, avisando el productor de paso de que
    // está listo para iniciar el consumo
    for(int i=0; i<5; i++){
        mq_send(almacen2, &espacio, sizeof(char), prioridad);
    }

    // Se inicia el bucle de consumo, que se detendrá cuando reciba un asterisco '*' como item
    while(1) {
        // Se duerme el proceso de 0 a T microsegundos
        usleep(rand()%(T));

        // Se llama a la funcion de recepción de mensajes, se imprime por pantalla 
        recibirMensaje(&item);
        printf("Item recibido: %c\n", item);
        
        // Se envía un item vacío al buzón 'almacen2' confirmando la recepción
        mq_send(almacen2, &espacio, sizeof(char), prioridad);

        // Se comprueba si el item recibido es un asterisco '*' o no
        if(item == '*') {
            // En caso se que sea asterisco, se consumen los items restantes y se finaliza el bucle
            // Se obtienen los atributos del buzón 'almacen1' para consultar la cantidad de items que 
            // aún contiene
            mq_getattr(almacen1, &attr);
            for(int i=0; i < attr.mq_curmsgs; i++){
                recibirMensaje(&item);
                printf("Item recibido: %c\n", item);
            }
            printf("\nFin de la ejecución\n");
            break;
        }else{
            // En caso de que no sea asterisco, se escribe el item en el archivo de salida
            fprintf(doc, "%c\n", item);
        }
    }

    // Se cierra el archivo de salida
    if(fclose(doc) == EOF) {
        printf("Error al cerrar el archivo %s\n", fichero_escritura);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
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
    strcpy(fichero_escritura, argv[1]);

    // Paso 2: Se abren los buzones que habrá creado previamente el productor
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

    // Paso 3: Se inicia el proceso de consumo y paso de mensajes
    consumidor();

    //PASO 4: Se cierran los buzones y se eliminan para asegurase que no quedan activos en el kernel
	mq_close(almacen1);
	mq_close(almacen2);
	
	mq_unlink("/ALMACEN1");
	mq_unlink("/ALMACEN2");

    exit(EXIT_SUCCESS);
}