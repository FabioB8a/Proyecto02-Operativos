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

}