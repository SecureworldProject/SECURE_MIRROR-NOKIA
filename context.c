
#ifndef context_c
#define context_c

#include "context.h"
#include "string.h"
#include <ctype.h>
#include <string.h>


enum IrpOperation {
	ON_READ,
	ON_WRITE
};
#define NUM_IRP_OPERATIONS 2


#ifndef FALSE
#define FALSE 0
#endif //FALSE

#ifndef TRUE
#define TRUE 1
#endif //TRUE

#ifndef BOOL
#define BOOL int
#endif //BOOL


// FOR FUTURE USE - performance improvement: create a context for each volume (each mirror thread) and a context for each file handle (in PDOKAN_FILE_INFO.Context)
/*
struct VolumeContext {
	union {
		struct Folder* folder;		// Pointer to the struct folder of this volume
		struct Pendrive* pendrive;	// Pointer to the struct folder of this volume
	};
	char** sync_folders;			// Array of synchronization folders that affect this volume (after conversion from "C:\..." to "M:\...", "N:\...", etc)
	BOOL any_sync_folder;
	BOOL is_pendrive;				// If this volume is a pendrive (detected from USB and mounted as such)
};

struct FileContext {
	struct App* app;
	BOOL is_sync_folder;
};
*/




/////  FUNCTION PROTOTYPES  /////


static struct App* createApp();

static void destroyApp(struct App** app);


static enum Operation getTableOperation(enum IrpOperation irp_operation, char* app_full_path, char* file_full_path);


inline struct OpTable* getTable(char* file_full_path);

inline struct App* getApp(char* app_full_path);

inline char getDiskType(char* file_full_path);

inline enum Operation* getOperations(char disk_letter, enum AppType app_type, struct OpTable* table);

static void formatPath(char** full_path);


/////  FUNCTION DEFINITIONS  /////

/**
* Returns a newly allocated and initialized (path="", name="" and type=ANY) pointer to an app.
* If it is not possible to allocate memory, frees internal buffers and returns NULL.
*
* @return struct App* app
*		The initialized app pointer. May be NULL.
**/
static struct App* createApp() {
	struct App* app =  malloc(sizeof(struct App));
	if (app) {

		app->name = malloc(MAX_PATH * sizeof(char));
		if (app->name)
			memset(app->name, '\0', MAX_PATH * sizeof(char));
		else
			goto _LABEL_FREE;

		app->path = malloc(MAX_PATH * sizeof(char));
		if (app->path)
			memset(app->path, '\0', MAX_PATH * sizeof(char));
		else
			goto _LABEL_FREE;

		app->type = ANY;
	} else
		goto _LABEL_FREE;

	return app;

	_LABEL_FREE:
	if (app) {
		if (app->path) free(app->path);
		if (app->name) free(app->name);
		free(app);
	}
	return NULL;
}

/**
* Frees all the memory from the app pointer passed as parameter and all its fields.
*
* @param struct App** app
*		The pointer to the app pointer to be destroyed. The pointer to App is always nullified.
**/
static void destroyApp(struct App** app) {
	struct App* a = *app;

	if (a) {
		if (a->path) free(a->path);
		if (a->name) free(a->name);
		free(a);
	}
	a = NULL;
}


/**
* Gets the operation that has to be done when reading/writing by checking the corresponding OpTable with the app and filepath given.
*
* @param enum IrpOperation irp_operation
*		Determines if the operation needed is for a read or write.
* @param char* app_full_path
*		The full path (name and extension included) to the executable of the irp originating process.
* @param char* file_full_path
*		The full path (name and extension included) of the file which irp operation is about.
*
* @return enum Operation
*		The logic that must be done for the given irp operation, appfile
**/
static enum Operation getTableOperation(enum IrpOperation irp_operation, char* app_full_path, char* file_full_path) {
	enum Operation *result_operation = NULL;
	struct OpTable* table = NULL;
	enum AppType app_type = ANY;
	char disk_letter = '\0';

	formatPath(&app_full_path);
	formatPath(&file_full_path);

	// Gets the table to apply to the file path
	table = getTable(file_full_path);

	// Gets the app_type for the given app_full_path
	app_type = (getApp(app_full_path))->type;

	// Gets the disk for the given file path
	disk_letter = getDiskType(file_full_path);

	// Gets the operation for the disk and app_type in the table given. The irp_operation is an enum that is used as index
	result_operation = getOperations(disk_letter, app_type, table);
	if (result_operation) {
		return result_operation[irp_operation];
	}

	return NOTHING;
}


