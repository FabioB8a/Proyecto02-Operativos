#ifndef RESPUESTA_H
#define RESPUESTA_H

#define MAX_TAM 120

/// Enumeración con los tipos de mensajes que puedo enviar

enum TipoRespuesta {
	RESPUESTA_REGISTRO,
	RESPUESTA_LISTAR_U,
	RESPUESTA_LISTAR_G,
	RESPUESTA_MENSAJE_INDIVIDUAL,
	RESPUESTA_CREACION_GRUPO,
	RESPUESTA_MENSAJE_GRUPAL,

};

/// Tipos de mensaje que puedo enviar y sus estructuras

struct RespuestaRegistro {
	int codigo;
	char mensaje[MAX_TAM];
};

struct RespuestaListarU {
	int conectados[MAX_TAM];
	int tam_maximo;
};

struct RespuestaListarG {
	int integrantes[MAX_TAM];
	int tam_maximo;
};

struct RespuestaMensajeIndividual {
	int origen;
	int destino;
	char nombre[MAX_TAM];
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

/// Unión con todos los tipos de mensaje
union TipoContenidoRespuesta {
	struct RespuestaRegistro respuestaRegistro;
	struct RespuestaListarU respuestaListarU;
	struct RespuestaListarG respuestaListarG;
	struct RespuestaMensajeIndividual mensajeIndividual;
	struct RespuestaCreacionGrupo creacionGrupo;
	struct RespuestaMensajeGrupal mensajeGrupal;
};

/// Datos que se envían entre cliente y servidor
struct RespuestaServidor {
	enum TipoRespuesta tipo;
	union TipoContenidoRespuesta contenido;
};



#endif