/* Realizado por: Fabio Buitrago- Camilo Martinez - Luisa Parra
 * Proyecto 02: Chattering <Sistemas Operativos>
 * Pontificia Universidad Javeriana
 * Contiene: Implementación de cliente de Chattering
 * El cliente solicita los servicios de registro, listar usuarios conectados,
 * crear grupos, listar integrantes de grupo, enviar mensajes individuales,
 * enviar mensajes grupas y salir guardando la información correspondiente
*/

/**
 * Funciones utilizadas para la implementación del cliente
*/
#include <fcntl.h> // Operaciones de control sobre descriptores
#include <stdio.h> // Entrada y salida estándar de consola C
#include <stdlib.h> // Asignación de memoria (malloc) y funciones de casteo (atoi)
#include <string.h> // Manipulación de cadenas de caracteres
#include <unistd.h> // Invocación de llamadas al sistema y operaciones del SOP (open, read, write, close)
#include <sys/types.h> // Definicion tipos de datos llamadas al sistema
#include <sys/stat.h>  // Definición funciones para obtención de información del estado de archivos
#include <signal.h> // Funciones y constantes para caputa las señales (signal)
#include <ctype.h> // Comprobar tipo de dato
#include <errno.h> // Proporciona macros y funciones para el manejo de errores (perror)

/**
 * Llamado de cabecera peticion.h y respuesta.h
 * Utilizado para enviar las peticiones y recibir las respuestas del servidor
 * en estructuras por medio de pipes nominales
*/
#include "peticion.h"
#include "respuesta.h"

/**
 * Tamaño máximo de un mensaje + petición
 * Tamaño del buffer (utilizado para parámetros que no sean mensajes) <Peticiones>
*/
//#define _GNU_SOURCE
#define MAX_TAM 100
#define TAM_BUFFER 100

/**
 * Variables globales
*/

volatile sig_atomic_t recibido_consola_signal = 0; //Indicar la señal enviada por el servidor

int pipe_especifico = 0; // File Descriptor asociado al pipe específico de Talker a Servidor
int resultado = 0; // CAntidad de información recibida en bytes leídos (En caso de ocurrir un error retorna -1 o al EOF retorna 0)
char nomPipeTalker[TAM_BUFFER]; // Nombre indicado para el pipe especificado por el Talker

