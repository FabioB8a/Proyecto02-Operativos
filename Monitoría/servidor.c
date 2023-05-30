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

#define MAX_TAM 120

// TOCA HACER TODOS LOS PERROR

int main(int argc, char **argv) {

  /// Proceso Servidor:
  struct PeticionCliente recibidos;
  // Obtener el pipe general junto con el tamaño máximo
  char *nombre_Pipe = NULL;
  int tam_maximo = 0;

  if (argc != 5)
  {
      printf(" -> Número inválido de argumentos.\n");
      printf(" -> Recuerda: Uso correcto del ejecutable: ./Manager -n cantidadTalkers -p nombrePipe\n");
      exit(1);
  }
  
  for (int i = 1; i < argc; i += 2)
  {
    if (strcmp(argv[i], "-n") == 0 && !tam_maximo)
    {
        tam_maximo = atoi(argv[i + 1]);
    }
    else if (strcmp(argv[i], "-p") == 0 && !nombre_Pipe)
    {
        nombre_Pipe = argv[i + 1];
    }
    else
    {
      printf(" -> Argumento inválido o repetido: %s.\n", argv[i]);
      exit(1);
    }
  }

  if (tam_maximo <= 0)
  {
    printf(" -> La cantidad de Talkers debe ser mayor a 0\n");
    exit(1);
  }

  struct Cliente {
    int id;
    int fd_pipe;
    int pid;
    char nombre_asociado[MAX_TAM];
  };

  struct Cliente *arregloClientes = malloc(tam_maximo * sizeof(struct Cliente));
  int numCliente = 0;
  
  // Creación de pipe
  unlink(nombre_Pipe);
  mkfifo(nombre_Pipe, S_IRUSR | S_IWUSR);

  // Abrir el pipe
  int pipe_fd_general = open (nombre_Pipe, O_RDONLY | O_NONBLOCK);
  int resultado = 0; 
  int salir = 0;
  struct RespuestaServidor respuesta;
  int fd_destino;
  printf("   ____   _   _      _       _____    _____  U _____ u   ____                 _   _     ____   \n");
  printf("U /\"___| |'| |'| U  /\"\\  u  |_ \" _|  |_ \" _| \\| ___\"|/U |  _\"\\ u     ___     | \\ |\"| U /\"___|u\n");
  printf("\\| | u  /| |_| |\\ \\/ _ \\/     | |      | |    |  _|\"   \\| |_) |/    |_\"_|   <|  \\| |>\\| |  _ / \n");
  printf(" | |/__ U|  _  |u / ___ \\    /| |\\    /| |\\   | |___    |  _ <       | |    U| |\\  |u | |_| |  \n");
  printf("  \\____| |_| |_| /_/   \\_\\  u |_|U   u |_|U   |_____|   |_| \\_\\    U/| |\\u   |_| \\_|   \\____|  \n");
  printf(" _// \\\\  //   \\\\  \\\\    >>  _// \\\\_  _// \\\\_  <<   >>   //   \\\\_.-,_|___|_,-.||   \\\\,-._)(|_   \n");
  printf("(__)(__)(_\") (\"_)(__)  (__)(__) (__)(__) (__)(__) (__) (__)  (__)\\_)-' '-(_/ (_\")  (_/(__)__)  \n");
  printf("_______________________________________________________________________________________________\n");
  printf("_________________________________________M A N A G E R_________________________________________\n");
  do {
    resultado = read(pipe_fd_general, &recibidos, sizeof(struct PeticionCliente));
    struct RespuestaServidor respuesta;
    if (resultado > 0) {
      switch (recibidos.tipo) {
        case REGISTRO:
          
          respuesta.tipo = RESPUESTA;
          respuesta.contenido.respuesta.codigo = recibidos.contenido.registro.idRegistro;
          // Verificación de índice encontrado
          int indiceEncontrado = -1;

          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.respuesta.codigo) {
                  indiceEncontrado = i;
                  break;
              }
          }

          if (indiceEncontrado == -1) {
              printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
              printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
              strcpy(respuesta.contenido.respuesta.mensaje, "Agregado correctamente");
              struct Cliente nuevoCliente;
              nuevoCliente.id = recibidos.contenido.registro.idRegistro;
              strcpy(nuevoCliente.nombre_asociado, recibidos.contenido.registro.nombre_pipe);
              mkfifo(recibidos.contenido.registro.nombre_pipe, S_IRUSR | S_IWUSR);
              fd_destino = open(recibidos.contenido.registro.nombre_pipe, O_WRONLY | O_NONBLOCK);
              nuevoCliente.fd_pipe = fd_destino;
              nuevoCliente.pid = recibidos.contenido.registro.pid;
              arregloClientes[numCliente] = nuevoCliente;
              numCliente++;
              write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
              kill(recibidos.contenido.registro.pid, SIGUSR1);
          }
          else{
            printf(" -> El usuario es repetido :(\n");
            strcpy(respuesta.contenido.respuesta.mensaje, "A");
            mkfifo(recibidos.contenido.registro.nombre_pipe, S_IRUSR | S_IWUSR);
            fd_destino = open(recibidos.contenido.registro.nombre_pipe, O_WRONLY | O_NONBLOCK);
            write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
            kill(recibidos.contenido.registro.pid, SIGUSR1);
          }
        break;

        case MENSAJE_INDIVIDUAL:

          printf(" -> Mensaje individual\n");
          printf(" -> Esto es un mensaje individual\n");
          printf(" -> El id de origen: %d\n", recibidos.contenido.mensajeIndividual.origen);
          printf(" -> El id de destino: %d\n", recibidos.contenido.mensajeIndividual.destino);
          respuesta.tipo = MENSAJE;
          respuesta.contenido.mensaje.origen = recibidos.contenido.mensajeIndividual.origen;
          respuesta.contenido.mensaje.destino = recibidos.contenido.mensajeIndividual.destino;
          strcpy(respuesta.contenido.mensaje.mensaje,recibidos.contenido.mensajeIndividual.mensaje);
          strcpy(respuesta.contenido.mensaje.nombre,recibidos.contenido.mensajeIndividual.nombre);
          indiceEncontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.mensaje.destino) {
                  indiceEncontrado = i;
                  break;
              }
          }
          write(arregloClientes[indiceEncontrado].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
          kill(arregloClientes[indiceEncontrado].pid, SIGUSR1);
        break;

        }
    } else if (resultado <= 0) {
        // No hay datos disponibles, el otro extremo de la tubería se ha cerrado
        // salir = 1; // Finalizar el ciclo
    }
    else{

    }
    fflush(stdout);
} while (!salir);

}