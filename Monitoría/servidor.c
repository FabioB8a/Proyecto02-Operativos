#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "peticion.h"
#include "respuesta.h"

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
  };

  struct Cliente *arregloClientes = malloc(tam_maximo * sizeof(struct Cliente));
  
  // Creación de pipe
  unlink(nombre_Pipe);
  mkfifo(nombre_Pipe, S_IRUSR | S_IWUSR);

  // Abrir el pipe
  int pipe_fd_general = open (nombre_Pipe, O_RDONLY);
  int resultado = 0; 
  int salir = 0;
  struct RespuestaServidor respuesta;
  int fd_destino;
do {
    resultado = read(pipe_fd_general, &recibidos, sizeof(struct PeticionCliente));

    if (resultado > 0) {
        switch (recibidos.tipo) {
            case REGISTRO:
                printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
                printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);

                // Verification that it exists
                respuesta.tipo = RESPUESTA;
                respuesta.contenido.respuesta.codigo = recibidos.contenido.registro.idRegistro;
                strcpy(respuesta.contenido.respuesta.mensaje, "Enviado correctamente");

                unlink(recibidos.contenido.registro.nombre_pipe);
                mkfifo(recibidos.contenido.registro.nombre_pipe, S_IRUSR | S_IWUSR);
                fd_destino = open(recibidos.contenido.registro.nombre_pipe, O_WRONLY | O_NONBLOCK);
                write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
                close(fd_destino);
                break;
        }
    } else if (resultado == 0) {
        // No data available, the other end of the pipe has been closed
        salir = 1; // Exit the loop
    } else {
        // Handle read error
        perror("Error while reading from pipe");
        salir = 1; // Exit the loop
    }
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
