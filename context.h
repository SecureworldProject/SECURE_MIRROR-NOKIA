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
	#define PRINTX(DEPTH, ...) do { if (ENABLE_PRINTS) { for (int x=0; x<DEPTH; x++){ printf("    ");} printf(__VA_ARGS__); } else NOOP;} while (0)


	static void printContext(struct Context ctx);

	struct Context {
		struct Folder** folders;
		struct ParentalControl* parental;
		char** sync_folders;
		struct OpTable** tables;									// Cambiado a plural //////////////////////////////////////////////////
		struct App** apps;											// Cambiado a plural //////////////////////////////////////////////////
		struct ChallengeEquivalenceGroup** groups;					// Cambiado a plural //////////////////////////////////////////////////
	} context;

	struct Folder {
		char* path;
		char* mount_point;
		enum Driver driver;
		struct Protection* protection;
	};

	enum Driver {
		DOKAN,
		WINFSP
	};

	struct Protection {
		char* op_table;
		char** challenge_group_ids;									// Añadido _ids // cambiar tambien en el json ////////////////////
		char* cipher;
	};

	struct ParentalControl {
		char* folder;
		char** users;
		char** challenge_group_ids;									// Añadido _ids // cambiar tambien en el json ////////////////////
	};

	struct OpTable {
		char* ID_table;
		struct TableTuple** table_tuples;							// Cambiado a plural /////////////////////////////////////////////
	};

	struct TableTuple {
		enum AppType app_type;
		char* disk;			// char  '0'=sync folders, '1'=pendrives, <letters> = manually mirrored disks
		enum Operation on_read;
		enum Operation on_write;
	};

	enum AppType {
		BROWSER,
		MAILER,
		BLOCKED,
		ANY
	};
	//const char* APP_TYPE_STRINGS[] = { "BROWSER", "MAILER", "BLOCKED", "ANY" };

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
		struct Challenge** challenges;								// Cambiado a plural //////////////////////////////////////////////////
	};

	struct Challenge {
		char* id_challenge;
		char* properties;	// “prop1=valor&prop2=valor…”
	};



	static void printContext(struct Context ctx) {
		// Folders
		PRINT("\n");
		PRINT("Folders:\n");
		for (int i = 0; i < _msize(ctx.folders) / sizeof(struct Folder*); i++) {
			PRINT1("Folder %d:\n", i);
			PRINT2("Path: %s\n", ctx.folders[i]->path);
			PRINT2("Mount point: %s\n", ctx.folders[i]->mount_point);
			PRINT2("Driver: %d\n", ctx.folders[i]->driver);
			PRINT2("Protection\n");
			PRINT3("Op table: %s\n", ctx.folders[i]->protection->op_table);
			PRINT3("Challenge groups: ");
			for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_group_ids) / sizeof(char*); j++) {
				PRINT("%s%s", ctx.folders[i]->protection->challenge_group_ids[j], (j + 1 < _msize(ctx.folders[i]->protection->challenge_group_ids)/sizeof(char*)) ? ", " : "\n");
			}
			PRINT3("Cipher: %c\n", *(ctx.folders[i]->protection->cipher));
		}
		
		// Parental control
		PRINT("\n");
		PRINT("Parental:\n");
		PRINT1("Folder: %s\n", ctx.parental->folder);
		PRINT1("Challenge groups: ");
		for (int i = 0; i < _msize(ctx.parental->challenge_group_ids) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.parental->challenge_group_ids[i], (i + 1 < _msize(ctx.parental->challenge_group_ids) / sizeof(char*)) ? ", " : "\n");
		}
		PRINT1("Users: ");
		for (int i = 0; i < _msize(ctx.parental->users) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.parental->users[i], (i + 1 < _msize(ctx.parental->users) / sizeof(char*)) ? ", " : "\n");
		}


		// Sync folders
		PRINT("\n");
		PRINT("Sync folders: ");
		for (int i = 0; i < _msize(ctx.sync_folders) / sizeof(char*); i++) {
			PRINT("%s%s", ctx.sync_folders[i], (i + 1 < _msize(ctx.sync_folders) / sizeof(char*)) ? ", " : "\n");
		}

		// Operative tables
		PRINT("\n");
		PRINT("Tables:\n");
		// Iterate over tables
		for (int i = 0; i < _msize(ctx.tables) / sizeof(struct OpTable*); i++) {
			PRINT1("Table id: %s \n", ctx.tables[i]->ID_table);
			PRINT2("Row \t\t App Type \t\t Disk \t\t Action on Read \t Action on Write\n");
			//PRINT2("|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\n");		// This is to see the tabs size in the console

			// Iterate over rows of each table (the so called "table tuples")
			for (int j = 0; j < _msize(ctx.tables[i]->table_tuples) / sizeof(struct TableTuple*); j++) {
				PRINT2("%d\t\t %d \t\t %s \t %d \t\t\t %d \n",
					j,
					ctx.tables[i]->table_tuples[j]->app_type,
					ctx.tables[i]->table_tuples[j]->disk,
					ctx.tables[i]->table_tuples[j]->on_read,
					ctx.tables[i]->table_tuples[j]->on_write);
			}
		}
		

		// Seguir con el resto...
		// TO DO
		PRINT("\n");
		PRINT("End printing...\n");
	}

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//context_h