inline struct OpTable* getTable(char* file_full_path) {
	char letter = '\0';

	// For the moment assume the path is good and then letter is position 0 of the string.
	letter = file_full_path[0];
	letter = toupper(letter);

	// Return the table from the context folder which MountPoint matches the letter
	for (int i = 0; i < _msize(ctx.folders) / sizeof(char*); i++) {
		if (ctx.folders[i]->mount_point == letter) {
			return ctx.folders[i]->protection->op_table;
		}
	}

	// Check if the letter is a pendrive
	for (int i = 0; i < strlen(ctx.pendrive->mount_points); i++) {
		if (ctx.pendrive->mount_points[i] == letter) {
			return ctx.pendrive->protection->op_table;
		}
	}

	// If not found, return NULL (this should never happen)
	return NULL;
}

inline struct App* getApp(char* app_full_path) {
	enum AppType app_type = ANY;
	struct App* app = NULL;
	char* tmp_str = NULL;
	size_t len = 0;
	BOOL match_found = FALSE;

	// Initialize an app
	app = createApp();

	// Find last position of a forward slash, which divides string in "path" and "name" (e.g.:  C:/path/to/folder/name.exe)
	tmp_str = strrchr(app_full_path, '/');

	// Fill app path and name
	*tmp_str = '\0';
	len = strlen(app_full_path);
	strcpy(app->path, app_full_path);
	app->path[len] = '/';
	app->path[len + 1] = '\0';
	strcpy(app->name, tmp_str + 1);

	// For every app in the list check if the path is the same
	for (int i = 0; !match_found && i < _msize(ctx.apps) / sizeof(struct App*); i++) {
		if (strcmp(ctx.apps[i]->path, app->path) == 0) {
			match_found = TRUE;
			app->type = ctx.apps[i]->type;
			break;
		}
	}

	// For every app in the list check if the name is the same
	for (int i = 0; !match_found && i < _msize(ctx.apps) / sizeof(struct App*); i++) {
		if (strcmp(ctx.apps[i]->name, app->name) == 0) {
			match_found = TRUE;
			app->type = ctx.apps[i]->type;
			break;
		}
	}

	return app;
}

inline char getDiskType(char* file_full_path) {
	// It can be '0' (sync folders), '1' (pendrives) or any letter ('a', 'b', 'c', etc.)
	char* tmp_str = NULL;
	char letter = '\0';

	// Check if it is syncfolder
	for (int i = 0; i < _msize(ctx.sync_folders) / sizeof(char*); i++) {
		tmp_str = strstr(file_full_path, ctx.sync_folders[i]);
		if (tmp_str != NULL && tmp_str == file_full_path) {
			return '0';			// It matches a syncfolder
		}
	}

	// Check if it is a pendrive
	letter = file_full_path[0];
	letter = toupper(letter);		// This should already have been done in formatPath()

	// Check if the letter is a pendrive
	for (int i = 0; i < strlen(ctx.pendrive->mount_points); i++) {
		if (ctx.pendrive->mount_points[i] == letter) {
			return '1';
		}
	}

	// It is a letter (file is not in a sync folder nor a pendrive)
	return letter;
}

inline enum Operation* getOperations(char disk_letter, enum AppType app_type, struct OpTable* table) {
	enum Operation* operations = NULL;

	operations = malloc(NUM_IRP_OPERATIONS * sizeof(enum Operation));
	if (operations) {
		operations[ON_READ] = NOTHING;
		operations[ON_WRITE] = NOTHING;

		for (int i = 0; i < _msize(table->tuples) / sizeof(struct Tuple*); i++) {
			if (table->tuples[i]->app_type == app_type && table->tuples[i]->disk == disk_letter) {
				operations[ON_READ] = table->tuples[i]->on_read;
				operations[ON_WRITE] = table->tuples[i]->on_write;
			}
		}
	}

	return operations;
}

static void formatPath(char** full_path) {
	char* tmp_str = NULL;

	// WARNING!!!  read below
	// Paths may have many different writtings:
	//		- "X:\folder1\folder2\file.txt"
	//		- "\Device\Harddiskvolume1\folder1\folder2\file.txt"
	//		- "\Device\Harddisk0\Partition0\folder1\folder2\file.txt"
	//		- "\\?\Volume{a2b4c6d8-0000-0000-00000100000000000}\folder1\folder2\file.txt"
	// Appart from the different writtings there are additional problems:
	//		- May have forward or backward slashes.
	//		- May be preceded by "\\?\" or "\\.\".
	//		- May contain relative paths inside (like references to the same path "./" or to the parent directory "../").
	//		- If the path refers to a folder, it may or not contain a trailing slash.

	// Clear possible backward slashes into forward slashes
	tmp_str = strchr(*full_path, '\\');
	while (tmp_str != NULL) {
		*tmp_str = '/';
		tmp_str = strchr(*full_path, '\\');
	}

	// TO DO MORE FORMATTING
}


#endif context_c