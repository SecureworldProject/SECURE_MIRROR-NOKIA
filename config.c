/*
* SecureWorld file config.c
* carga la configuración en el contexto usando un parser. contiene la función loadContext() y checkContext()
y otras posibles funciones auxiliares que necesite. El contexto incluye la carga operativa que luego condiciona
el comportamiento de la función logic().

Nokia Febrero 2021
*/

#include <sys/stat.h>
#include "json.h"
#include "json.c"
#include "config.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
//#include <windows.h>

#include <stdlib.h>
#include "context.h"
#include <string.h>
//#include <ctype.h>    // already in context.c


static void print_depth_shift(int depth) {
    int j;
    for (j = 0; j < depth; j++) {
        printf(" ");
    }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth) {
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.object.length;

    for (x = 0; x < length; x++) {
        print_depth_shift(depth);
        //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        process_value(value->u.object.values[x].value, depth + 1);
    }
}

static void process_array(json_value* value, int depth) {
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.array.length;
    //printf("Longitud del array %d\n", length);
    for (x = 0; x < length; x++) {
        process_value(value->u.array.values[x], depth);
    }
}

static void process_value(json_value* value, int depth) {
    if (value == NULL) {
        return;
    }

    if (value->type != json_object) {
        print_depth_shift(depth);
    }

    switch (value->type) {
    case json_none:
        printf("none\n");
        break;
    case json_object:
        process_object(value, depth + 1);
        break;
    case json_array:
        process_array(value, depth + 1);
        break;
    case json_integer:
        printf("int: %10" PRId64 "\n", value->u.integer);
        break;
    case json_double:
        printf("double: %f\n", value->u.dbl);
        break;
    case json_string:
        printf("string: %s\n", value->u.string.ptr);
        break;
    case json_boolean:
        printf("bool: %d\n", value->u.boolean);
        break;
    }
}

