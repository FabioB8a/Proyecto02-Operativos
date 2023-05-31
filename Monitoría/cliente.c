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
char nomPipeTalker[MAX_TAM];

// Funcion de manejo de señales
void console_signal_handler(int signum) {
    struct RespuestaServidor respuesta;
    resultado = read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));
    if (resultado > 0) {
        // Obtener el tipo de respuesta asociado
        switch (respuesta.tipo) {
            case RESPUESTA_REGISTRO:
                
                printf("\n -> Id asociado: %d\n", respuesta.contenido.respuestaRegistro.codigo);
                printf(" -> Respuesta del servidor: %s\n", respuesta.contenido.respuestaRegistro.mensaje);
                if (strcmp(respuesta.contenido.respuestaRegistro.mensaje, "Usuario duplicado") == 0) {
                    printf(" -> Usuario no registrado correctamente\n");
                    unlink(nomPipeTalker);
                    exit(0);
                }
                else {
                    printf(" -> Usuario registrado correctamente\n");
                }
            break;

            case RESPUESTA_MENSAJE_INDIVIDUAL:
                printf(" -> MSG: %s (Usuario %d): %s\n",respuesta.contenido.mensajeIndividual.nombre,respuesta.contenido.mensajeIndividual.origen,respuesta.contenido.mensajeIndividual.mensaje);
            break;
            case RESPUESTA_CREACION_GRUPO:
                printf(" -> El grupo %d ha sido creado exitosamente\n", respuesta.contenido.creacionGrupo.id_grupo);
                printf(" -> El mensaje es: %s\n",respuesta.contenido.creacionGrupo.mensaje);
            break;
            case RESPUESTA_MENSAJE_GRUPAL:
                printf(" -> GRP: (Usuario %d): %s\n",respuesta.contenido.mensajeGrupal.origen,respuesta.contenido.mensajeGrupal.mensaje);
            break;
        }
        fflush(stdout);
    }
}

int main(int argc, char **argv) {

  signal(SIGUSR1, console_signal_handler);

  char *nomPipeManager = NULL;
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

  datos.tipo = CONSULTA_REGISTRO;
  datos.contenido.registro.idRegistro = idTalker;
  printf(" -> Usted desea ingresar a Chattering...\n");
  printf(" -> Por favor, ingresa el nombre de tu pipe: ");

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

    printf("\n_______________________________________________________________________________________________\n");
    printf("_______________________________________________________________________________________________\n");
    printf("   ____   _   _      _       _____    _____  U _____ u   ____                 _   _     ____   \n");
    printf("U /\"___| |'| |'| U  /\"\\  u  |_ \" _|  |_ \" _| \\| ___\"|/U |  _\"\\ u     ___     | \\ |\"| U /\"___|u\n");
    printf("\\| | u  /| |_| |\\ \\/ _ \\/     | |      | |    |  _|\"   \\| |_) |/    |_\"_|   <|  \\| |>\\| |  _ / \n");
    printf(" | |/__ U|  _  |u / ___ \\    /| |\\    /| |\\   | |___    |  _ <       | |    U| |\\  |u | |_| |  \n");
    printf("  \\____| |_| |_| /_/   \\_\\  u |_|U   u |_|U   |_____|   |_| \\_\\    U/| |\\u   |_| \\_|   \\____|  \n");
    printf(" _// \\\\  //   \\\\  \\\\    >>  _// \\\\_  _// \\\\_  <<   >>   //   \\\\_.-,_|___|_,-.||   \\\\,-._)(|_   \n");
    printf("(__)(__)(_\") (\"_)(__)  (__)(__) (__)(__) (__)(__) (__) (__)  (__)\\_)-' '-(_/ (_\")  (_/(__)__)  \n");
    printf("_______________________________________________________________________________________________\n");
    printf("__________________________________________T A L K E R__________________________________________\n");

    do {

        char mensaje_envio[MAX_TAM];
        fgets(mensaje_envio, MAX_TAM, stdin);
        mensaje_envio[strcspn(mensaje_envio, "\n")] = '\0';

        char command[10];
        char string[MAX_TAM];

        char* token = strtok(mensaje_envio, " ");
        if (token != NULL) {
            strcpy(command, token);
        }

        token = strtok(NULL, "'");
        if (token != NULL) {
            strcpy(string, token);
        }

        if (strcmp(command, "sent") == 0)
        {
            char mensaje[MAX_TAM];
            int number;

            sscanf(string, "\"%[^\"]\" %d", mensaje, &number);

            datos.tipo = CONSULTA_MENSAJE_INDIVIDUAL;
            datos.contenido.mensajeIndividual.origen = idTalker;
            datos.contenido.mensajeIndividual.destino = number;
            strcpy(datos.contenido.mensajeIndividual.mensaje, mensaje);
            strcpy(datos.contenido.mensajeIndividual.nombre, nomPipeTalker);

            write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));
        }

        else if(strcmp(command, "group") == 0)
        {
            datos.tipo = CONSULTA_CREACION_GRUPO;
            datos.contenido.creacionGrupo.solicitante = idTalker;
            int id_grupo;
            int tamanio = 0;

            // Extract the first integer
            sscanf(string, "%d", &id_grupo);
            
            // Extract the array of integers
            char* token = strtok(string, " ");
            if (token != NULL) {
                token = strtok(NULL, ",");
                while (token != NULL) {
                    datos.contenido.creacionGrupo.integrantes[tamanio++] = atoi(token);
                    token = strtok(NULL, ",");
                }
            }
            datos.contenido.creacionGrupo.id_grupo = id_grupo;
            datos.contenido.creacionGrupo.cantidad_integrantes = tamanio;
            write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));
        }

        else if(strcmp(command, "sentgroup") == 0)
        {
            char mensaje[MAX_TAM];
            int number;

            sscanf(string, "\"%[^\"]\" %d", mensaje, &number);

            datos.tipo = CONSULTA_MENSAJE_GRUPAL;
            datos.contenido.mensajeGrupal.origen = idTalker;
            datos.contenido.mensajeGrupal.grupo_destino = number;
            strcpy(datos.contenido.mensajeGrupal.mensaje,mensaje);

            write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente)));
        }
            
    fflush(stdout);
    } while (!salir);

}