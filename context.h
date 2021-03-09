/*
* SecureWorld file context.h
contiene la estructura en C en la que se carga el contexto escrito en el fichero config.json, y que contiene la configuración de todo el sistema.
Los distintos proyectos (challenges, mirror, app descifradora etc) deben incluir este fichero para compilar pues sus DLL
se invocarán con un parámetro de tipo contexto que se definirá en context.h

Nokia Febrero 2021
*/

#ifndef context_h
#define context_h

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus

	#define NOOP ((void)0)
	#define ENABLE_PRINTS 1					// Affects the PRINT() function. If 0 does not print anything. If 1 traces are printed.
	#define PRINT(...) do { if (ENABLE_PRINTS) printf(__VA_ARGS__); else NOOP;} while (0)
	#define PRINT1(...) PRINT("    "); PRINT(__VA_ARGS__)
	#define PRINT2(...) PRINT("        "); PRINT(__VA_ARGS__)
	#define PRINT3(...) PRINT("            "); PRINT(__VA_ARGS__)
	#define PRINT4(...) PRINT("                "); PRINT(__VA_ARGS__)
	#define PRINT5(...) PRINT("                    "); PRINT(__VA_ARGS__)


	static void printContext(struct Context ctx);

	struct Context {
		struct Folder** folders;
		struct ParentalControl* parental;
		char** sync_folders;
		struct OpTable** tables;					// Cambiar a plural ////////////// y mirar el resto
		struct App** apps;
		struct ChallengeEquivalenceGroup** groups;
	} context;

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



	static void printContext(struct Context ctx) {
		// Folders
		for (int i = 0; i < _msize(ctx.folders) / sizeof(struct Folder); i++) {
			PRINT("Folder:\n");
			PRINT1("Path: %s\n", ctx.folders[i]->path);
			PRINT1("Mount point: %s\n", ctx.folders[i]->mount_point);
			PRINT1("Driver: %d\n", ctx.folders[i]->driver);
			PRINT1("Protection\n");
			PRINT2("Op table: %s\n", ctx.folders[i]->protection->op_table);
			PRINT2("Challenge groups: ");
			for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_groups) / sizeof(char*); j++) {
				printf("%s%s", ctx.folders[i]->protection->challenge_groups[j], (j + 1 < _msize(ctx.folders[i]->protection->challenge_groups)/sizeof(char*)) ? ", " : "\n");
			}
			PRINT2("Cipher: %c\n", *(ctx.folders[i]->protection->cipher));
		}
		
		// Parental control
		PRINT("Parental:\n");
		PRINT1("Folder: %s\n", ctx.parental->folder);
		PRINT1("Challenge groups: ");
		for (int i = 0; i < _msize(ctx.parental->challenge_groups) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.parental->challenge_groups[i], (i + 1 < _msize(ctx.parental->challenge_groups) / sizeof(char*)) ? ", " : "\n");
		}
		PRINT1("Users: ");
		for (int i = 0; i < _msize(ctx.parental->users) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.parental->users[i], (i + 1 < _msize(ctx.parental->users) / sizeof(char*)) ? ", " : "\n");
		}


		// Sync folders
		PRINT("Parental:\n");
		PRINT1("Sync folders: ");
		for (int i = 0; i < _msize(ctx.sync_folders) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.sync_folders[i], (i + 1 < _msize(ctx.sync_folders) / sizeof(char*)) ? ", " : "\n");
		}

		// Operative tables
		//ctx.table
		

		// Seguir con el resto...
		// TO DO
	}

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//context_h