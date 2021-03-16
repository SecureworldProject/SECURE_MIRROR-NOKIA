/*
* SecureWorld file context.h
contiene la estructura en C en la que se carga el contexto escrito en el fichero config.json, y que contiene la configuración de todo el sistema.
Los distintos proyectos (challenges, mirror, app descifradora etc) deben incluir este fichero para compilar pues sus DLL
se invocarán con un parámetro de tipo contexto que se definirá en context.h

Nokia Febrero 2021
*/

#ifndef context_h
#define context_h

#include <time.h>


#ifdef __cplusplus
extern "C" {
	#endif	//__cplusplus

	#ifndef NULL
		#define NULL (void*)0
	#endif //NULL

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
	static char* getTimeFormattedString(time_t time);
	static time_t getTimeFromString(char* formatted_time_str);
	//static int getWeekDay(int y, int m, int d);
	//static int getYearDay(int year, int month, int day);
	static struct ChallengeEquivalenceGroup* getChallengeGroupById(char* group_id);
	static struct OpTable* getOpTableById(char* table_id);


	///////////////////////////// hacer typedefs todos los structs??? así no hay que escribir siempre struct (ventaja: se programa más rapido) //////////////
	#pragma region Here is the context with all the asociated structs and enums
	struct Context {
		struct Folder** folders;
		struct ParentalControl* parental;
		char** sync_folders;
		struct OpTable** tables;
		struct App** apps;
		struct ChallengeEquivalenceGroup** groups;
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
		struct OpTable* op_table;
		struct ChallengeEquivalenceGroup** challenge_group_ids;		//  Quitar ids//////////////////////
		char* cipher;
	};

	struct ParentalControl {
		char* folder;
		char** users;
		struct ChallengeEquivalenceGroup** challenge_group_ids;		// Quitar _ids/////////////////////////
	};

	struct OpTable {
		char* id;
		struct Tuple** tuples;
	};

	struct Tuple {
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
		char* path;
		char* name;
		enum AppType type;
	};

	struct ChallengeEquivalenceGroup {
		char* id;
		char* subkey;
		char* expires;			// YYMMDDhhmmss						// Cambiar a tipo time_t de libreria c <time.h>
		struct Challenge** challenges;
	};

	struct Challenge {
		char* id;
		char* properties;		// "prop1=valor&prop2=valor..."
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
				PRINT("%s%s", (char*) ctx.folders[i]->protection->challenge_group_ids[j], (j + 1 < _msize(ctx.folders[i]->protection->challenge_group_ids)/sizeof(char*)) ? ", " : "\n");
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
			PRINT1("Table id: %s \n", ctx.tables[i]->id);
			PRINT2(" ______________________________________________________ \n");
			PRINT2("|       |            |        |           |            |\n");
			PRINT2("|  Row  |  App Type  |  Disk  |  On Read  |  On Write  |\n");
			PRINT2("|_______|____________|________|___________|____________|\n");
			PRINT2("|       |            |        |           |            |\n");

			// Iterate over rows of each table (the so called "table tuples")
			for (int j = 0; j < _msize(ctx.tables[i]->tuples) / sizeof(struct Tuple*); j++) {
				PRINT2("|  %2d   |     %2d     |   %2s   |    %2d     |     %2d     |\n",
					j,
					ctx.tables[i]->tuples[j]->app_type,
					ctx.tables[i]->tuples[j]->disk,
					ctx.tables[i]->tuples[j]->on_read,
					ctx.tables[i]->tuples[j]->on_write
				);
			}
			PRINT2("|_______|____________|________|___________|____________|\n");
		}

		// Apps
		PRINT("\n");
		PRINT("Apps\n");
		for (int i = 0; i < _msize(ctx.apps) / sizeof(struct App*); i++) {
			PRINT1("App %d:\n", i);
			PRINT2("App name: %s \n", ctx.apps[i]->name);
			PRINT2("App path: %s \n", ctx.apps[i]->path);
			PRINT2("App type: %d \n", ctx.apps[i]->type);
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
				PRINT4("Id: %s \n", ctx.groups[i]->challenges[j]->id);
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
		struct ChallengeEquivalenceGroup* group = getChallengeGroupById(id);
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
				PRINT3("Id: %s \n", group->challenges[j]->id);
				PRINT3("Properties: %s \n", group->challenges[j]->properties);
			}
		}
	}



	/**
	* Transforms a time_t in the correct format string for the context ("YYYYMMDDhhmmss").
	* Allocates memory inside. Remember to free the returned char*.
	* 
	* @param time_t timer
	*		The time_t value to convert into formatted string
	* 
	* @return char*
	*		The time formatted as string ("YYYYMMDDhhmmss"). Memory allocated inside, remember to free.
	**/
	static char* getTimeFormattedString(time_t timer) {

		char* formatted_time_str = (char*)malloc(sizeof(char) * 14);
		if (formatted_time_str == NULL) {
			return NULL;
		}
		struct tm time_info;

		PRINT("Value of timer is: %lld \n", timer);
		PRINT("That is around %lld years\n", timer /(60*60*24*365));

		if (localtime_s(&time_info, &timer) == 0) {
			sprintf(formatted_time_str, "%04d%02d%02d%02d%02d%02d",
				time_info.tm_year + 1900,		// .tm_year are years since 1900
				time_info.tm_mon + 1,			// .tm_mon is in range [0-11]
				time_info.tm_mday,				// .tm_mday is in range[1-31]
				time_info.tm_hour,				// .tm_hour is in range[0-23]
				time_info.tm_min,				// .tm_min is in range[0-59]
				time_info.tm_sec				// .tm_sec is in range[0-60]. Can include leap seccond
			);
		} else {
			free(formatted_time_str);
			formatted_time_str = NULL;
		}

		return formatted_time_str;
	}

	/**
	* Transforms a time formatted string into a time_t value.
	*
	* @param char* formatted_time_str
	*		The time formatted as string ("YYYYMMDDhhmmss").
	*
	* @return time_t
	*		The time_t value converted from the formatted string.
	**/
	static time_t getTimeFromString(char* formatted_time_str) {

		time_t timer = 0;
		struct tm time_info = {0};
		int num;
		size_t len = strlen(formatted_time_str);

		PRINT("String is: '%s' with length of %llu\n", formatted_time_str, len);

		// Check string length
		if (len != 14) {
			return timer;
		}

		// Check characters are digits
		for (size_t i = 0; i < len; i++) {
			PRINT("formatted_time_str[i] = %c\n", formatted_time_str[i]);
			if (formatted_time_str[i] < '0' || formatted_time_str[i]>'9') {
				return timer;
			}
		}

		// Fill time_info with current time to get the DST value (all other fields will be overriden)
		time_t curr_time = time(NULL);
		localtime_s(&time_info, &curr_time);

		// PARSE STRING
		// Get Year
		num = (formatted_time_str[0] - '0') * 1000 + (formatted_time_str[1] - '0') * 100 + (formatted_time_str[2] - '0') * 10 + (formatted_time_str[3] - '0');
		num -= 1900;
		if (num < 0) {
			return timer;
		}
		time_info.tm_year = num;

		// Get Month
		num = (formatted_time_str[4] - '0') * 10 + (formatted_time_str[5] - '0');
		num -= 1;
		if (num < 0 || num > 11) {
			return timer;
		}
		time_info.tm_mon = num;

		// Get Day
		num = (formatted_time_str[6] - '0') * 10 + (formatted_time_str[7] - '0');
		if (num < 1 || num > 31) {
			return timer;
		}
		time_info.tm_mday = num;

		// Get Hours
		num = (formatted_time_str[8] - '0') * 10 + (formatted_time_str[9] - '0');
		if (num < 0 || num > 23) {
			return timer;
		}
		time_info.tm_hour = num;

		// Get Minutes
		num = (formatted_time_str[10] - '0') * 10 + (formatted_time_str[11] - '0');
		if (num < 0 || num > 59) {
			return timer;
		}
		time_info.tm_min = num;

		// Get Seconds
		num = (formatted_time_str[12] - '0') * 10 + (formatted_time_str[13] - '0');
		if (num < 0 || num > 60) {
			return timer;
		}
		time_info.tm_sec = num;

		// Gets the result and fills the WeekDay and YearDay
		timer = mktime(&time_info);

		// Get WeekDay
		//time_info.tm_wday = getWeekDay(time_info.tm_year, time_info.tm_mon, time_info.tm_mday);
		PRINT("WeekDay: %d\n", time_info.tm_wday);

		// Get YearDay
		//time_info.tm_yday = getYearDay(time_info.tm_year, time_info.tm_mon, time_info.tm_mday);
		PRINT("YearDay: %d\n", time_info.tm_yday);

		return timer;
	}

	// Formula "modifies" parameters (reason not to inline). Sunday=0, Monday=1, ... Saturday = 6
	/*static int getWeekDay(int y, int m, int d) {
		// https://stackoverflow.com/questions/6054016/c-program-to-find-day-of-week-given-date
		return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
	}*/

	// Formula "modifies" parameters (reason not to inline). Result is in range [0-365]
	/*static int getYearDay(int year, int month, int day) {
		// https://www.geeksforgeeks.org/find-the-day-number-in-the-current-year-for-the-given-date/
		int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		if (month > 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
			++day;
		}

		while (month-- > 0) {
			day = day + days[month - 1];
		}

		return day;
	}*/

	static struct ChallengeEquivalenceGroup* getChallengeGroupById(char* group_id) {
		for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
			if (strcmp(ctx.groups[i]->id, group_id) == 0) {
				return ctx.groups[i];
			}
		}
		return NULL;
	}

	static struct OpTable* getOpTableById(char* table_id) {
		for (int i = 0; i < _msize(ctx.tables) / sizeof(struct OpTable*); i++) {
			if (strcmp(ctx.tables[i]->id, table_id) == 0) {
				return ctx.tables[i];
			}
		}
		return NULL;
	}


	#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//context_h