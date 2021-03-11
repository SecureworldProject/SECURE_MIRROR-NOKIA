#include "context.h"

enum Operation getTableOperation(int irp_operation, char *app_path, char *app_file);
struct OpTable* getTable(char* file_path);
// TO DO
//getAppType(app_path)
//getDiskType(file_path)
//getOperations(disk, apptype, table_ptr)
//struct App initApp(&app)
//struct App destroyApp(&app)



enum Operation getTableOperation(int irp_operation, char* app_path, char* app_file) {
	enum Operation result_operation = NOTHING;

	// TO DO

	return result_operation;
}

struct OpTable* getTable(char* file_path) {
	struct OpTable* op_table = NULL;

	// TO DO

	return op_table;
}