/**
Función: console_signal_handler
Parámetros:
    signum - número de señal asociado
Valor de salida:
    Ninguno
Descripción:
    1. Se declara una variable respuesta de tipo struct RespuestaServidor para almacenar 
    la respuesta recibida.
    2. Se utiliza la función read para leer datos del pipe específico, guardando el resultado 
    en la variable resultado.
    3. Si resultado es igual a -1, se verifica si el error se debe a que no hay datos 
    disponibles en el momento. En caso afirmativo, no se realiza ninguna acción adicional. De lo contrario, se muestra un mensaje de error utilizando perror y se sale del programa.
    4. Si resultado es mayor a 0, indica que se ha leído con éxito una respuesta del pipe.
    5. Se utiliza una estructura de control switch para determinar el tipo de respuesta 
    asociado a la variable respuesta.tipo.
    6. Dependiendo del tipo de respuesta, se muestra información específica en la consola 
    utilizando printf. Esto puede incluir datos como el ID asociado, mensajes de respuesta del servidor, lista de usuarios conectados, lista de usuarios en un grupo, mensajes individuales, mensajes grupales, mensajes de creación de grupo, mensajes de salida, o mensajes de error.
    7. Finalmente, se llama a fflush(stdout) para asegurarse de que los datos se impriman 
    inmediatamente en la salida estándar.
*/
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
                if (strcmp(respuesta.contenido.respuestaRegistro.mensaje, "Agregado correctamente") != 0) {
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

/* * *
Función: main
Parámetros de entrada: 
    argc - número de argumentos ingresados por el usuario
    argv - arreglo que contiene los argumentos ingresados por el usuario
Valor de salida: 
    Entero que representa si el programa finalizó sin errores (0) o con algun error (1)
Descripción: 
    La función main es el punto de entrada del programa Talker. A continuación, se describe brevemente su funcionamiento:
    1. Se registra la función console_signal_handler como controlador de la señal SIGUSR1.
    2. Se declaran variables locales, como nomPipeManager y idTalker, para almacenar los 
    argumentos pasados por línea de comandos.
    3. Se verifica si el número de argumentos pasados es igual a 5. En caso contrario, 
    se muestra un mensaje de error y se sale del programa.
    4. Se recorren los argumentos pasados por línea de comandos utilizando un bucle. Se asigna 
    el valor correspondiente a idTalker si se encuentra el argumento -i, y se asigna el valor 
    correspondiente a nomPipeManager si se encuentra el argumento -p. Si se encuentra un 
    argumento inválido o repetido, se muestra un mensaje de error y se sale del programa.
    5. Se verifica si idTalker es menor o igual a 0. En caso afirmativo, se muestra un mensaje 
    de error y se sale del programa.
    6. Se crea una variable datos de tipo struct PeticionCliente y se configuran sus campos 
    con valores correspondientes a una consulta de registro.
    7. Se solicita al usuario ingresar el nombre del pipe a utilizar mediante la función 
    fgets, y se elimina el carácter de nueva línea del final de la cadena.
    8. Se copia el nombre del pipe en el campo correspondiente de datos.
    9. Se obtiene el ID del proceso actual utilizando getpid().
    10. Se abre el pipe nomPipeManager en modo escritura no bloqueante y se guarda el 
    descriptor de archivo en pipe_servidor_general. Si la apertura del pipe falla, se muestra 
    un mensaje de error y se sale del programa.
    11. Se escribe la estructura datos en el pipe utilizando write. Si la escritura falla, 
    se muestra un mensaje de error y se sale del programa.
    12. Se crea el pipe nomPipeTalker utilizando mkfifo con los permisos adecuados.
    13. Se abre el pipe nomPipeTalker en modo lectura no bloqueante y se guarda el descriptor 
    de archivo en pipe_especifico. Si la apertura del pipe falla, se muestra un mensaje de 
    error y se sale del programa.
    14. Se muestra un banner de presentación en la consola.
    15. Se inicia un bucle que permite al usuario ingresar comandos desde la consola.
    16. Se lee el comando ingresado utilizando fgets y se elimina el carácter de nueva 
    línea del final de la cadena.
    17. Se realiza un análisis del comando ingresado para determinar su tipo y extraer los 
    argumentos correspondientes.
    18. Dependiendo del tipo de comando, se configuran los campos de datos con los valores 
    adecuados.
    19. Se escribe la estructura datos en el pipe pipe_servidor_general utilizando write. 
    Si la escritura falla, se muestra un mensaje de error y se sale del programa.
    20. Se repite el bucle hasta que la variable salir se establezca en un valor diferente de 0.
Importante: 
    Se realiza la verificación especificada para que el programa evite ejecutarse 
    si alguno de los argumentos contiene errores tanto en la digitación del usuario 
    como a la hora de realizar llamadas al sistema
* * */
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

    fgets(nomPipeTalker, TAM_BUFFER, stdin);
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

    // Opcional, verificación que el pipe no existe, en caso de que existe se borra utilizando unlink
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
    printf("\n -> Comandos\n");
    printf(" -> sent \"<Mensaje>\" id: Máximo de 100 caracteres y el id debe estar conectado\n");
    printf(" -> list: Listar todos los usuarios conectados\n");
    printf(" -> list <GID>: Listar todos los usuarios del grupo GID\n");
    printf(" -> group <GID> <id1, id2, id3... idN>: Crea el grupo con id GID y despues los integrantes\n");
    printf(" -> sentgroup \"<Mensaje>\" GID: Máximo de 100 caracteres y el id del grupo debe existir\n");
    printf(" -> salir: Salir del programa y guardar el id asociado\n");
    printf("\n_______________________________________________________________________________________________\n");
    printf("__________________________________________T A L K E R__________________________________________\n");
    do {

        char mensaje_envio[MAX_TAM+TAM_BUFFER];
        fgets(mensaje_envio, MAX_TAM+TAM_BUFFER, stdin);
        mensaje_envio[strcspn(mensaje_envio, "\n")] = '\0';
        fflush(stdin);

        char command[10];
        char string[MAX_TAM+TAM_BUFFER] = "";

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

            if(sscanf(string, "\"%[^\"]\" %d", mensaje, &number) != 2){
                printf(" -> Recuerda, el id es un numero\n");
            }
            else {
                datos.tipo = CONSULTA_MENSAJE_INDIVIDUAL;
                datos.contenido.mensajeIndividual.origen = idTalker;
                datos.contenido.mensajeIndividual.destino = number;
                if (strlen(mensaje) >= MAX_TAM){
                    printf(" -> El mensaje debe contener menos de %d caracteres\n",MAX_TAM);
                }else{
                    strcpy(datos.contenido.mensajeIndividual.mensaje, mensaje);
                    strcpy(datos.contenido.mensajeIndividual.nombre, nomPipeTalker);

                    if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                        perror("Write: ");
                        exit(1);
                    }
                }
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

            if (sscanf(string, "%d", &id_grupo) == 1){
                int valido = 1;
                char* token = strtok(string, " ");
                if (token != NULL) {
                    token = strtok(NULL, ",");
                    if(!isdigit(*token)){
                        valido = 0;
                    }
                    while (token != NULL) {
                        datos.contenido.creacionGrupo.integrantes[tamanio++] = atoi(token);
                        if(!isdigit(*token)){
                            valido = 0;
                        }
                        token = strtok(NULL, ",");
                    }
                }
                if(valido == 1){
                    datos.contenido.creacionGrupo.id_grupo = id_grupo;
                    datos.contenido.creacionGrupo.cantidad_integrantes = tamanio;
                    if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                        perror("Write: ");
                        exit(1);
                    }
                }
                else{
                    printf(" -> ID usuario no válido, recuerde que es un numero\n");
                }
            }
            else {
                printf(" -> El GID debe ser un entero\n");
            }
        }

        else if(strcmp(command, "sentgroup") == 0)
        {
            char mensaje[MAX_TAM];
            int number;

            if(sscanf(string, "\"%[^\"]\" %d", mensaje, &number) != 2){
                printf(" -> Recuerda, el GID es un numero\n");
            }
            else {

                datos.tipo = CONSULTA_MENSAJE_GRUPAL;
                datos.contenido.mensajeGrupal.origen = idTalker;
                datos.contenido.mensajeGrupal.grupo_destino = number;
                strcpy(datos.contenido.mensajeGrupal.mensaje,mensaje);

                if(write(pipe_servidor_general, &datos, (sizeof(struct PeticionCliente))) == -1){
                    perror("Write: ");
                    exit(1);
                }
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