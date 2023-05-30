#ifndef DATOS_H
#define DATOS_H

#define MAX_TAM 120

/// Enumeración con los tipos de mensajes que puedo enviar

enum TipoPeticion {
	REGISTRO,
	MENSAJE_INDIVIDUAL,
	MENSAJE_GRUPAL,
	CREACION_GRUPO,
	UNIR_GRUPO,
	DESCONEXION_GRUPO
};

/// Tipos de mensaje que puedo enviar y sus estructuras

struct Registro {
	int idRegistro;
	int pid;
	char nombre_pipe[MAX_TAM];
};

struct MensajeIndividual {
	int origen;
	int destino;
	char nombre[MAX_TAM];
	char mensaje[MAX_TAM];
};

struct MensajeGrupal {
	int origen;
	int grupo_destino;
	char mensaje[MAX_TAM];
};

struct CreacionGrupo {
	int solicitante;
	int id_grupo;
	char nombre_grupo[MAX_TAM];
};

struct UnirGrupo {
	int solicitante;
	int id_grupo;
};

struct DesconexionGrupo {
	int solicitante;
	int id_grupo;
};

/// Unión con todos los tipos de mensaje
union TipoContenido {
	struct Registro registro;
	struct MensajeIndividual mensajeIndividual;
	struct MensajeGrupal mensajeGrupal;
	struct CreacionGrupo creacionGrupo;
	struct UnirGrupo unirGrupo;
	struct DesconexionGrupo desconexionGrupo;
};

/// Datos que se envían entre cliente y servidor
struct PeticionCliente {
	enum TipoPeticion tipo;
	union TipoContenido contenido;
};



#endif