static void processProtection(struct Protection* ctx_value, json_value* value, int depth) {
    int dict_length, x, num_groups;

    dict_length = value->u.object.length;
    for (x = 0; x < dict_length; x++) {

        if (strcmp(value->u.object.values[x].name, "OpTable") == 0) {
            // This will be a OpTable pointer but for now it will hold the id, so force it as char pointer
            ctx_value->op_table = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx_value->op_table) {
                strcpy(ctx_value->op_table, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        if (strcmp(value->u.object.values[x].name, "ChallengeEqGroups") == 0) {
            //Este objeto es un array
            num_groups = value->u.object.values[x].value->u.array.length;
            if (num_groups <= 0) {          // Fixes warning C6386 (Visual Studio bug)
                ctx_value->challenge_groups = NULL;
            } else {
                ctx_value->challenge_groups = (char**) malloc(num_groups * sizeof(char*));   // Allocate space for all pointers to char
                if (ctx_value->challenge_groups) {
                    for (int i = 0; i < num_groups; i++) {
                        ctx_value->challenge_groups[i] = (char*) malloc(sizeof(char) * ((value->u.object.values[x].value->u.array.values[i]->u.string.length) + 1));
                        if (ctx_value->challenge_groups[i]) {
                            strcpy(ctx_value->challenge_groups[i], value->u.object.values[x].value->u.array.values[i]->u.string.ptr);
                        } // else --> The pointer is null because it was not possible to allocate memory
                    }
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        }

        if (strcmp(value->u.object.values[x].name, "Cipher") == 0) {
            ctx_value->cipher = malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx_value->cipher) {
                strcpy(ctx_value->cipher, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }
    }


}

static void processFolder(int index, json_value* value, int depth) {
    int dict_length, x;
    enum Driver driver;
    char* driver_str;
    json_value* protection_value;

    dict_length = value->u.object.length;
    for (x = 0; x < dict_length; x++) {

        if (strcmp(value->u.object.values[x].name, "Path") == 0) {
            ctx.folders[index]->path = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length)+1));
            if (ctx.folders[index]->path) {
                strcpy(ctx.folders[index]->path, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        else if (strcmp(value->u.object.values[x].name, "MountPoint") == 0) {
            /*ctx.folders[index]->mount_point = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx.folders[index]->mount_point) {
                strcpy(ctx.folders[index]->mount_point, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory*/
            if (value->u.object.values[x].value->u.string.length >= 1) {
                ctx.folders[index]->mount_point = toupper(value->u.object.values[x].value->u.string.ptr[0]);
            } else {
                ctx.folders[index]->mount_point = '\0'; // ERROR
                printf("WARNING: incorrect MountPoint.\n");
            }
        }

        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            //printf("He pillado el driver: %s\n", value->u.object.values[x].value->u.string.ptr);
            driver_str = value->u.object.values[x].value->u.string.ptr;
            if (strcmp(driver_str, "WinFSP") == 0)          driver = WINFSP;
            else if (strcmp(driver_str, "DOKAN") == 0)      driver = DOKAN;
            else                                            driver = DOKAN;     // This is the default driver: Dokan
            ctx.folders[index]->driver = driver;
        }

        else if (strcmp(value->u.object.values[x].name, "Protection") == 0) {
            ctx.folders[index]->protection = (struct Protection*)malloc(sizeof(struct Protection));
            if (ctx.folders[index]->protection) {
                protection_value = value->u.object.values[x].value;
                processProtection(ctx.folders[index]->protection, protection_value, depth + 1);
            } // else --> The pointer is null because it was not possible to allocate memory
        }
    }
}

static void processFolders(json_value* value, int depth) {
    int array_length;
    int array_pos = 0;
    int in_folder_length = 0;
    int in_folder_pos = 0;

    array_length = value->u.array.length;
    if (array_length <= 0) {        // Fixes warning C6386 (Visual Studio bug)
        ctx.folders = NULL;
    } else {
        ctx.folders = (struct Folder**)malloc(array_length * sizeof(struct Folder*));
        if (ctx.folders) {
            for (int i = 0; i < array_length; i++) {
                ctx.folders[i] = (struct Folder*)malloc(sizeof(struct Folder));
                if (ctx.folders[i]) {
                    processFolder(i, value->u.array.values[i], depth);
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        } // else --> The pointer is null because it was not possible to allocate memory
    }
}

static void processPendrive(json_value* value, int depth) {
    int dict_length, x, i;
    enum Driver driver;
    char* driver_str;
    json_value* protection_value;

    dict_length = value->u.object.length;
    for (x = 0; x < dict_length; x++) {

        if (strcmp(value->u.object.values[x].name, "MountPoints") == 0) {
            ctx.pendrive->mount_points = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx.pendrive->mount_points) {
                //strcpy(ctx.pendrive->mount_points, value->u.object.values[x].value->u.string.ptr);
                for (int i = 0; i < strlen(value->u.object.values[x].value->u.string.length); i++) 		{
                    ctx.pendrive->mount_points[i] = (char)toupper(value->u.object.values[x].value->u.string.ptr[i]);
                }
                ctx.pendrive->mount_points[value->u.object.values[x].value->u.string.length] = '\0';
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            driver_str = value->u.object.values[x].value->u.string.ptr;
            if (strcmp(driver_str, "WinFSP") == 0)          driver = WINFSP;
            else if (strcmp(driver_str, "DOKAN") == 0)      driver = DOKAN;
            else                                            driver = DOKAN;     // This is the default driver: Dokan
            ctx.pendrive->driver = driver;
        }

        else if (strcmp(value->u.object.values[x].name, "Protections") == 0) {
            ctx.pendrive->protection = (struct Protection*)malloc(sizeof(struct Protection));
            if (ctx.pendrive->protection) {
                protection_value = value->u.object.values[x].value;
                processProtection(ctx.pendrive->protection, protection_value, depth + 1);
            } // else --> The pointer is null because it was not possible to allocate memory
        }
    }
}

static void processParentalControl(json_value* value, int depth) {
    int array_length, dict_length, users_array_length;
    json_value* array_value;

    array_length = value->u.array.length;
    ctx.parental = malloc(array_length * sizeof(struct ParentalControl));
    if (ctx.parental) {
        for (int i = 0; i < array_length; i++) {
            array_value = value->u.array.values[i];
            dict_length = array_value->u.object.length;
            for (int j = 0; j < dict_length; j++) {
                if (strcmp(array_value->u.object.values[j].name, "Folder") == 0) {
                    ctx.parental[i].folder = (char*)malloc(sizeof(char) * ((array_value->u.object.values[j].value->u.string.length) + 1));
                    if (ctx.parental[i].folder) {
                        strcpy(ctx.parental[i].folder, array_value->u.object.values[j].value->u.string.ptr);
                    } // else --> The pointer is null because it was not possible to allocate memory
                }

                else if (strcmp(array_value->u.object.values[j].name, "Users") == 0) {
                    users_array_length = array_value->u.object.values[j].value->u.array.length;
                    if (users_array_length <= 0) {      // Fixes warning C6386 (Visual Studio bug)
                        ctx.parental[i].users = NULL;
                    } else {
                        ctx.parental[i].users = (char**)malloc(users_array_length * sizeof(char*));
                        if (ctx.parental[i].users) {
                            for (int k = 0; k < users_array_length; k++) {
                                //ctx.parental[i].users[k] = array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr;
                                ctx.parental[i].users[k] = (char*)malloc(sizeof(char) * ((array_value->u.object.values[j].value->u.array.values[k]->u.string.length) + 1));
                                if (ctx.parental[i].users[k]) {
                                    strcpy(ctx.parental[i].users[k], array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr);
                                } // else --> The pointer is null because it was not possible to allocate memory
                            }
                        } // else --> The pointer is null because it was not possible to allocate memory
                    }
                }

                else if (strcmp(array_value->u.object.values[j].name, "ChallengeEqGroups") == 0) {
                    users_array_length = array_value->u.object.values[j].value->u.array.length;
                    if (users_array_length <= 0) {      // Fixes warning C6386 (Visual Studio bug)
                        ctx.parental[i].users = NULL;
                    } else {
                        // This will be a ChallengeEquivalenceGroup pointer pointer but for now it will hold ids, so force it to char pointer pointer
                        ctx.parental[i].challenge_groups = (char**)malloc(users_array_length * sizeof(char*));
                        if (ctx.parental[i].challenge_groups) {
                            for (int k = 0; k < users_array_length; k++) {
                                // This will be a ChallengeEquivalenceGroup pointer but for now it will hold is, so force it to char pointer
                                ctx.parental[i].challenge_groups[k] = (char*)malloc(sizeof(char) * ((array_value->u.object.values[j].value->u.array.values[k]->u.string.length) + 1));
                                if (ctx.parental[i].challenge_groups[k]) {
                                    strcpy(ctx.parental[i].challenge_groups[k], array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr);
                                } // else --> The pointer is null because it was not possible to allocate memory
                            }
                        } // else --> The pointer is null because it was not possible to allocate memory
                    }
                }
            }
        }
    }
}

static void processSyncFolders(json_value* value, int depth) {
    int array_length;
    array_length = value->u.array.length;
    if (array_length <= 0) {      // Fixes warning C6386 (Visual Studio bug)
        ctx.sync_folders = NULL;
    } else {
        ctx.sync_folders = (char**)malloc(array_length * sizeof(char*));
        if (ctx.sync_folders) 	{
            for (int i = 0; i < array_length; i++) {
                ctx.sync_folders[i] = (char*)malloc(sizeof(char) * ((value->u.array.values[i]->u.string.length) + 1));
                if (ctx.sync_folders[i]) {
                    strcpy(ctx.sync_folders[i], value->u.array.values[i]->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        } // else --> The pointer is null because it was not possible to allocate memory
    }
}

static void processTableTuple(int table_index, int row_index, json_value* value, int depth) {
    int num_elems, x;
    enum AppType app_type = ANY;
    char* app_type_str;
    char* op_str;
    enum Operation op = NOTHING;

    num_elems = value->u.object.length;

    ctx.tables[table_index]->tuples[row_index] = (struct Tuple*)malloc(sizeof(struct Tuple));
    if (ctx.tables[table_index]->tuples[row_index]) {
        for (x = 0; x < num_elems; x++) {
            if (strcmp(value->u.object.values[x].name, "AppType") == 0) {
                app_type_str = value->u.object.values[x].value->u.string.ptr;
                if (strcmp(app_type_str, "BROWSER") == 0) app_type = BROWSER;
                else if (strcmp(app_type_str, "MAILER") == 0) app_type = MAILER;
                else if (strcmp(app_type_str, "BLOCKED") == 0) app_type = BLOCKED;
                else {
                    printf("WARNING: incorrect apptype, defaulting to ANY.\n");
                    app_type = ANY;         // If string does not match, ANY is used by default
                }
                ctx.tables[table_index]->tuples[row_index]->app_type = app_type;
            }

            if (strcmp(value->u.object.values[x].name, "Disk") == 0) {
                if (value->u.object.values[x].value->u.string.length >= 1) {
                    ctx.tables[table_index]->tuples[row_index]->disk = toupper(value->u.object.values[x].value->u.string.ptr[0]);
                } else {
                    ctx.tables[table_index]->tuples[row_index]->disk = '\0';
                    printf("WARNING: incorrect Disk.\n");
                }
            }

            if (strcmp(value->u.object.values[x].name, "READ") == 0) {
                op_str = value->u.object.values[x].value->u.string.ptr;
                if (strcmp(op_str, "NOTHING") == 0)         op = NOTHING;
                else if (strcmp(op_str, "CIPHER") == 0)     op = CIPHER;
                else if (strcmp(op_str, "DECIPHER") == 0)   op = DECIPHER;
                else if (strcmp(op_str, "MARK") == 0)       op = MARK;
                else if (strcmp(op_str, "UNMARK") == 0)     op = UNMARK;
                else if (strcmp(op_str, "IF_MARK_UNMARK_ELSE_CIPHER") == 0)             op = IF_MARK_UNMARK_ELSE_CIPHER;
                else if (strcmp(op_str, "IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING") == 0)  op = IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING;
                else {
                    printf("WARNING: incorrect read operation, defaulting to NOTHING.\n");
                    op = NOTHING;          // If string does not match, NOTHING is used by default
                }
                ctx.tables[table_index]->tuples[row_index]->on_read = op;
            }

            if (strcmp(value->u.object.values[x].name, "WRITE") == 0) {
                op_str = value->u.object.values[x].value->u.string.ptr;
                if (strcmp(op_str, "NOTHING") == 0)         op = NOTHING;
                else if (strcmp(op_str, "CIPHER") == 0)     op = CIPHER;
                else if (strcmp(op_str, "DECIPHER") == 0)   op = DECIPHER;
                else if (strcmp(op_str, "MARK") == 0)       op = MARK;
                else if (strcmp(op_str, "UNMARK") == 0)     op = UNMARK;
                else if (strcmp(op_str, "IF_MARK_UNMARK_ELSE_CIPHER") == 0)             op = IF_MARK_UNMARK_ELSE_CIPHER;
                else if (strcmp(op_str, "IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING") == 0)  op = IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING;
                else {
                    printf("WARNING: incorrect write operation, defaulting to NOTHING.\n");
                    op = NOTHING;          // If string does not match, NOTHING is used by default
                }
                ctx.tables[table_index]->tuples[row_index]->on_write = op;
            }
        }
    } // else --> The pointer is null because it was not possible to allocate memory
}

static void processOperativeTables(json_value* value, int depth) {
    int num_tables, i, j, num_rows;
    char* id_table;
    json_value* row_value;

    num_tables = value->u.object.length;
    if (num_tables <= 0) {      // Fixes warning C6386 (Visual Studio bug)
        ctx.tables = NULL;
    } else {
        ctx.tables = (struct OpTable**)malloc(num_tables * sizeof(struct OpTable*));
        if (ctx.tables) {
            for (i = 0; i < num_tables; i++) {
                id_table = value->u.object.values[i].name;
                ctx.tables[i] = (struct OpTable*)malloc(sizeof(struct OpTable));
                if (ctx.tables[i]) {
                    ctx.tables[i]->id = (char*)malloc(sizeof(char) * ((value->u.object.values[i].name_length) + 1));
                    if (ctx.tables[i]->id) {
                        strcpy(ctx.tables[i]->id, id_table);

                        // Each Tuple is a row of the Table (the number of tuples is the number of rows)
                        num_rows = value->u.object.values[i].value->u.array.length;
                        if (num_rows <= 0) {      // Fixes warning C6386 (Visual Studio bug)
                            ctx.tables[i]->tuples = NULL;
                        } else {
                            ctx.tables[i]->tuples = (struct Tuple**)malloc(num_rows * sizeof(struct Tuple*));
                            if (ctx.tables[i]->tuples) {
                                for (j = 0; j < num_rows; j++) {
                                    row_value = value->u.object.values[i].value->u.array.values[j];
                                    processTableTuple(i, j, row_value, depth + 1);
                                }
                            } // else --> The pointer is null because it was not possible to allocate memory
                        }
                    } // else --> The pointer is null because it was not possible to allocate memory
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        } // else --> The pointer is null because it was not possible to allocate memory
    }
}

static void processApp(int index, json_value* value, int depth) {
    int num_elem, i;
    char* app_type_str = "";
    enum AppType type = ANY;

    // Cada app es un diccionario, una tupla con nombre de tres elementos
    ctx.apps[index] = (struct App*)malloc(sizeof(struct App));
    if (ctx.apps[index]) {
        num_elem = value->u.object.length;
        for (i = 0; i < num_elem; i++) {
            if (strcmp(value->u.object.values[i].name, "AppPath") == 0) {
                ctx.apps[index]->path = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
                if (ctx.apps[index]->path) {
                    strcpy(ctx.apps[index]->path, value->u.object.values[i].value->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }

            else if (strcmp(value->u.object.values[i].name, "AppName") == 0) {
                ctx.apps[index]->name = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
                if (ctx.apps[index]->name) {
                    strcpy(ctx.apps[index]->name, value->u.object.values[i].value->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }

            else if (strcmp(value->u.object.values[i].name, "AppType") == 0) {
                app_type_str = value->u.object.values[i].value->u.string.ptr;
                if (strcmp(app_type_str, "BROWSER") == 0) type = BROWSER;
                else if (strcmp(app_type_str, "MAILER") == 0) type = MAILER;
                else if (strcmp(app_type_str, "BLOCKED") == 0) type = BLOCKED;
                else if (strcmp(app_type_str, "ANY") == 0) type = ANY;
                else {
                    printf("WARNING: incorrect apptype, defaulting to ANY.\n");
                    type = ANY;         // If string does not match, ANY is used by default
                }
                ctx.apps[index]->type = type;
            }
        }
    } // else --> The pointer is null because it was not possible to allocate memory
}

static void processApps(json_value* value, int depth) {
    //Lista de diccionarios
    int i, num_apps;
    json_value* app_value;
    num_apps = value->u.array.length;

    if (num_apps <= 0) {      // Fixes warning C6386 (Visual Studio bug)
        ctx.apps = NULL;
    } else {
        ctx.apps = (struct App**)malloc(num_apps * sizeof(struct App*));
        if (ctx.apps) {
            for (i = 0; i < num_apps; i++) {
                app_value = value->u.array.values[i];
                processApp(i, app_value, depth + 1);
            }
        } // else --> The pointer is null because it was not possible to allocate memory
    }
}

static void processChallenge(int group_index, int challenge_index, json_value* value, int depth) {
    int i, num_fields;

    num_fields = value->u.object.length;
    ctx.groups[group_index]->challenges[challenge_index] = malloc(sizeof(struct Challenge));
    if (ctx.groups[group_index]->challenges[challenge_index]) {
        for (i = 0; i < num_fields; i++) {
            // NOTE: Description and Requirements fields are merely informative. They are not passed to the context in any form.

            if (strcmp(value->u.object.values[i].name, "Id") == 0) {
                ctx.groups[group_index]->challenges[challenge_index]->id = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
                if (ctx.groups[group_index]->challenges[challenge_index]->id) {
                    strcpy(ctx.groups[group_index]->challenges[challenge_index]->id, value->u.object.values[i].value->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }

            else if (strcmp(value->u.object.values[i].name, "Props") == 0) {
                ctx.groups[group_index]->challenges[challenge_index]->properties = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
                if (ctx.groups[group_index]->challenges[challenge_index]->properties) {
                    strcpy(ctx.groups[group_index]->challenges[challenge_index]->properties, value->u.object.values[i].value->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        }
    }
}

static void processChallengeEqGroup(int index, json_value* value, int depth) {
    int i, j, num_elems;
    int num_challenges = 0;
    json_value* challenge_value;

    num_elems = value->u.object.length;
    for (i = 0; i < num_elems; i++) {

        if (strcmp(value->u.object.values[i].name, "SubKey") == 0) {
            ctx.groups[index]->subkey = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
            if (ctx.groups[index]->subkey) {
                strcpy(ctx.groups[index]->subkey, value->u.object.values[i].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        else if (strcmp(value->u.object.values[i].name, "Expires") == 0) {
            ctx.groups[index]->expires = (time_t)0;         // Subkey expired in 1970
        }

        else if (strcmp(value->u.object.values[i].name, "ChallengeList") == 0) {
            num_challenges = value->u.object.values[i].value->u.array.length;
            if (num_challenges <= 0) {      // Fixes warning C6386 (Visual Studio bug)
                ctx.groups[index]->challenges = NULL;
            } else {
                ctx.groups[index]->challenges = (struct Challenge**)malloc(num_challenges * sizeof(struct Challenge*));
                if (ctx.groups[index]->challenges) {
                    for (j = 0; j < num_challenges; j++) {
                        challenge_value = value->u.object.values[i].value->u.array.values[j];
                        processChallenge(index, j, challenge_value, depth + 1);
                    }
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        }
    }
}

static void processChallengeEqGroups(json_value* value, int depth) {
    int i, num_groups;
    json_value* group_value;

    num_groups = value->u.object.length;
    if (num_groups <= 0) {      // Fixes warning C6386 (Visual Studio bug)
        ctx.groups = NULL;
    } else {
        ctx.groups = (struct ChallengeEquivalenceGroup**)malloc(num_groups * sizeof(struct ChallengeEquivalenceGroup*));
        if (ctx.groups) {
            for (i = 0; i < num_groups; i++) {
                ctx.groups[i] = malloc(sizeof(struct ChallengeEquivalenceGroup));
                if (ctx.groups[i]) {
                    // The group id is processeed here because it is the name of dictionary, the rest is done inside processChallengeGroup()
                    ctx.groups[i]->id = malloc(sizeof(char) * ((value->u.object.values[i].name_length) + 1));
                    if (ctx.groups[i]->id) {
                        strcpy(ctx.groups[i]->id, value->u.object.values[i].name);
                        group_value = value->u.object.values[i].value;
                        processChallengeEqGroup(i, group_value, depth + 1);
                    } // else --> The pointer is null because it was not possible to allocate memory
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        } // else --> The pointer is null because it was not possible to allocate memory
    }
}

/**
* Processes the json_value given as parameter (interpreted as full contents of config.json) and fills the context
*
* @return
**/
static void processContext(json_value* value, int depth) {
    int num_main_fields;
    printf("\nProcessing config.json and filling context...\n");
    num_main_fields = value->u.object.length;
    for (int i = 0;i < num_main_fields;i++) {
        if      (strcmp(value->u.object.values[i].name, "Folders") == 0)            processFolders(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "Pendrive") == 0)           processPendrive(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "ParentalControl") == 0)    processParentalControl(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "SyncFolders") == 0)        processSyncFolders(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "OperativeTables") == 0)    processOperativeTables(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "Apps") == 0)               processApps(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "ChallengeEqGroups") == 0)  processChallengeEqGroups(value->u.object.values[i].value, depth + 1);
        else printf("WARINING: the field '%s' included in config.json is not registered and will not be processed.\n", value->u.object.values[i].name);
    }
    printf("Processing completed\n");
}

/**
* Loads, reads and processes the config.json filling the global ctx variable. 
* 
* @return
**/
void loadContext() {
    char* file_name;
    FILE* fp;
    struct stat file_status;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;

    // Set json path
    file_name = "../../config.json";

    // Check availability of file and get size
    if (stat(file_name, &file_status) != 0) {
        fprintf(stderr, "File %s not found\n", file_name);
        exit(1);
    }
    file_size = file_status.st_size;

    // Assign space for the file contents
    file_contents = (char*)malloc(file_status.st_size);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        exit(1);
    }

    // Try to open file
    fp = fopen(file_name, "rb");    // Read is done y binary mode. Othrewise fread() does not return (fails)
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", file_name);
        //fclose(fp);       // It is not needed to close file if fopen() fails
        free(file_contents);
        exit(1);
    }
    if (fread(file_contents, file_size, 1, fp) != 1) {
        fprintf(stderr, "Unable t read content of %s\n", file_name);
        fprintf(stderr, "size %d\n", file_size);
        fclose(fp);
        free(file_contents);
        exit(1);
    }
    fclose(fp);

    // JSON parsing and syntax validation
    json = (json_char*)file_contents;
    value = json_parse(json, file_size);
    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    // Fill the context with all the JSON data
    processContext(value, 0);

    // Free unnecessary pointers
    json_value_free(value);     // This frees all the internal pointers. Be sure to have copied the data and not just pointed to it.
    free(file_contents);

    // Translate strings ids to pointers
    translateIdsToPointers();

    // Convert sync folder paths to use secure-mirror letters
    convertSyncFolderPaths();

    // Format all paths in the context
    formatCtxPaths();

    return;
}

/**
* Translates the char pointers which hold identifiers refering to other structs into pointers to those corresponding structs. 
* Frees the identifier pointers so there is no memory leak. 
* More specifically modifies the following fields: 
*   (Folders[i]->Protection->OpTable & ChallengeEqGroups) ,
*   (Pendrive->Protection->OpTable & ChallengeEqGroups) ,
*   (Parental Control->ChallengeEqGroups).
* 
* @return
**/
void translateIdsToPointers() {
    void* tmp_ptr;

    printf("\nTranslating ids to pointers where possible...\n");

    //PRINT("Translating ids to pointers: Folders  -->  Protection\n");
    for (int i = 0; i < _msize(ctx.folders) / sizeof(struct Folder*); i++) {

        // Fix ids from:    Folders  -->  Protection  -->  OpTable
        //PRINT1("Translating ids to pointers: Folders  -->  Protection  -->  OpTable\n");

        //PRINT1("ID before changes: %s\n", (char*)ctx.folders[i]->protection->op_table);
        tmp_ptr = getOpTableById((char*)ctx.folders[i]->protection->op_table);  // Get true pointer
        free(ctx.folders[i]->protection->op_table);                             // Free the char* of the ID
        ctx.folders[i]->protection->op_table = tmp_ptr;                         // Assign the true pointer
        //PRINT1("ID after changes: %s\n", ctx.folders[i]->protection->op_table->id);

        // Fix ids from:    Folders  -->  Protection  -->  ChallengeEqGroups
        //PRINT1("Translating ids to pointers: Folders  -->  Protection  -->  ChallengeEqGroups: \n");
        for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_groups) / sizeof(char*); j++) {

            //PRINT2("ID before changes: %s\n", (char*)ctx.folders[i]->protection->challenge_groups[j]);
            tmp_ptr = getChallengeGroupById((char*)ctx.folders[i]->protection->challenge_groups[j]);    // Get true pointer
            free(ctx.folders[i]->protection->challenge_groups[j]);                                      // Free the char* of the ID
            ctx.folders[i]->protection->challenge_groups[j] = tmp_ptr;                                  // Assign the true pointer
            //PRINT2("ID after changes: %s\n", ctx.folders[i]->protection->challenge_groups[j]->id);
        }
    }


    //PRINT("Translating ids to pointers: Pendrive  -->  Protection\n");

    // Fix ids from:    Pendrive  -->  Protection  -->  OpTable
    //PRINT1("Translating ids to pointers: Pendrive  -->  Protection  -->  OpTable\n");

    //PRINT1("ID before changes: %s\n", (char*)ctx.pendrive->protection->op_table);
    tmp_ptr = getOpTableById((char*)ctx.pendrive->protection->op_table);        // Get true pointer
    free(ctx.pendrive->protection->op_table);                                   // Free the char* of the ID
    ctx.pendrive->protection->op_table = tmp_ptr;                               // Assign the true pointer
    //PRINT1("ID after changes: %s\n", ctx.pendrive->protection->op_table->id);

    // Fix ids from:    Pendrive  -->  Protection  -->  ChallengeEqGroups
    //PRINT1("Translating ids to pointers: Pendrive  -->  Protection  -->  ChallengeEqGroups: \n");
    for (int j = 0; j < _msize(ctx.pendrive->protection->challenge_groups) / sizeof(char*); j++) {

        //PRINT2("ID before changes: %s\n", (char*)ctx.pendrive->protection->challenge_groups[j]);
        tmp_ptr = getChallengeGroupById((char*)ctx.pendrive->protection->challenge_groups[j]);          // Get true pointer
        free(ctx.pendrive->protection->challenge_groups[j]);                                            // Free the char* of the ID
        ctx.pendrive->protection->challenge_groups[j] = tmp_ptr;                                        // Assign the true pointer
        //PRINT2("ID after changes: %s\n", ctx.folders[i]->protection->challenge_groups[j]->id);
    }


    // Fix ids from:    Parental Control  -->  ChallengeEqGroups
    //PRINT("Translating ids to pointers: Parental Control  -->  ChallengeEqGroups: \n");
    for (int i = 0; i < _msize(ctx.parental->challenge_groups) / sizeof(char*); i++) {

        //PRINT1("ID before changes: %s\n", (char*)ctx.parental->challenge_groups[i]);
        tmp_ptr = getChallengeGroupById((char*)ctx.parental->challenge_groups[i]);      // Get true pointer
        free((char*)ctx.parental->challenge_groups[i]);                                 // Free the char* of the ID
        ctx.parental->challenge_groups[i] = tmp_ptr;                                    // Assign the true pointer
        //PRINT1("ID after changes: %s\n", ctx.parental->challenge_groups[i]->id);
    }
    printf("Translation completed\n");
}


void convertSyncFolderPaths() {
    char* tmp_str = NULL;
    char** new_sync_folders = NULL;

    printf("\nConverting sync folder paths to use secure-mirror letters where possible...\n");
    PRINT("TO DO");


    // TO DO  --> convert each sync folder path
    for (int i = 0; i < _msize(ctx.sync_folders) / sizeof(char*); i++) {
        // Each folder is: ctx.sync_folders[i]

        // Check if for each of the Folders the path is subprefix of the current syncfolder or vice versa
        for (int j = 0; j < _msize(ctx.folders) / sizeof(struct Folder*); j++) {
            // Each folder is: ctx.folders[j].path

            /*tmp_str = strstr(ctx.sync_folders[i], ctx.folders[j]->path);  // also the inverse?? a mount folder can be inside a full syncfolder
            if (tmp_str != NULL && tmp_str == file_full_path) {
                // It matches a syncfolder
            }*/
        }
    }

    // TO DO free the sync folders that could not be converted and realloc syncfolders for the correct size


    printf("Conversion completed\n");
}


void formatCtxPaths() {
    printf("\nFormatting paths...\n");
    PRINT("TO DO");
    // TO DO
    //for each path in the context, call formatPath()
    printf("Formatting completed\n");
}
