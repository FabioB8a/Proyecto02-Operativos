#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "Nom.h"
#include "signal.h"

int main(int argc, char **argv){

    char *nomPipe = NULL;
    int cantidadTalkers = 0;

    if (argc != 5)
    {
        printf(" -> Número inválido de argumentos.\n");
        printf(" -> Recuerda: Uso correcto del ejecutable: ./Manager -n cantidadTalkers -p nombrePipe\n");
        exit(1);
    }

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-n") == 0 && !cantidadTalkers)
        {
            cantidadTalkers = atoi(argv[i + 1]);
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

    if (cantidadTalkers <= 0)
    {
        printf(" -> La cantidad de Talkers debe ser mayor a 0\n");
    }
       
    exit(0);
}