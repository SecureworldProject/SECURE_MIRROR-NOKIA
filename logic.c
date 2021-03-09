/*
* SecureWorld file logic.c
contiene la lógica común logic(ctx, operación) a cualquier operación.
SOLO una lógica que implementa la operativa cargada en loadContext() para todas las operaciones. 
invoca a parentControl(), keymaker(), cypher() y decypher().

Nokia Febrero 2021
*/

#include "dokan.h"
#include "context.h"
#include "winnt.h"
#include <psapi.h>



void preLogic(int num, struct App app);
void postLogic(int num, struct App app);
void getApp(struct App* app, PDOKAN_FILE_INFO dokan_file_info, struct Context ctx);



void preLogic(int num, struct App app) {
	PRINT("HELLO!!  %d \n", num);
}

void postLogic(int num, struct App app) {
	PRINT("BYE!!  %d \n", num);
}


void getApp(struct App * app, PDOKAN_FILE_INFO dokan_file_info, struct Context ctx) {
	PRINT("hemos llegado aqui---------------\n");
	HANDLE process_handle;
	CHAR process_full_path[MAX_PATH] = { 0 };
	char* tmp_str;
	size_t len;

	BOOL match_found = FALSE;

	process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dokan_file_info->ProcessId);

	if (GetProcessImageFileNameA(process_handle, process_full_path, sizeof(process_full_path) / sizeof(*process_full_path)) > 0) {
		// Clear possible backward slashes into forward slashes
		tmp_str = strchr(process_full_path, '\\');
		while (tmp_str != NULL) {
			*tmp_str = '/';
			tmp_str = strchr(process_full_path, '\\');
		}

		// Find last position of a forward slash, which divides string in "path_to_folder" and "name" (e.g.:  C:/path/to/folder/name.exe)
		tmp_str = strrchr(process_full_path, '/');

		// Fill app path and name
		*tmp_str = '\0';
		len = strlen(process_full_path);
		strcpy(app->app_path, process_full_path);
		app->app_path[len] = '/';
		app->app_path[len+1] = '\0';
		strcpy(app->app_name, tmp_str + 1);

		// For every app in the list check if the path is the same
		for (int i = 0; !match_found && i < _msize(ctx.app) / sizeof(struct App*); i++) {
			if (strcmp(ctx.app[i]->app_path, app->app_path) == 0) {
				match_found = TRUE;
				app->app_type = ctx.app[i]->app_type;
				break;
			}
		}

		// For every app in the list check if the name is the same
		for (int i = 0; !match_found && i < _msize(ctx.app) / sizeof(struct App*); i++) {
			if (strcmp(ctx.app[i]->app_name, app->app_name) == 0) {
				match_found = TRUE;
				app->app_type = ctx.app[i]->app_type;
				break;
			}
		}
	}

	return app;
}
