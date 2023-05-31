#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#include "peticion.h"
#include "respuesta.h"

#define MAX_TAM 120
#define MAX_TAM_GRUPOS 30
#define MAX_TAM_POR_GRUPO 20

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

  struct Grupo {
    int id_grupo;
    int id_clientes[MAX_TAM_POR_GRUPO];
    int cantidad_clientes;
  };

  struct Cliente *arregloClientes = malloc(tam_maximo * sizeof(struct Cliente));
  int numCliente = 0;
  struct Grupo *arregloGrupos = malloc(MAX_TAM_GRUPOS * sizeof(struct Grupo));
  int numGrupo = 0;
  
  
  // Creación de pipe
  unlink(nombre_Pipe);
  mkfifo(nombre_Pipe, S_IRUSR | S_IWUSR);

  // Abrir el pipe
  int pipe_fd_general = open (nombre_Pipe, O_RDONLY | O_NONBLOCK);
  int resultado = 0; 
  int salir = 0;
  struct RespuestaServidor respuesta;
  int fd_destino;
  int indice_encontrado = -1;
  int indice_grupo_encontrado = -1;
  int creacion_exitosa = 1;

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
  printf("_________________________________________M A N A G E R_________________________________________\n");
  do {
    resultado = read(pipe_fd_general, &recibidos, sizeof(struct PeticionCliente));
    struct RespuestaServidor respuesta;
    if (resultado > 0) {
      switch (recibidos.tipo) {
        case CONSULTA_REGISTRO:
          
          respuesta.tipo = RESPUESTA_REGISTRO;
          respuesta.contenido.respuestaRegistro.codigo = recibidos.contenido.registro.idRegistro;
          // Verificación de índice encontrado
          indice_encontrado = -1;

          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.respuestaRegistro.codigo) {
                  indice_encontrado = i;
                  break;
              }
          }
          printf("\n -> Registro\n");
          if (indice_encontrado == -1) {
              printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
              printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
              strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Agregado correctamente");
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
              sleep(1);
              kill(recibidos.contenido.registro.pid, SIGUSR1);
          }
          else{
            printf("\n -> El usuario es repetido :(\n");
            strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Usuario duplicado");
            mkfifo(recibidos.contenido.registro.nombre_pipe, S_IRUSR | S_IWUSR);
            fd_destino = open(recibidos.contenido.registro.nombre_pipe, O_WRONLY | O_NONBLOCK);
            write(fd_destino, &respuesta, sizeof(struct RespuestaServidor));
            sleep(1);
            kill(recibidos.contenido.registro.pid, SIGUSR1);
          }
        break;

        case CONSULTA_MENSAJE_INDIVIDUAL:

          printf("\n -> Mensaje individual\n");
          printf(" -> El id de origen: %d\n", recibidos.contenido.mensajeIndividual.origen);
          printf(" -> El id de destino: %d\n", recibidos.contenido.mensajeIndividual.destino);
          respuesta.tipo = RESPUESTA_MENSAJE_INDIVIDUAL;
          respuesta.contenido.mensajeIndividual.origen = recibidos.contenido.mensajeIndividual.origen;
          respuesta.contenido.mensajeIndividual.destino = recibidos.contenido.mensajeIndividual.destino;
          strcpy(respuesta.contenido.mensajeIndividual.mensaje,recibidos.contenido.mensajeIndividual.mensaje);
          strcpy(respuesta.contenido.mensajeIndividual.nombre,recibidos.contenido.mensajeIndividual.nombre);
          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.mensajeIndividual.destino) {
                  indice_encontrado = i;
                  break;
              }
          }
          write(arregloClientes[indice_encontrado].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
          kill(arregloClientes[indice_encontrado].pid, SIGUSR1);
        break;

        case CONSULTA_CREACION_GRUPO:

          creacion_exitosa = 1;
          respuesta.tipo = RESPUESTA_CREACION_GRUPO;
          respuesta.contenido.creacionGrupo.id_grupo = recibidos.contenido.creacionGrupo.id_grupo;

          printf("\n -> Solicitud creación grupo\n");
          printf(" -> El id del solicitante es: %d\n", recibidos.contenido.creacionGrupo.solicitante);
          printf(" -> El id del grupo a crear es: %d\n", recibidos.contenido.creacionGrupo.id_grupo);

          // Búsqueda de grupo
          indice_grupo_encontrado = -1;
          for (int i= 0; i< numGrupo; i++){
            if(arregloGrupos[i].id_grupo == recibidos.contenido.creacionGrupo.id_grupo){
              indice_grupo_encontrado = i;
              break;
            }
          }

          if (indice_grupo_encontrado != -1){
            creacion_exitosa = -1;
          }

          // Búsqueda de creador
          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.creacionGrupo.solicitante) {
                  indice_encontrado = i;
                  break;
              }
          }

          if(indice_encontrado == -1){
            creacion_exitosa = -1;
          }

          struct Grupo nuevoGrupo;
          nuevoGrupo.id_grupo = recibidos.contenido.creacionGrupo.id_grupo;
          nuevoGrupo.cantidad_clientes = 0;

          int indice_agregado;

          // Verificar que todos los integrantes a agregar existan
          if(indice_grupo_encontrado == -1){

            for(int i = 0; i< recibidos.contenido.creacionGrupo.cantidad_integrantes; i++) {
              indice_agregado = -1;
              for (int j = 0; j < numCliente; j++) {
                if (recibidos.contenido.creacionGrupo.integrantes[i] == arregloClientes[j].id) {
                    indice_agregado = j;
                }
              }
              if (indice_agregado == -1) {
                creacion_exitosa = -1;
                break;
              }else{
                nuevoGrupo.id_clientes[nuevoGrupo.cantidad_clientes] = indice_agregado;
                nuevoGrupo.cantidad_clientes++;
              }
            }
          }

          if (creacion_exitosa == 1){
            arregloGrupos[numGrupo] = nuevoGrupo;
            numGrupo++;
            printf(" -> Grupo %d agregado exitosamente",nuevoGrupo.id_grupo);
            strcpy(respuesta.contenido.creacionGrupo.mensaje,"Creación de grupo exitosa");
            write(arregloClientes[indice_encontrado].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
            kill(arregloClientes[indice_encontrado].pid, SIGUSR1);
          } else {
            printf(" -> Ocurrió un problema a la hora de crear el grupo");
            strcpy(respuesta.contenido.creacionGrupo.mensaje,"Creación de grupo  no exitosa");
            write(arregloClientes[indice_encontrado].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
            kill(arregloClientes[indice_encontrado].pid, SIGUSR1);
          }

        break;

        case CONSULTA_MENSAJE_GRUPAL:
          printf("\n -> Mensaje grupal\n");
          printf(" -> El id de origen: %d\n", recibidos.contenido.mensajeGrupal.origen);
          printf(" -> El grupo de destino: %d\n", recibidos.contenido.mensajeGrupal.grupo_destino);
          
          indice_grupo_encontrado = -1;
          for (int i= 0; i< numGrupo; i++){
            if(arregloGrupos[i].id_grupo == recibidos.contenido.mensajeGrupal.grupo_destino){
              indice_grupo_encontrado = i;
              break;
            }
          }
          if (indice_grupo_encontrado != -1){
            respuesta.tipo = RESPUESTA_MENSAJE_GRUPAL;
            respuesta.contenido.mensajeGrupal.origen = recibidos.contenido.mensajeGrupal.origen;
            strcpy(respuesta.contenido.mensajeGrupal.mensaje,recibidos.contenido.mensajeGrupal.mensaje);
            for(int j = 0; j<arregloGrupos[indice_grupo_encontrado].cantidad_clientes; j++){
              write(arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].fd_pipe, &respuesta, sizeof(struct RespuestaServidor));
              kill(arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].pid, SIGUSR1);
            }
          }
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