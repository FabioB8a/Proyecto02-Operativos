#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#include "peticion.h"
#include "respuesta.h"

#define MAX_TAM 120
#define MAX_TAM_GRUPOS 30
#define MAX_TAM_POR_GRUPO 20

// TOCA HACER TODOS LOS PERROR
void enviar_mensaje(struct RespuestaServidor respuesta, char* nombre_pipe, int pID);

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
  
  // NO REGISTRADO = 0
  // CONECTADO = 1
  // NO CONECTADO = -1
  int infoConexion[tam_maximo];
  for(int i=0; i<tam_maximo; i++){
    infoConexion[i] = 0;
  }
  
  // Creación de pipe
  if (access(nombre_Pipe, F_OK) == 0)
  {
      if (unlink(nombre_Pipe) == -1) {
          perror("Unlink: ");
          exit(1);
      }

  }

  if (mkfifo (nombre_Pipe, S_IRUSR | S_IWUSR) == -1)
  {
      perror("Mkfifo: ");
      exit(1);
  }

  // Abrir el pipe
  int pipe_fd_general = open (nombre_Pipe, O_RDONLY | O_NONBLOCK);
  if (pipe_fd_general == -1){
    perror("Open: ");
    exit(1);
  }

  int resultado = 0; 
  int salir = 0;
  struct RespuestaServidor respuesta;
  int fd_destino;
  int indice_encontrado = -1;
  int pipe_encontrado = -1;
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
    if (resultado == -1){
      // No hay datos disponibles en el momento
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
          continue;
      } else {
          perror("Read: ");
          exit(1);
      }
    }
    if (resultado > 0) {
      switch (recibidos.tipo) {
        case CONSULTA_REGISTRO:
          
          respuesta.tipo = RESPUESTA_REGISTRO;
          respuesta.contenido.respuestaRegistro.codigo = recibidos.contenido.registro.idRegistro;

          indice_encontrado = -1;
          pipe_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.registro.idRegistro) {
                  indice_encontrado = i;
              }
              if(strcmp(arregloClientes[i].nombre_asociado,recibidos.contenido.registro.nombre_pipe) == 0){
                  pipe_encontrado = i;
              }
          }

          printf("\n -> Registro\n");
          // Pipe repetido (No se envía mensaje ya que en el cliente se mata el proceso)
          if (pipe_encontrado != -1 && infoConexion[indice_encontrado] == 1){ 
            printf(" -> Pipe repetido\n");
            printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
            printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
          }
          // En caso de que no exista pipe repetido
          else {
            // NO existe el índice
            if (indice_encontrado == -1) {
                // No es válido
                if(recibidos.contenido.registro.idRegistro < 1 || recibidos.contenido.registro.idRegistro > tam_maximo)
                {
                  printf(" -> Id Inválido, debe ser mayor o igual a 1 y menor o igual a %d\n",tam_maximo);
                  printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
                  printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
                  strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Id inválido");
                }
                // Registro
                else {
                  printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
                  printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
                  strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Agregado correctamente");
                  struct Cliente nuevoCliente;
                  nuevoCliente.id = recibidos.contenido.registro.idRegistro;
                  strcpy(nuevoCliente.nombre_asociado, recibidos.contenido.registro.nombre_pipe);
                  nuevoCliente.pid = recibidos.contenido.registro.pid;
                  arregloClientes[numCliente] = nuevoCliente;
                  infoConexion[numCliente] = 1;
                  numCliente++;
                }
            }
            // ID repetido
            else{
              // Desconectado (Encontra el indice y está desconectado)
              if (indice_encontrado != -1  &&  infoConexion[indice_encontrado] == -1)
              {
                printf(" -> Reinicio de conexión\n");
                printf(" -> El nuevo nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
                printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
                strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Usuario reconectado");
                strcpy(arregloClientes[indice_encontrado].nombre_asociado, recibidos.contenido.registro.nombre_pipe);
                arregloClientes[indice_encontrado].pid = recibidos.contenido.registro.pid;
                infoConexion[numCliente] = 1;
              }

              // Conectado actualmente
              else
              {
                printf(" -> Usuario repetido\n");
                printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
                printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
                strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Usuario duplicado");
              }
            }
            sleep(1);
            enviar_mensaje(respuesta,recibidos.contenido.registro.nombre_pipe,recibidos.contenido.registro.pid);
          }


        break;

        case CONSULTA_LISTAR_U:

          respuesta.tipo = RESPUESTA_LISTAR_U;
          printf("\n -> Solicitud Listar Usuarios Conectados\n");
          printf(" -> El usuario que solicita el servicio es: %d\n",recibidos.contenido.solicitudListaU.solicitante);

          // Verificación de índice encontrado
          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.solicitudListaU.solicitante) {
                  indice_encontrado = i;
                  break;
              }
          }
          respuesta.contenido.respuestaListarU.tam_maximo = numCliente;
          for (int i=0; i<numCliente; i++){
            respuesta.contenido.respuestaListarU.conectados[i] = arregloClientes[i].id;
          }
          enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);

        break;

        case CONSULTA_LISTAR_G:

          respuesta.tipo = RESPUESTA_LISTAR_G;
          printf("\n -> Solicitud Listar Integrantes Grupo\n");
          printf(" -> El usuario que solicita el servicio es: %d\n",recibidos.contenido.solicitudListaG.solicitante);
          printf(" -> El grupo consultado es: G%d\n",recibidos.contenido.solicitudListaG.id_grupo);
          printf(" -> numGrupo: %d\n", numGrupo);

          // Búsqueda de grupo
          indice_grupo_encontrado = -1;
          for (int i= 0; i< numGrupo; i++){
            printf(" -> ID: %d\n", arregloGrupos[i].id_grupo );
            if(arregloGrupos[i].id_grupo == recibidos.contenido.solicitudListaG.id_grupo){
              indice_grupo_encontrado = i;
              break;
            }
          }

          // Verificación de índice encontrado
          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.solicitudListaG.solicitante) {
                  indice_encontrado = i;
                  break;
              }
          }
          if(indice_grupo_encontrado == -1){
            printf(" -> Indice NO encontrado, el grupo no existe\n");
            respuesta.contenido.respuestaListarG.tam_maximo = -1;
          }
          else{
            respuesta.contenido.respuestaListarG.tam_maximo = arregloGrupos[indice_grupo_encontrado].cantidad_clientes;
            for (int i=0; i< respuesta.contenido.respuestaListarG.tam_maximo; i++){
              respuesta.contenido.respuestaListarG.integrantes[i] = arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[i]].id;
            }
          }
          enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
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
          if(indice_encontrado != -1){
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }
          else{
            indice_encontrado = -1;
            for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == respuesta.contenido.mensajeIndividual.origen) {
                  indice_encontrado = i;
                  break;
            }
          }
          respuesta.tipo = ERROR;
          strcpy(respuesta.contenido.error.mensaje, "Usuario inválido");
          enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }

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
            printf(" -> Grupo %d agregado exitosamente\n",nuevoGrupo.id_grupo);
            strcpy(respuesta.contenido.creacionGrupo.mensaje,"Creación de grupo exitosa");
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          } else {
            printf(" -> Ocurrió un problema a la hora de crear el grupo\n");
            strcpy(respuesta.contenido.creacionGrupo.mensaje,"Creación de grupo  no exitosa");
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
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
              enviar_mensaje(respuesta,arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].nombre_asociado,arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].pid);
            }
          }
          else {
            // Búsqueda de creador
            indice_encontrado = -1;
            for (int i = 0; i < numCliente; i++) {
                if (arregloClientes[i].id == recibidos.contenido.mensajeGrupal.origen) {
                    indice_encontrado = i;
                    break;
            }
            }
            respuesta.tipo = ERROR;
            strcpy(respuesta.contenido.error.mensaje, "Grupo inválido");
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }
        break;

        case SOLICITUD_SALIDA:

          respuesta.tipo = RESPUESTA_SALIDA;
          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.solicitudSalida.solicitante) {
                  indice_encontrado = i;
                  break;
              }
          }
          infoConexion[indice_encontrado] = -1;
          respuesta.contenido.mensajeSalida.origen = recibidos.contenido.solicitudSalida.solicitante;
          strcpy(respuesta.contenido.mensajeSalida.mensaje, "Ha logrado salir correctamente, datos almacenados");
          enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);

        break;
        
        }
    }
    fflush(stdout);
} while (!salir);

close(pipe_fd_general);
}

void enviar_mensaje(struct RespuestaServidor respuesta, char* nombre_pipe, int pID)
{
  int file_descriptor;
  file_descriptor = open(nombre_pipe, O_WRONLY | O_NONBLOCK);
  if (file_descriptor == -1){
    perror("Open: ");
    exit(1);
  }
  if(write(file_descriptor, &respuesta, sizeof(struct RespuestaServidor)) == -1){
    perror("Write: ");
    exit(1);
  }
  if(kill(pID, SIGUSR1) == -1){
    perror("Kill: ");
    exit(1);
  }
  if (close(file_descriptor) == -1){
  perror("Close: ");
  exit(1);
  }
}