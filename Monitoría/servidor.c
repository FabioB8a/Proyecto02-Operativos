/* Realizado por: Fabio Buitrago- Camilo Martinez - Luisa Parra
 * Proyecto 02: Chattering <Sistemas Operativos>
 * Pontificia Universidad Javeriana
 * Contiene: Implementación del servidor de Chattering
 * El servidor recibe los servicios de registro, listar usuarios conectados,
 * crear grupos, listar integrantes de grupo, enviar mensajes individuales,
 * enviar mensajes grupas y salir guardando la información correspondiente
*/
#include <fcntl.h> // Operaciones de control sobre descriptores
#include <stdio.h> // Entrada y salida estándar de consola C
#include <stdlib.h> // Asignación de memoria (malloc) y funciones de casteo (atoi)
#include <string.h> // Manipulación de cadenas de caracteres
#include <unistd.h> // Invocación de llamadas al sistema y operaciones del SOP (open, read, write, close)
#include <sys/types.h> // Definicion tipos de datos llamadas al sistema
#include <sys/stat.h>  // Definición funciones para obtención de información del estado de archivos
#include <signal.h> // Funciones y constantes para caputa las señales (signal)
#include <errno.h> // Proporciona macros y funciones para el manejo de errores (perror)

/**
 * Funciones utilizadas para la implementación del servidor
 */
#include "peticion.h"
#include "respuesta.h"

/**
 * Tamaño máximo de un mensaje
 * Tamaño del buffer (utilizado para parámetros que no sean mensajes) <Peticiones>
 * Tamaño máximo de grupos dentro del servidor
 * Tamaño máximo de integrantes por grupo
*/
#define MAX_TAM 100
#define TAM_BUFFER 100
#define MAX_TAM_GRUPOS 30
#define MAX_TAM_POR_GRUPO 20

/**
 * Declaración de las funciones
*/
void enviar_mensaje(struct RespuestaServidor respuesta, char* nombre_pipe, int pID);

