/*
* SecureWorld file context.h
contiene la estructura en C en la que se carga el contexto escrito en el fichero config.json, y que contiene la configuración de todo el sistema.
Los distintos proyectos (challenges, mirror, app descifradora etc) deben incluir este fichero para compilar pues sus DLL
se invocarán con un parámetro de tipo contexto que se definirá en context.h

Nokia Febrero 2021
*/

#ifndef context_h
#define context_h

struct Context {
	struct Folder** folders;
	struct ParentalControl* parental;
	char** sync_folders;
	struct OpTable** table;
	struct App** app;
	struct ChallengeEquivalenceGroup** group;
};

struct Folder {
	char* path;
	char* mount_point; //El json lo trata como string directamente
	enum Driver driver;
	struct Protection* protection;
};

enum Driver {
	DOKAN,
	WINFSP
};

struct Protection {
	char* op_table;
	char** challenge_groups;
	char* cipher;
};

struct ParentalControl {
	char* folder;
	char** users;
	char** challenge_groups;
};

struct OpTable {
	char* ID_table;
	struct TableTuple** table_tuple;
};

struct TableTuple {
	enum AppType app_type;
	char* disk;		// “0”=sync folders, “1”=pendrives, <letters> = mirrored disks
	enum Operation on_read;
	enum Operation on_write;
};

enum AppType {
	BROWSER,
	MAILER,
	BLOCKED,
	ANY
};
//const char* APP_TYPE_STRINGS[] = { "BROWSER", "MAILER", "BLOCKED","ANY" };

enum Operation {
	NOTHING,
	CIPHER,
	DECIPHER,
	MARK,
	UNMARK,
	IF_MARK_UNMARK_ELSE_CIPHER,
	IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING
};

struct App {
	char* app_path;
	char* app_name;
	enum AppType app_type;
};

struct ChallengeEquivalenceGroup {
	char* id;
	char* subkey;
	char* expires;			// YYMMDDhhmmss
	struct Challenge** challenge;
};

struct Challenge {
	char* id_challenge;
	char* properties;	// “prop1=valor&prop2=valor…”
};

#endif