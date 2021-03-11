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

	#define NULL (void*)0
	#define NOOP ((void)0)
	#define ENABLE_PRINTS 1					// Affects the PRINT() function. If 0 does not print anything. If 1 traces are printed.
	#define PRINT(...) do { if (ENABLE_PRINTS) printf(__VA_ARGS__); else NOOP;} while (0)
	#define PRINT1(...) PRINT("    "); PRINT(__VA_ARGS__)
	#define PRINT2(...) PRINT("        "); PRINT(__VA_ARGS__)
	#define PRINT3(...) PRINT("            "); PRINT(__VA_ARGS__)
	#define PRINT4(...) PRINT("                "); PRINT(__VA_ARGS__)
	#define PRINT5(...) PRINT("                    "); PRINT(__VA_ARGS__)
	#define PRINTX(DEPTH, ...) do { if (ENABLE_PRINTS) { for (int x=0; x<DEPTH; x++){ printf("    ");} printf(__VA_ARGS__); } else NOOP;} while (0)


	static void printContext();
	static void printChallengeGroups();
	static void printDateNice(char* date);
	static void printChallengeGroup(char *id);
	static struct ChallengeEquivalenceGroup* getChallengeGroup(char* id);

	///////////////////////////// hacer typedefs todos los structs??? así no hay que escribir siempre struct (ventaja: se programa más rapido) //////////////
	#pragma region Here is the context with all the asociated structs and enums
	struct Context {
		struct Folder** folders;
		struct ParentalControl* parental;
		char** sync_folders;
		struct OpTable** tables;									// Cambiado a plural //////////////////////////////////////////////////
		struct App** apps;											// Cambiado a plural //////////////////////////////////////////////////
		struct ChallengeEquivalenceGroup** groups;					// Cambiado a plural //////////////////////////////////////////////////
	} ctx;

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
		char** challenge_group_ids;									// Añadido _ids /////// cambiar tambien en el json ///////////////
		char* cipher;
	};

	struct ParentalControl {
		char* folder;
		char** users;
		char** challenge_group_ids;									// Añadido _ids /////// cambiar tambien en el json ///////////////
	};

	struct OpTable {
		char* ID_table;												////////////////////// Cambiar solo a id
		struct TableTuple** table_tuples;							// Cambiado a plural /////////////////////////////////////////////
																	/////// Cambiar a tuples o rows (pero tambien el nombre del struct a OpTableRow)
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
		char* app_path;												////////////////////// Cambiar solo a path
		char* app_name;												////////////////////// Cambiar solo a name
		enum AppType app_type;										////////////////////// Cambiar solo a type
	};

	struct ChallengeEquivalenceGroup {
		char* id;
		char* subkey;
		char* expires;			// YYMMDDhhmmss
		struct Challenge** challenges;								// Cambiado a plural //////////////////////////////////////////////////
	};

	struct Challenge {
		char* id_challenge;											////////////////////// Cambiar solo a id
		char* properties;	// “prop1=valor&prop2=valor…”
	};
	#pragma endregion


	static void printContext() {
		PRINT("\n");
		PRINT("================================================================================\n");
		PRINT("PRINT CONTEXT BEGINS\n");

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
			PRINT2(" ______________________________________________________ \n");
			PRINT2("|       |            |        |           |            |\n");
			PRINT2("|  Row  |  App Type  |  Disk  |  On Read  |  On Write  |\n");
			PRINT2("|_______|____________|________|___________|____________|\n");
			PRINT2("|       |            |        |           |            |\n");

			// Iterate over rows of each table (the so called "table tuples")
			for (int j = 0; j < _msize(ctx.tables[i]->table_tuples) / sizeof(struct TableTuple*); j++) {
				PRINT2("|  %2d   |     %2d     |   %2s   |    %2d     |     %2d     |\n",
					j,
					ctx.tables[i]->table_tuples[j]->app_type,
					ctx.tables[i]->table_tuples[j]->disk,
					ctx.tables[i]->table_tuples[j]->on_read,
					ctx.tables[i]->table_tuples[j]->on_write
				);
			}
			PRINT2("|_______|____________|________|___________|____________|\n");
		}

		// Apps
		PRINT("\n");
		PRINT("Apps\n");
		for (int i = 0; i < _msize(ctx.apps) / sizeof(struct App*); i++) {
			PRINT1("App %d:\n", i);
			PRINT2("App name: %s \n", ctx.apps[i]->app_name);
			PRINT2("App path: %s \n", ctx.apps[i]->app_path);
			PRINT2("App type: %d \n", ctx.apps[i]->app_type);
		}

		// Challenge Equivalence Groups
		printChallengeGroups(ctx);

		PRINT("\n");
		PRINT("PRINT CONTEXT ENDS\n");
		PRINT("================================================================================\n");
	}

	static void printChallengeGroups() {
		PRINT("\n");
		PRINT("Challenge Equivalence Groups\n");
		for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
			PRINT1("EqGroup:\n");
			PRINT2("Id: %s \n", ctx.groups[i]->id);
			PRINT2("Subkey: %s \n", ctx.groups[i]->subkey);
			PRINT2("Expires: %s (", ctx.groups[i]->expires); printDateNice(ctx.groups[i]->expires); PRINT(")\n");
			PRINT2("Challenges: \n");
			for (int j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct ChallengeEquivalenceGroup*); j++) {
				PRINT3("Challenge:\n");
				PRINT4("Id: %s \n", ctx.groups[i]->challenges[j]->id_challenge);
				PRINT4("Properties: %s \n", ctx.groups[i]->challenges[j]->properties);
			}
		}
	}

	// The printed format is "YYYY-MM-DD - hh:mm:ss". Example: "2021-03-11 - 14:21:08"
	static void printDateNice(char* date) {
		if (strlen(date) != 14) {
			PRINT("ERROR");
		} else {
			PRINT("%c%c%c%c-%c%c-%c%c - %c%c:%c%c:%c%c",
				date[0], date[1], date[2], date[3],			// Year
				date[4], date[5],							// Month
				date[6], date[7],							// Day
				date[8], date[9],							// Hours
				date[10], date[11],							// Minutes
				date[12], date[13]							// Seconds
			);
		}
	}

	static void printChallengeGroup(char* id) {
		struct ChallengeEquivalenceGroup* group = getChallengeGroup(id);
		if (group == NULL) {
			PRINT("ERROR\n");
		} else {
			PRINT("EqGroup:\n");
			PRINT1("Id: %s \n", group->id);
			PRINT1("Subkey: %s \n", group->subkey);
			PRINT1("Expires: %s (", group->expires); printDateNice(group->expires); PRINT(")\n");
			PRINT1("Challenges: \n");
			for (int j = 0; j < _msize(group->challenges) / sizeof(struct ChallengeEquivalenceGroup*); j++) {
				PRINT2("Challenge:\n");
				PRINT3("Id: %s \n", group->challenges[j]->id_challenge);
				PRINT3("Properties: %s \n", group->challenges[j]->properties);
			}
		}
	}

	static struct ChallengeEquivalenceGroup* getChallengeGroup(char* id) {
		for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
			if (strcmp(ctx.groups[i]->id, id) == 0) {
				return ctx.groups[i];
			}
		}
		return NULL;
	}


	#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//context_h