/* * *
Función: main
Parámetros de entrada: 
    argc - número de argumentos ingresados por el usuario
    argv - arreglo que contiene los argumentos ingresados por el usuario
Valor de salida: 
    Entero que representa si el programa finalizó sin errores (0) o con algun error (1)
Descripción: 
    La función main es el punto de entrada del programa Manager. A continuación, se describe brevemente su funcionamiento:
    1. Se declara una estructura llamada PeticionCliente para almacenar los datos recibidos.
    2. Se inicializan las variables nombre_Pipe y tam_maximo con el valor NULL y 0, respectivamente.
    3. Se verifica si el número de argumentos pasados al programa es igual a 5. Si no es así, 
    se imprime un mensaje de error y se sale del programa.
    4. Se recorren los argumentos pasados al programa en pares (usando un bucle for con incremento 
    de 2).
    5. Si el argumento actual es -n y tam_maximo aún no tiene un valor asignado, se convierte el 
    siguiente argumento en un entero y se asigna a tam_maximo.
    6. Si el argumento actual es -p y nombre_Pipe aún no tiene un valor asignado, se asigna el 
    siguiente argumento a nombre_Pipe.
    7. Si ninguno de los casos anteriores se cumple, se imprime un mensaje de error y se sale del 
    programa.
    8. Se verifica si tam_maximo es menor o igual a 0. Si es así, se imprime un mensaje de error 
    y se sale del programa.
    9. Se crean estructuras de datos para almacenar información sobre clientes y grupos.
    10. Se inicializa un array infoConexion con el tamaño máximo de clientes y se establece el 
    valor de cada elemento en 0.
    11. Se verifica si ya existe un archivo con el nombre especificado en nombre_Pipe. Si existe, 
    se elimina.
    12. Se crea un nuevo archivo FIFO (pipe) con el nombre nombre_Pipe y se establecen los permisos 
    de acceso.
    13. Se abre el pipe en modo lectura y no bloqueante, y se obtiene un descriptor de archivo.
    14. Se inicializan varias variables utilizadas en el bucle principal del programa.
    15. Se muestra un mensaje de bienvenida o encabezado del programa en la salida estándar.
    
    A partir de este punto, se entra en un bucle principal que realiza las siguientes operaciones:
    1. Lee datos del pipe general en una variable recibidos de tipo PeticionCliente.
    2. Si no hay datos disponibles en el pipe, el programa verifica si la causa es que no hay datos 
    disponibles en el momento o si hay un error de lectura. En caso de que no haya datos disponibles, 
    el programa continúa con la siguiente iteración del bucle. Si hay un error de lectura, se muestra 
    un mensaje de error y se sale del programa.
    3. Si se leen datos del pipe, se realiza un procesamiento adicional según el tipo de consulta 
    especificado en recibidos.tipo.
    4. Dependiendo del tipo de consulta, se generan respuestas adecuadas y se envían a través de pipes 
    individuales a los clientes correspondientes.
    5. El bucle principal se repite hasta que se cumpla una condición para salir del bucle.
Importante: 
    Se realiza la verificación especificada para que el programa evite ejecutarse 
    si alguno de los argumentos contiene errores tanto en la digitación del usuario 
    como a la hora de realizar llamadas al sistema
* * */
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
    char nombre_asociado[TAM_BUFFER];
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
  
  // Verificación de no existencia de un pipe nominal con el mismo nombre
  // EN caso afirmativo, se realiza unlink
  if (access(nombre_Pipe, F_OK) == 0)
  {
      if (unlink(nombre_Pipe) == -1) {
          perror("Unlink: ");
          exit(1);
      }

  }

  // Crear el pipe
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
  printf("\n -> Manager iniciado y el sistema podrá tener como máximo %d usuarios\n",tam_maximo);
  printf("\n_______________________________________________________________________________________________\n");
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

        // Registro
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
          if (pipe_encontrado != -1 && infoConexion[pipe_encontrado] == 1){ 
            printf(" -> Pipe repetido\n");
            printf(" -> El nombre del pipe es: %s\n", recibidos.contenido.registro.nombre_pipe);
            printf(" -> El ID del pipe es: %d\n", recibidos.contenido.registro.idRegistro);
          }
          // Si se supera el límite de usuarios
          else if (numCliente == tam_maximo){
            printf(" -> Tamaño maximo de clientes alcanzado\n");
            strcpy(respuesta.contenido.respuestaRegistro.mensaje, "Alcance maximo alcanzado");
            sleep(1);
            enviar_mensaje(respuesta,recibidos.contenido.registro.nombre_pipe,recibidos.contenido.registro.pid);
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
                infoConexion[indice_encontrado] = 1;
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

        // Consultar lista usuarios conectados
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
          respuesta.contenido.respuestaListarU.tam_maximo = 0;
          for (int i=0; i<numCliente; i++){
            if (infoConexion[i] == 1){
              respuesta.contenido.respuestaListarU.conectados[i] = arregloClientes[i].id;
              respuesta.contenido.respuestaListarU.tam_maximo++;
            }
          }
          enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);

        break;

        // Consultar lista de integrantes de grupo
        case CONSULTA_LISTAR_G:

          respuesta.tipo = RESPUESTA_LISTAR_G;
          printf("\n -> Solicitud Listar Integrantes Grupo\n");
          printf(" -> El usuario que solicita el servicio es: %d\n",recibidos.contenido.solicitudListaG.solicitante);
          printf(" -> El grupo consultado es: G%d\n",recibidos.contenido.solicitudListaG.id_grupo);
          printf(" -> numGrupo: %d\n", numGrupo);

          indice_grupo_encontrado = -1;
          for (int i= 0; i< numGrupo; i++){
            printf(" -> ID: %d\n", arregloGrupos[i].id_grupo );
            if(arregloGrupos[i].id_grupo == recibidos.contenido.solicitudListaG.id_grupo){
              indice_grupo_encontrado = i;
              break;
            }
          }

          indice_encontrado = -1;
          for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.solicitudListaG.solicitante) {
                  indice_encontrado = i;
                  break;
              }
          }
          if(indice_grupo_encontrado == -1){
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

        // Mensaje individual
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
          if(indice_encontrado != -1 && infoConexion[indice_encontrado] == 1){
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }
          else{
            respuesta.tipo = ERROR;
            if (infoConexion[indice_encontrado] == 1){
              printf("Usuario no conectado");
            strcpy(respuesta.contenido.error.mensaje, "Usuario no conectado");
            }
            else{
              printf("Usuario no añadido");
              strcpy(respuesta.contenido.error.mensaje, "Usuario no añadido");
            }

            indice_encontrado = -1;
            for (int i = 0; i < numCliente; i++) {
              if (arregloClientes[i].id == recibidos.contenido.mensajeIndividual.origen) {
                  indice_encontrado = i;
                  break;
              }
            }
            printf("EL indice es: %d",indice_encontrado);
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }

        break;

        // Crear grupo
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
            sprintf(respuesta.contenido.creacionGrupo.mensaje, "Creación de grupo %d exitoso", nuevoGrupo.id_grupo);

            for(int i=0; i<nuevoGrupo.cantidad_clientes; i++){
              enviar_mensaje(respuesta,arregloClientes[nuevoGrupo.id_clientes[i]].nombre_asociado,arregloClientes[nuevoGrupo.id_clientes[i]].pid);
            }

          } else {
            printf(" -> Ocurrió un problema a la hora de crear el grupo\n");
            strcpy(respuesta.contenido.creacionGrupo.mensaje,"Creación de grupo  no exitosa - Id usuarios inválidos");
            enviar_mensaje(respuesta,arregloClientes[indice_encontrado].nombre_asociado,arregloClientes[indice_encontrado].pid);
          }

        break;

        // Mensaje grupal
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
              if (infoConexion[arregloGrupos[indice_grupo_encontrado].id_clientes[j]] == 1 && recibidos.contenido.mensajeGrupal.origen != arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].id)
              {
                enviar_mensaje(respuesta,arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].nombre_asociado,arregloClientes[arregloGrupos[indice_grupo_encontrado].id_clientes[j]].pid);
              }
            }
          }
          else {
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

        // Salida
        case SOLICITUD_SALIDA:

          printf("\n -> Mensaje solicitud salida\n");
          printf(" -> El id a salir es: %d\n", recibidos.contenido.solicitudSalida.solicitante);

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

/**
Función: enviar_mensaje
Parámetros:
    struct RespuestaServidor respuesta: Respuesta a enviar por el pipe Nominal
    char* nombre_pipe: Nombre del pipe, para asociar al descriptor
    int pID: pID del proceso Talker, utilizado para realizar la Signal
Valor de salida:
    Ninguno
Descripción:
    1. Abre el archivo de tubería con el nombre especificado por nombre_pipe utilizando la función 
    open. El modo de apertura se establece en escritura solamente (O_WRONLY) y se habilita la apertura 
    no bloqueante (O_NONBLOCK).
    Si la llamada a open devuelve -1, indica que se produjo un error al abrir el archivo de tubería. 
    En ese caso, se imprime un mensaje de error utilizando perror y se sale de la función con exit(1).

    2. Escribe la estructura respuesta en el archivo de tubería utilizando la función write. La estructura 
    se pasa como un puntero a través del operador &. El tamaño de la estructura se especifica mediante 
    sizeof(struct RespuestaServidor).
    Si la llamada a write devuelve -1, indica que se produjo un error al escribir en el archivo de tubería. 
    En ese caso, se imprime un mensaje de error utilizando perror y se sale de la función con exit(1).

    3. Envía la señal SIGUSR1 al proceso con el ID especificado por pID utilizando la función kill. Esta señal 
    se utiliza para notificar al proceso destinatario que hay un mensaje disponible en la tubería.
    Si la llamada a kill devuelve -1, indica que se produjo un error al enviar la señal. En ese caso, se imprime 
    un mensaje de error utilizando perror y se sale de la función con exit(1).

    4. Cierra el archivo de tubería utilizando la función close.
    Si la llamada a close devuelve -1, indica que se produjo un error al cerrar el archivo de tubería. En ese caso, 
    se imprime un mensaje de error utilizando perror y se sale de la función con exit(1).
**/
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