#include "context.h"
#include "string.h"


enum IRP_OPERATION {
	ON_READ,
	ON_WRITE
};
#define NUM_IRP_OPERATIONS 2



/////  FUNCTION PROTOTYPES  /////


struct App* createApp();

void destroyApp(struct App** app);


enum Operation getTableOperation(enum IRP_OPERATION irp_operation, char *app_path, char * file_path);


struct OpTable* getTable(char* file_path);

enum AppType getAppType(char* app_path);

char getDiskType(char* file_path);

enum Operation* getOperations(char disk_letter, enum AppType app_type, struct OpTable* table);




/////  FUNCTION DEFINITIONS  /////

/**
* Returns a newly allocated and initialized (path="", name="" and type=ANY) pointer to an app.
*
* @return struct App* app
*		The initialized app pointer.
**/
struct App* createApp() {
	struct App* app =  malloc(sizeof(struct App));

	app->name = malloc(MAX_PATH * sizeof(char));
	memset(app->name, '\0', MAX_PATH * sizeof(char));
	app->name = malloc(MAX_PATH * sizeof(char));
	memset(app->path, '\0', MAX_PATH * sizeof(char));
	app->type = ANY;

	return app;
}

/**
* Frees all the memory from the app pointer passed as parameter and all its fields.
*
* @param struct App** app
*		The pointer to the app pointer to be destroyed.
**/
void destroyApp(struct App** app) {
	free((*app)->name);
	(*app)->name = NULL;
	free((*app)->path);
	(*app)->path = NULL;
	free(*app);
	(*app) = NULL;
}


enum Operation getTableOperation(enum IRP_OPERATION irp_operation, char* app_path, char* file_path) {
	enum Operation result_operation = NOTHING;
	struct OpTable *table = NULL;
	enum AppType app_type = ANY;
	char disk_letter = '\0';

	// Gets the table to apply to the file path
	table = getTable(file_path);

	// Gets the app_type for the given app_path
	app_type = getAppType(app_path);

	// Gets the disk for the given file path
	disk_letter = getDiskType(file_path);

	// Gets the operation for the disk and app_type in the table given. The irp_operation is an enum that is used as index
	result_operation = (getOperations(disk_letter, app_type, table))[irp_operation];

	return result_operation;
}


struct OpTable* getTable(char* file_path) {
	struct OpTable* op_table = NULL;

	// TO DO

	return op_table;
}

enum AppType getAppType(char* app_path) {
	enum AppType app_type = ANY;

	// TO DO

	return app_type;
}

char getDiskType(char* file_path) {
	// It can be '0' (sync folders), '1' (pendrives) or any letter ('a', 'b', 'c', etc.)
	char disk = '\0';	// none

	// TO DO

	return disk;
}

enum Operation* getOperations(char disk_letter, enum AppType app_type, struct OpTable* table) {
	enum Operation* operations = NULL;

	operations = malloc(NUM_IRP_OPERATIONS * sizeof(enum Operation));
	operations[ON_READ] = NOTHING;
	operations[ON_WRITE] = NOTHING;

	// TO DO

	return operations;
}


