#ifndef RESPUESTA_H
#define RESPUESTA_H

/**
 * Tamaño máximo de un mensaje + petición
 * Tamaño del buffer (utilizado para parámetros que no sean mensajes) <Peticiones>
*/
#define MAX_TAM 100
#define TAM_BUFFER 100

// Enumeración con los tipos de mensajes que puede enviar el cliente al servidor
enum TipoRespuesta {
	RESPUESTA_REGISTRO,
	RESPUESTA_LISTAR_U,
	RESPUESTA_LISTAR_G,
	RESPUESTA_MENSAJE_INDIVIDUAL,
	RESPUESTA_CREACION_GRUPO,
	RESPUESTA_MENSAJE_GRUPAL,
	RESPUESTA_SALIDA,
	ERROR,
};

// Tipos de respuestas que puedo enviar junto a sus estructuras
struct RespuestaRegistro {
	int codigo;
	char mensaje[MAX_TAM];
};

struct RespuestaListarU {
	int conectados[TAM_BUFFER];
	int tam_maximo;
};

struct RespuestaListarG {
	int integrantes[TAM_BUFFER];
	int tam_maximo;
};

struct RespuestaMensajeIndividual {
	int origen;
	int destino;
	char nombre[TAM_BUFFER];
	char mensaje[MAX_TAM];
};

struct RespuestaCreacionGrupo {
	int id_grupo;
	char mensaje[MAX_TAM];
};

struct RespuestaMensajeGrupal {
	int origen;
	char mensaje[MAX_TAM];
};

struct RespuestaSalida {
	int origen;
	char mensaje[MAX_TAM];
};

struct Error {
	char mensaje[MAX_TAM];
};

// Unión con todos los tipos de respuestas
union TipoContenidoRespuesta {
	struct RespuestaRegistro respuestaRegistro;
	struct RespuestaListarU respuestaListarU;
	struct RespuestaListarG respuestaListarG;
	struct RespuestaMensajeIndividual mensajeIndividual;
	struct RespuestaCreacionGrupo creacionGrupo;
	struct RespuestaMensajeGrupal mensajeGrupal;
	struct RespuestaSalida mensajeSalida;
	struct Error error;
};

// Datos que se envían entre cliente y servidor
struct RespuestaServidor {
	enum TipoRespuesta tipo;
	union TipoContenidoRespuesta contenido;
};



#endif