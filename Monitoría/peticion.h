#ifndef DATOS_H
#define DATOS_H

#define MAX_TAM 120
#define MAX_TAM_GRUPOS 30
#define MAX_TAM_POR_GRUPO 20

/// Enumeración con los tipos de mensajes que puedo enviar

enum TipoPeticion {
	CONSULTA_REGISTRO,
	CONSULTA_MENSAJE_INDIVIDUAL,
	CONSULTA_CREACION_GRUPO,
	CONSULTA_MENSAJE_GRUPAL,
	UNIR_GRUPO,
	DESCONEXION_GRUPO
};

/// Tipos de mensaje que puedo enviar y sus estructuras

struct SolicitudRegistro {
	int idRegistro;
	int pid;
	char nombre_pipe[MAX_TAM];
};

struct SolicitudMensajeIndividual {
	int origen;
	int destino;
	char nombre[MAX_TAM];
	char mensaje[MAX_TAM];
};

struct SolicitudCreacionGrupo {
	int solicitante;
	int id_grupo;
	int integrantes[MAX_TAM_POR_GRUPO];
	int cantidad_integrantes;
};

struct SolicitudMensajeGrupal {
	int origen;
	int grupo_destino;
	char mensaje[MAX_TAM];
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
	struct SolicitudRegistro registro;
	struct SolicitudMensajeIndividual mensajeIndividual;
	struct SolicitudCreacionGrupo creacionGrupo;
	struct SolicitudMensajeGrupal mensajeGrupal;
	struct UnirGrupo unirGrupo;
	struct DesconexionGrupo desconexionGrupo;
};

/// Datos que se envían entre cliente y servidor
struct PeticionCliente {
	enum TipoPeticion tipo;
	union TipoContenido contenido;
};



#endif