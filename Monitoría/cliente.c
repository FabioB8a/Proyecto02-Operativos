#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "peticion.h"
#include "respuesta.h"

#define _GNU_SOURCE
#define MAX_TAM 120

volatile sig_atomic_t comando = 0;

void handle_signal(int signum) {
    if (signum == SIGUSR1) {
        comando = 1;
    }
}

int main(int argc, char **argv) {

  char *nomPipeManager = NULL;
  char nomPipeTalker[MAX_TAM];
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
      else if (strcmp(argv[i], "-p") == 0 && !nomPipeManager)
      {
          nomPipeManager = argv[i + 1];
      }
      else
      {
          printf(" -> Argumento inválido o repetido: %s.\n", argv[i]);
          exit(1);
      }
  }

  if (idTalker <= 0)
  {
      printf(" -> La cantidad de Talkers debe ser mayor a 0\n");
      exit(1);
  }
  /// Proceso Cliente:
  // Quiere enviar un mensaje a otro usuario
  // 1. Armar el mensaje

  struct PeticionCliente datos;

  datos.tipo = REGISTRO;
  datos.contenido.registro.idRegistro = idTalker;
  printf(" -> Por favor, ingresa el nombre de tu pipe: \n");

  fgets(nomPipeTalker, MAX_TAM, stdin);
  nomPipeTalker[strcspn(nomPipeTalker, "\n")] = '\0';

  strcpy(datos.contenido.registro.nombre_pipe, nomPipeTalker);


  mkfifo (nomPipeManager, S_IRUSR | S_IWUSR);
  int pipe_servidor_general = open (nomPipeManager, O_WRONLY | O_NONBLOCK);// Aquí abrir el pipe (o tenerlo abierto previamente)
  write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));

  unlink(nomPipeTalker);
  mkfifo (nomPipeTalker, S_IRUSR | S_IWUSR);
  // 2. Recibir la respuesta (O_NONBLOCK para hacerlo asíncrono (No bloqueante))
  int pipe_especifico = open(nomPipeTalker, O_RDONLY | O_NONBLOCK);

  struct RespuestaServidor respuesta;
  int resultado = 0; // Inicializar el resultado
  int salir = 0;


int pipe_flags = fcntl(pipe_especifico, F_GETFL); // Get the current pipe flags
pipe_flags |= O_NONBLOCK; // Add the non-blocking flag
fcntl(pipe_especifico, F_SETFL, pipe_flags); // Set the modified flags



do {
    resultado = read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));
    if (resultado > 0) {
        // Handle the received data from the pipe
        switch (respuesta.tipo) {
            case RESPUESTA:
                printf(" -> Id asociado: %d\n", respuesta.contenido.respuesta.codigo);
                printf(" -> Respuesta del servidor: %s\n", respuesta.contenido.respuesta.mensaje);
                if (strcmp(respuesta.contenido.respuesta.mensaje, "A") == 0) {
                    printf("NO existo");
                }
            break;

            case MENSAJE:
                printf(" -> El mensaje asociado es: %s\n",respuesta.contenido.mensaje.mensaje);
                printf(" -> El mensaje te lo envió: %d\n",respuesta.contenido.mensaje.origen);
            break;
        }
    }
    else {
        printf("Menu:\n");
        printf("1. Option 1: Mensaje individual\n");
        printf("2. Option 2\n");
        int option;
        printf("Select an option: ");

        // Read from stdin without blocking
        if (scanf("%d", &option) == 1) {
            getchar(); // Consume the newline character

            switch (option) {
                case 1:
                    printf("MENSAJE INDIVIDUAL: ");
                    datos.tipo = MENSAJE_INDIVIDUAL;
                    datos.contenido.mensajeIndividual.origen = idTalker;

                    int idDestino;
                    printf("Ingrese el id destino: ");
                    scanf("%d", &idDestino);
                    datos.contenido.mensajeIndividual.destino = idDestino;
                    
                    char mensajesA[MAX_TAM];
                    printf("Ingrese el mensaje: ");
                    fgets(mensajesA, MAX_TAM, stdin);
                    mensajesA[strcspn(mensajesA, "\n")] = '\0';
                    strcpy(datos.contenido.mensajeIndividual.mensaje, mensajesA);

                    write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));
                    break;
                case 2:
                    // Option 2 logic
                    break;
                // ... handle other options ...
                default:
                    printf("Opción Inválida.\n");
            }
        }
    }

    fflush(stdout);
} while (!salir);

  

}

  // datos.contenido.registro.nombre_pipe = idTalker;
  // datos.contenido.mensajeIndividual.origen = 0l;  // Mi ID
  // datos.contenido.mensajeIndividual.destino = 1l; // ID del detino
  // strcpy(datos.contenido.mensajeIndividual.mensaje, "¿Qué esta ocurriendo aquí?"); // Contenido

  // Mientras que no haya un resultado
  // while (1) {
  //   // Intentar leer del pipe
  //   resultado = read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));

  //   // Se ha recibido algo por el pipe
  //   if (resultado> 0) {
  //     switch (respuesta.tipo) {
  //     case MENSAJE:
  //       printf("Ha recibido un mensaje de: %d",
  //              respuesta.contenido.mensaje.origen);
  //       printf("Mensaje: %s", respuesta.contenido.mensaje.mensaje);

  //     case RESPUESTA:
  //       printf("Respuesta del servidor: %d", respuesta.contenido.respuesta.codigo);
  //       printf("Respuesta del servidor: %s", respuesta.contenido.respuesta.mensaje);
  //     }
  //   } else {
  //     /// Código asíncrono:
  //     // No se ha leído nada por el pipe
  //   }
  // }