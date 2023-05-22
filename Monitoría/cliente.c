#include <fcntl.h>
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

  datos.tipo = MENSAJE_INDIVIDUAL;
  datos.contenido.mensajeIndividual.origen = 0l;  // Mi ID
  datos.contenido.mensajeIndividual.destino = 1l; // ID del detino
  strcpy(datos.contenido.mensajeIndividual.mensaje, "¿Qué esta ocurriendo aquí?"); // Contenido

  mkfifo (nomPipe, S_IRUSR | S_IWUSR);

  int pipe_servidor_general = open (nomPipe, O_WRONLY | O_NONBLOCK);// Aquí abrir el pipe (o tenerlo abierto previamente)

  write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));

  // 2. Recibir la respuesta (O_NONBLOCK para hacerlo asíncrono (No bloqueante))
  int pipe_especifico = open("nombre del pipe", O_RDONLY | O_NONBLOCK);

  struct RespuestaServidor respuesta;
  int resultado = -1; // Inicializar el resultado

  // Mientras que no haya un resultado
  while (1) {
    // Intentar leer del pipe
    resultado =
        read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));

    // Se ha recibido algo por el pipe
    if (resultado == 0) {
      switch (respuesta.tipo) {
      case MENSAJE:
        printf("Ha recibido un mensaje de: %d",
               respuesta.contenido.mensaje.origen);
        printf("Mensaje: %s", respuesta.contenido.mensaje.mensaje);

      case RESPUESTA:
        printf("Respuesta del servidor: %d",
               respuesta.contenido.respuesta.codigo);
      }
    } else {
      /// Código asíncrono:
      // No se ha leído nada por el pipe
    }
  }
}