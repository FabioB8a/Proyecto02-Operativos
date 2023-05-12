#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "Nom.h"
#include "signal.h"

#define TAMMENSAJE 100

typedef void (*signalhandler_t)(int);
int fd1=0;
char mensaje[TAMMENSAJE];

signalhandler_t handler(void){
	printf("Desde el Manejador");
	read(fd1,mensaje,TAMMENSAJE);
	printf("El proceso cliente termina y lee %s \n", mensaje);
}

int main(int argc, char **argv){

    char *nomPipe = NULL;
    int idTalker = 0;

    if (argc != 5)
    {
        printf(" -> Número inválido de argumentos.\n");
        printf(" -> Recuerda: Uso correcto del ejecutable: ./Talker -i IDTalker -p nombrePipe\n");
        exit(1);
    }

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-i") == 0 && !idTalker)
        {
            idTalker = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-p") == 0 && !nomPipe)
        {
            nomPipe = argv[i + 1];
        }
        else
        {
            printf(" -> Argumento inválido o repetido: %s.\n", argv[i]);
        }
    }

    if (idTalker <= 0)
    {
        printf(" -> La cantidad de Talkers debe ser mayor a 0\n");
    }

    // Creacion del pipe inicial, el que se recibe como argumento del main
    int  fd, fd1,  pid, n, cuantos,res,creado=0;
    datap datos;
    
    
    mode_t fifo_mode = S_IRUSR | S_IWUSR;
    
    // Creacion del pipe inicial, el que se recibe como argumento del main
    unlink(nomPipe);
            
    if (mkfifo (nomPipe, fifo_mode) == -1) {
        perror("Server mkfifo:");
        exit(1);
    }
    
    // Abre el pipe. 
    if ((fd = open (nomPipe, O_RDONLY)) == -1) {
            perror(" Servidor abriendo el pipe: ");
            exit(1);

    }
    
    // El otro proceso (cliente) le envia el nombre para el nuevo pipe y el pid. 
    cuantos = read (fd, &datos, sizeof(datos));
    if (cuantos == -1) {
        perror("proceso lector:");
        exit(1);
    }
    printf ("Server lee el string %s\n", datos.segundopipe);
    printf ("Server el pid %d\n", datos.pid );

    do { 
        if ((fd1 = open(datos.segundopipe, O_WRONLY)) == -1) {
            perror(" Server Abriendo el segundo pipe ");
            printf(" Se volvera a intentar despues\n");
            sleep(5); //los unicos sleeps que deben colocar son los que van en los ciclos para abrir los pipes que han creado o deben crear otros procesos         
        } else creado = 1; 
    }  while (creado == 0);


        // Se escribe un mensaje para el  proceso (client)
    
    write(fd1, "hola", 5);
    sleep(1);
    kill(datos.pid,SIGUSR1);

        
    exit(0);
}