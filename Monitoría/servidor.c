#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

  do {
    resultado = read(pipe_fd_general, &recibidos, sizeof(struct PeticionCliente));

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
              arregloClientes[numCliente] = nuevoCliente;
              write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
              numCliente++;
          }
          else{
            printf(" -> El usuario es repetido :(\n");
            strcpy(respuesta.contenido.respuesta.mensaje, "A");
            mkfifo(recibidos.contenido.registro.nombre_pipe, S_IRUSR | S_IWUSR);
            fd_destino = open(recibidos.contenido.registro.nombre_pipe, O_WRONLY | O_NONBLOCK);
            write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
          }
        break;

        case MENSAJE_INDIVIDUAL:
          printf(" -> Esto es un mensaje individual\n");
          printf(" -> El id de origen: %d\n", recibidos.contenido.mensajeIndividual.origen);
          printf(" -> El id de destino: %d\n", recibidos.contenido.mensajeIndividual.destino);
          respuesta.tipo = MENSAJE;
          respuesta.contenido.mensaje.origen = recibidos.contenido.mensajeIndividual.origen;
          respuesta.contenido.mensaje.destino = recibidos.contenido.mensajeIndividual.destino;
          strcpy(respuesta.contenido.mensaje.mensaje,recibidos.contenido.mensajeIndividual.mensaje);

          indiceEncontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.mensaje.origen) {
                  indiceEncontrado = i;
                  break;
              }
          }
          write(arregloClientes[indiceEncontrado].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
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

  // Leer el mensaje

  // int client_destino;
  

  

  
  // switch (recibidos.tipo) {
  // case MENSAJE_INDIVIDUAL:
  //   // Qué hacer si la solicitud es enviar un mensaje individual??
  //   printf("EL nombre del pipe de destino es es: %s",recibidos.contenido.mensajeIndividual.mensaje);
    
  //   // Crear la respuesta
  //   struct RespuestaServidor respuesta;
  //   respuesta.tipo = MENSAJE;
  //   respuesta.contenido.mensaje.origen = recibidos.contenido.mensajeIndividual.origen;
  //   respuesta.contenido.mensaje.destino = recibidos.contenido.mensajeIndividual.destino;
  //   strcpy(respuesta.contenido.mensaje.mensaje, recibidos.contenido.mensajeIndividual.mensaje);

  //   // Enviarle el mensaje
  //   //write(fd_destino, &respuesta, sizeof(struct Respuesta));

  //   break;

  // case MENSAJE_GRUPAL:
  //   recibidos.contenido.mensajeGrupal;
  //   break;

  // case CREACION_GRUPO:

  //   break;

  // case UNIR_GRUPO:
  //   break;

  // case DESCONEXION_GRUPO:
  //   break;
  // }
}

// // Buscar quién es el cliente destino?
//     client_destino = recibidos.contenido.mensajeIndividual.destino;

//     for (int i = 0; i < tam_maximo; i++) {
//       if (arregloClientes[i].id == client_destino) {

//         Guardar el pipe del cliente
//         fd_destino = arregloClientes[i].fd_pipe;
//       }
//     }
