#ifndef RESPUESTA_H
#define RESPUESTA_H

#define MAX_TAM 120

/// Enumeración con los tipos de mensajes que puedo enviar

enum TipoRespuesta {
	RESPUESTA,
	MENSAJE,
};

/// Tipos de mensaje que puedo enviar y sus estructuras

struct Respuesta {
	int codigo;
	char mensaje[MAX_TAM];
};

struct Mensaje {
	int origen;
	int destino;
	char mensaje[MAX_TAM];
};

/// Unión con todos los tipos de mensaje
union TipoContenidoRespuesta {
	struct Respuesta respuesta;
	struct Mensaje mensaje;
};

/// Datos que se envían entre cliente y servidor
struct RespuestaServidor {
	enum TipoRespuesta tipo;
	union TipoContenidoRespuesta contenido;
};



#endif