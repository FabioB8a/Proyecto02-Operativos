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

// Variables globales para indicar eventos de entrada
volatile sig_atomic_t recibido_consola_signal = 0;
int pipe_especifico = 0;
int resultado = 0;

// Funcion de manejo de señales
void console_signal_handler(int signum) {
    struct RespuestaServidor respuesta;
    resultado = read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));
    if (resultado > 0) {
        // Obtener el tipo de respuesta asociado
        switch (respuesta.tipo) {
            case RESPUESTA:
                printf("-> Usuario registrado correctamente");
                printf("-> Id asociado: %d\n", respuesta.contenido.respuesta.codigo);
                printf("-> Respuesta del servidor: %s\n", respuesta.contenido.respuesta.mensaje);
                if (strcmp(respuesta.contenido.respuesta.mensaje, "A") == 0) {
                    printf("Usuario no registrado correctamente");
                }
            break;

            case MENSAJE:
                printf("-> MSG: %s (Usuario %d): %s\n",respuesta.contenido.mensaje.nombre,respuesta.contenido.mensaje.origen,respuesta.contenido.mensaje.mensaje);
            break;
        }
        fflush(stdout);
    }

}

int main(int argc, char **argv) {

  signal(SIGUSR1, console_signal_handler);

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

  datos.contenido.registro.pid = getpid();

  mkfifo (nomPipeManager, S_IRUSR | S_IWUSR);
  int pipe_servidor_general = open (nomPipeManager, O_WRONLY | O_NONBLOCK);// Aquí abrir el pipe (o tenerlo abierto previamente)
  write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));

  unlink(nomPipeTalker);
  mkfifo (nomPipeTalker, S_IRUSR | S_IWUSR);
  // 2. Recibir la respuesta (O_NONBLOCK para hacerlo asíncrono (No bloqueante))
  pipe_especifico = open(nomPipeTalker, O_RDONLY | O_NONBLOCK);

  struct RespuestaServidor respuesta;
  int salir = 0;



    do {

        char mensaje_envio[MAX_TAM];
        fgets(mensaje_envio, MAX_TAM, stdin);
        mensaje_envio[strcspn(mensaje_envio, "\n")] = '\0';
        char command[10];
        char string[10];
        int number;
        sscanf(mensaje_envio, "%s \"%[^\"]\" %d", command, string, &number);

        if (strcmp(command, "sent") == 0)
        {
            struct PeticionCliente datos;
            datos.tipo = MENSAJE_INDIVIDUAL;
            datos.contenido.mensajeIndividual.origen = idTalker;
            datos.contenido.mensajeIndividual.destino = number;
            strcpy(datos.contenido.mensajeIndividual.mensaje, string);
            strcpy(datos.contenido.mensajeIndividual.nombre, nomPipeTalker);

            write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));
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