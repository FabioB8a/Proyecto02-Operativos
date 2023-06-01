#ifndef DATOS_H
#define DATOS_H

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

// Enumeración con los tipos de peticiones que puede enviar el cliente al servidor
enum TipoPeticion {
	CONSULTA_REGISTRO,
	CONSULTA_LISTAR_U,
	CONSULTA_LISTAR_G,
	CONSULTA_MENSAJE_INDIVIDUAL,
	CONSULTA_CREACION_GRUPO,
	CONSULTA_MENSAJE_GRUPAL,
	SOLICITUD_SALIDA,
};

// Tipos de peticiones que puedo enviar junto a sus estructuras
struct SolicitudRegistro {
	int idRegistro;
	int pid;
	char nombre_pipe[TAM_BUFFER];
};

struct SolicitudListaU {
	int solicitante;
};

struct SolicitudListaG {
	int solicitante;
	int id_grupo;
};

struct SolicitudMensajeIndividual {
	int origen;
	int destino;
	char nombre[TAM_BUFFER];
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

struct SolicitudSalida {
	int solicitante;
};

struct UnirGrupo {
	int solicitante;
	int id_grupo;
};

struct DesconexionGrupo {
	int solicitante;
	int id_grupo;
};

// Unión con todos los tipos de peticiones
union TipoContenido {
	struct SolicitudRegistro registro;
	struct SolicitudListaU solicitudListaU;
	struct SolicitudListaG solicitudListaG;
	struct SolicitudMensajeIndividual mensajeIndividual;
	struct SolicitudCreacionGrupo creacionGrupo;
	struct SolicitudMensajeGrupal mensajeGrupal;
	struct SolicitudSalida solicitudSalida;
	struct UnirGrupo unirGrupo;
	struct DesconexionGrupo desconexionGrupo;
};

// Datos que se envían entre cliente y servidor
struct PeticionCliente {
	enum TipoPeticion tipo;
	union TipoContenido contenido;
};



#endif