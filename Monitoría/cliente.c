#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#include "peticion.h"
#include "respuesta.h"

#define _GNU_SOURCE
#define MAX_TAM 120

// Variables globales para indicar eventos de entrada
volatile sig_atomic_t recibido_consola_signal = 0;

int pipe_especifico = 0;
int resultado = 0;
char nomPipeTalker[MAX_TAM];
int registro = 0;

// Funcion de manejo de señales
void console_signal_handler(int signum) {
    struct RespuestaServidor respuesta;
    resultado = read(pipe_especifico, &respuesta, sizeof(struct RespuestaServidor));
    if (resultado == -1){
      // No hay datos disponibles en el momento
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
          
      } else {
          perror("Read: ");
          exit(1);
      }
    }
    if (resultado > 0) {
        // Obtener el tipo de respuesta asociado
        switch (respuesta.tipo) {
            case RESPUESTA_REGISTRO:
                
                printf("\n -> Id asociado: %d\n", respuesta.contenido.respuestaRegistro.codigo);
                printf(" -> Respuesta del servidor: %s\n", respuesta.contenido.respuestaRegistro.mensaje);
                if (strcmp(respuesta.contenido.respuestaRegistro.mensaje, "Usuario duplicado") == 0 || strcmp(respuesta.contenido.respuestaRegistro.mensaje, "Id inválido") == 0) {
                    printf(" -> Usuario no registrado correctamente\n");
                    unlink(nomPipeTalker);
                    exit(1);
                }
            break;
            case RESPUESTA_LISTAR_U:
                printf(" -> Listar usuarios conectados: ");
                for(int i=0; i<respuesta.contenido.respuestaListarU.tam_maximo; i++){
                        if (i == respuesta.contenido.respuestaListarU.tam_maximo - 1){
                            printf("U(%d)\n",respuesta.contenido.respuestaListarU.conectados[i]);
                        }
                        else{
                            printf("U(%d),",respuesta.contenido.respuestaListarU.conectados[i]);
                        }
                        
                }
            break;
            case RESPUESTA_LISTAR_G:
                if(respuesta.contenido.respuestaListarG.tam_maximo == -1){
                    printf(" -> El grupo NO existe\n");
                }
                else{
                    printf(" -> Listar usuarios del grupo: ");
                    for(int i=0; i<respuesta.contenido.respuestaListarG.tam_maximo; i++){
                         if (i == respuesta.contenido.respuestaListarU.tam_maximo - 1){
                            printf("G(%d)\n",respuesta.contenido.respuestaListarG.integrantes[i]);
                        }
                        else{
                            printf("G(%d),",respuesta.contenido.respuestaListarG.integrantes[i]);
                        }
                    }
                }
            break;
            case RESPUESTA_MENSAJE_INDIVIDUAL:
                printf(" -> MSG: %s (Usuario %d): %s\n",respuesta.contenido.mensajeIndividual.nombre,respuesta.contenido.mensajeIndividual.origen,respuesta.contenido.mensajeIndividual.mensaje);
            break;
            case RESPUESTA_CREACION_GRUPO:
                printf(" -> El mensaje es: %s\n",respuesta.contenido.creacionGrupo.mensaje);
            break;
            case RESPUESTA_MENSAJE_GRUPAL:
                printf(" -> GRP: (Usuario %d): %s\n",respuesta.contenido.mensajeGrupal.origen,respuesta.contenido.mensajeGrupal.mensaje);
            break;
            case RESPUESTA_SALIDA:
                printf(" -> RTASalida: %s\n",respuesta.contenido.mensajeSalida.mensaje);
                printf(" -> Recuerda, tu numero de Id es: %d\n",respuesta.contenido.mensajeSalida.origen);
                unlink(nomPipeTalker);
                exit(0);
            break;
            case ERROR:
                printf(" -> Error: %s\n",respuesta.contenido.error.mensaje);
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

    struct PeticionCliente datos;

    datos.tipo = CONSULTA_REGISTRO;
    datos.contenido.registro.idRegistro = idTalker;
    printf(" -> Usted desea ingresar a Chattering...\n");
    printf(" -> Por favor, ingresa el nombre de tu pipe: ");

    fgets(nomPipeTalker, MAX_TAM, stdin);
    nomPipeTalker[strcspn(nomPipeTalker, "\n")] = '\0';

    strcpy(datos.contenido.registro.nombre_pipe, nomPipeTalker);

    datos.contenido.registro.pid = getpid();

    int pipe_servidor_general = open (nomPipeManager, O_WRONLY | O_NONBLOCK);// Aquí abrir el pipe (o tenerlo abierto previamente)
    
    if (pipe_servidor_general == -1){
        perror("Open pipe_manager: ");
        exit(1);
    }
    
    if (write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
        perror("Write: ");
        exit(1);
    }

    // if (access(nomPipeTalker, F_OK) == 0)
    // {
    //     if (unlink(nomPipeTalker) == -1) {
    //         perror("Unlink: ");
    //         exit(1);
    //     }

    // }

    if (mkfifo (nomPipeTalker, S_IRUSR | S_IWUSR) == -1){
        perror("Mkfifo: ");
        exit(1);
    }

    pipe_especifico = open(nomPipeTalker, O_RDONLY | O_NONBLOCK);

    if (pipe_especifico == -1){
        perror("Open pipe_talker: ");
        exit(1);
    }

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
        fflush(stdin);

        char command[10];
        char string[MAX_TAM] = "";

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

            if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                perror("Write: ");
                exit(1);
            }
        }

        else if (strcmp(command, "list") == 0)
        {
            if(isdigit(*string))
            {
                datos.tipo = CONSULTA_LISTAR_G;
                datos.contenido.solicitudListaG.solicitante = idTalker;
                datos.contenido.solicitudListaG.id_grupo = atoi(string);
                if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                    perror("Write: ");
                    exit(1);
                }

            }
            else if (strcmp(string, "") == 0){
                datos.tipo = CONSULTA_LISTAR_U;
                datos.contenido.solicitudListaU.solicitante = idTalker;

                if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                    perror("Write: ");
                    exit(1);
                }

            }
            else {
                printf(" -> Comando incorrecto\n");
            }
        }

        else if(strcmp(command, "group") == 0)
        {
            datos.tipo = CONSULTA_CREACION_GRUPO;
            datos.contenido.creacionGrupo.solicitante = idTalker;
            int id_grupo;
            int tamanio = 0;

            sscanf(string, "%d", &id_grupo);
            
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
            if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                perror("Write: ");
                exit(1);
            }
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

            if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                perror("Write: ");
                exit(1);
            }
        }

        else if(strcmp(command, "salir") == 0)
        {
            datos.tipo = SOLICITUD_SALIDA;
            datos.contenido.solicitudSalida.solicitante = idTalker;

            if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                perror("Write: ");
                exit(1);
            }
        }
        else
        {
            printf(" -> Comando no encontrado\n");
        }
            
    fflush(stdout);
    } while (!salir);

}