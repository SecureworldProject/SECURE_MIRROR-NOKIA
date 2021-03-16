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
    //printf("Longitud del objeto %d\n",length);
    for (x = 0; x < length; x++) {
        print_depth_shift(depth);
        printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
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
    printf("array\n");
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

/*static void processFolder_old(int index,json_value* value, int depth) {
    //cada value es un elemento del array, es decir un diccionario
    int dict_length, x, protection_length, i;
    char* path;
    char* mount_point;
    enum Driver driver;
    char* driver_str;
    json_value* protection_value;
    struct Protection protection;
    char* op_table = "";
    char** challenge_group_ids = NULL;
    char* cipher = "";

    dict_length = value->u.object.length;
    //printf("Longitud del objeto %d\n",length);
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);
        //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        if (strcmp(value->u.object.values[x].name, "Path") == 0) {
            //printf("He pillado el path: %s\n", value->u.object.values[x].value->u.string.ptr);
            path = value->u.object.values[x].value->u.string.ptr;
            ctx.folders[index].path = path;
        }
        else if (strcmp(value->u.object.values[x].name, "MountPoint") == 0) {
            //printf("He pillado el MOUNTPOINT: %s\n", value->u.object.values[x].value->u.string.ptr);
            mount_point = value->u.object.values[x].value->u.string.ptr;
            ctx.folders[index].mount_point = mount_point;
        }
        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            //printf("He pillado el driver: %s\n", value->u.object.values[x].value->u.string.ptr);
            driver_str = value->u.object.values[x].value->u.string.ptr; //Deberia ser el enum y es string
            if (strcmp(driver_str, "WinFSP") == 0) driver = WINFSP;
            else driver = DOKAN; //Todo lo que no sea WinFSP bien escrito es Dokan
            ctx.folders[index].driver = driver;
        }
        /*else if (strcmp(value->u.object.values[x].name, "Protections") == 0) {
            //Ojo protection es un structure, hay que meterse mas a fondo en el json
            protection_length = value->u.object.values[x].value->u.object.length;
            protection_value = value->u.object.values[x].value;
            printf("Protection tiene %d, \n", protection_length);
            for (i = 0;i < protection_length;i++) {
            //OJO EL FALLO ESTA AL PILLAR EL VALOR NO EN EL PROTECTION NI EN NADA DE ESO
                if (strcmp(protection_value->u.object.values[x].name, "OpTable") == 0) {
                    op_table = protection_value->u.object.values[x].value->u.string.ptr;
                    printf("optable %s\n", op_table);
                    //protection.op_table = op_table;
                }
                else if (strcmp(protection_value->u.object.values[x].name, "Cipher") == 0) {
                    cipher = protection_value->u.object.values[x].value->u.string.ptr;
                    //protection.cipher = cipher;
                }
            }
            //challenge_group_ids = ;
            //protection.op_table = op_table;
            //protection.challenge_group_ids = challenge_group_ids;
            //protection.cipher = cipher;
            ctx.folders[index].protection->op_table = op_table;
            ctx.folders[index].protection->challenge_group_ids = NULL;
            ctx.folders[index].protection->cipher = cipher;

        }*//*
    }
    printf("La carpeta %d tiene\n", index);
    printf("        path %s\n", ctx.folders[index].path);
    printf("        mount point %s\n", ctx.folders[index].mount_point);
    printf("        driver %d\n", ctx.folders[index].driver);
    //printf("        protection.optable %s\n", ctx.folders[index].protection->op_table);
    //printf("        protection.cipher %s\n", ctx.folders[index].protection->cipher);
}*/

static void processProtection(int index, json_value* value, int depth) {
    int dict_length, x, num_groups;

    dict_length = value->u.object.length;
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);

        if (strcmp(value->u.object.values[x].name, "OpTable") == 0) {
            // This will be a OpTable pointer but for now it will hold the id, so force it as char pointer
            ctx.folders[index]->protection->op_table = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx.folders[index]->protection->op_table) {
                strcpy(ctx.folders[index]->protection->op_table, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        if (strcmp(value->u.object.values[x].name, "ChallengeEqGroups") == 0) {
            //Este objeto es un array
            num_groups = value->u.object.values[x].value->u.array.length;
            if (num_groups <= 0) {          // Fixes warning C6386 (Visual Studio bug)
                ctx.folders[index]->protection->challenge_group_ids = NULL;
            } else {
                ctx.folders[index]->protection->challenge_group_ids = (char**) malloc(num_groups * sizeof(char*));   // Allocate space for all pointers to char
                if (ctx.folders[index]->protection->challenge_group_ids) {
                    for (int i = 0; i < num_groups; i++) {
                        ctx.folders[index]->protection->challenge_group_ids[i] = (char*) malloc(sizeof(char) * ((value->u.object.values[x].value->u.array.values[i]->u.string.length) + 1));
                        if (ctx.folders[index]->protection->challenge_group_ids[i]) {
                            strcpy(ctx.folders[index]->protection->challenge_group_ids[i], value->u.object.values[x].value->u.array.values[i]->u.string.ptr);
                        } // else --> The pointer is null because it was not possible to allocate memory
                    }
                } // else --> The pointer is null because it was not possible to allocate memory
            }
        }

        if (strcmp(value->u.object.values[x].name, "Cipher") == 0) {
            ctx.folders[index]->protection->cipher = malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx.folders[index]->protection->cipher) {
                strcpy(ctx.folders[index]->protection->cipher, value->u.object.values[x].value->u.string.ptr);
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
    //printf("Longitud del objeto %d\n",length);
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);
        //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        if (strcmp(value->u.object.values[x].name, "Path") == 0) {
            ctx.folders[index]->path = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length)+1));
            if (ctx.folders[index]->path) {
                strcpy(ctx.folders[index]->path, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        else if (strcmp(value->u.object.values[x].name, "MountPoint") == 0) {
            ctx.folders[index]->mount_point = (char*)malloc(sizeof(char) * ((value->u.object.values[x].value->u.string.length) + 1));
            if (ctx.folders[index]->mount_point) {
                strcpy(ctx.folders[index]->mount_point, value->u.object.values[x].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
        }

        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            //printf("He pillado el driver: %s\n", value->u.object.values[x].value->u.string.ptr);
            driver_str = value->u.object.values[x].value->u.string.ptr;
            if (strcmp(driver_str, "WinFSP") == 0)          driver = WINFSP;
            else if (strcmp(driver_str, "DOKAN") == 0)      driver = DOKAN;
            else                                            driver = DOKAN;     // This is the default driver: Dokan
            ctx.folders[index]->driver = driver;
        }

        else if (strcmp(value->u.object.values[x].name, "Protections") == 0) {
            ctx.folders[index]->protection = (struct Protection*)malloc(sizeof(struct Protection));
            if (ctx.folders[index]->protection) {
                protection_value = value->u.object.values[x].value;
                processProtection(index, protection_value, depth + 1);
            } // else --> The pointer is null because it was not possible to allocate memory
        }
    }
}

static void processFolders(json_value* value, int depth) {
    int array_length;
    int array_pos = 0;
    int in_folder_length = 0;
    int in_folder_pos = 0;

    printf("Procesando el Folder\n");
    array_length = value->u.array.length;
    printf("Hay %d carpetas\n", array_length);
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
                        ctx.parental[i].challenge_group_ids = (char**)malloc(users_array_length * sizeof(char*));
                        if (ctx.parental[i].challenge_group_ids) {
                            for (int k = 0; k < users_array_length; k++) {
                                // This will be a ChallengeEquivalenceGroup pointer but for now it will hold is, so force it to char pointer
                                ctx.parental[i].challenge_group_ids[k] = (char*)malloc(sizeof(char) * ((array_value->u.object.values[j].value->u.array.values[k]->u.string.length) + 1));
                                if (ctx.parental[i].challenge_group_ids[k]) {
                                    strcpy(ctx.parental[i].challenge_group_ids[k], array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr);
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
    int num_elems, i, len;
    enum AppType app_type = ANY;
    char* app_type_str;
    char* op_str;
    enum Operation op = NOTHING;

    num_elems = value->u.object.length;
    printf("La fila %d tiene %d elementos\n", row_index, num_elems);

    ctx.tables[table_index]->tuples[row_index] = (struct Tuple*)malloc(sizeof(struct Tuple));
    if (ctx.tables[table_index]->tuples[row_index]) {
        for (i = 0; i < num_elems; i++) {
            if (strcmp(value->u.object.values[i].name, "AppType") == 0) {
                app_type_str = value->u.object.values[i].value->u.string.ptr;
                if (strcmp(app_type_str, "BROWSER") == 0) app_type = BROWSER;
                else if (strcmp(app_type_str, "MAILER") == 0) app_type = MAILER;
                else if (strcmp(app_type_str, "BLOCKED") == 0) app_type = BLOCKED;
                else {
                    printf("WARNING: incorrect apptype, defaulting to ANY.\n");
                    app_type = ANY;         // If string does not match, ANY is used by default
                }
                ctx.tables[table_index]->tuples[row_index]->app_type = app_type;
            }

            if (strcmp(value->u.object.values[i].name, "Disk") == 0) {
                len = value->u.object.values[i].value->u.string.length;
                ctx.tables[table_index]->tuples[row_index]->disk = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
                if (ctx.tables[table_index]->tuples[row_index]->disk) {
                    strcpy(ctx.tables[table_index]->tuples[row_index]->disk, value->u.object.values[i].value->u.string.ptr);
                } // else --> The pointer is null because it was not possible to allocate memory
            }

            if (strcmp(value->u.object.values[i].name, "READ") == 0) {
                op_str = value->u.object.values[i].value->u.string.ptr;
                printf("op read es: %s\n", op_str);
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

            if (strcmp(value->u.object.values[i].name, "WRITE") == 0) {
                op_str = value->u.object.values[i].value->u.string.ptr;
                printf("op write es: %s\n", op_str);
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

    printf("The Tuple %d contained: AppType: %d - Disk: %s - ActionRead: %d - ActionWrite: %d \n",
        row_index,
        ctx.tables[table_index]->tuples[row_index]->app_type,
        ctx.tables[table_index]->tuples[row_index]->disk,
        ctx.tables[table_index]->tuples[row_index]->on_read,
        ctx.tables[table_index]->tuples[row_index]->on_write);
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
        printf("La app %d, tiene path: %s, name: %s, y tipo: %d que es %s\n", index, ctx.apps[index]->path, ctx.apps[index]->name, ctx.apps[index]->type, app_type_str);
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

            if (strcmp(value->u.object.values[i].name, "ID") == 0) {
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
        printf("El challenge %d del grupo %d tiene ID: %s y Props: %s\n", challenge_index, group_index, ctx.groups[group_index]->challenges[challenge_index]->id, ctx.groups[group_index]->challenges[challenge_index]->properties);
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
            ctx.groups[index]->expires = (char*)malloc(sizeof(char) * ((value->u.object.values[i].value->u.string.length) + 1));
            if (ctx.groups[index]->expires) {
                strcpy(ctx.groups[index]->expires, value->u.object.values[i].value->u.string.ptr);
            } // else --> The pointer is null because it was not possible to allocate memory
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
    printf("El grupo de equivalencia %d: expires= %s, subkey= %s y %d challenges\n",index, ctx.groups[index]->expires, ctx.groups[index]->subkey, num_challenges);
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
    printf("Processing config.json and filling context\n");
    num_main_fields = value->u.object.length;
    for (int i = 0;i < num_main_fields;i++) {
        printf("Nombre: %s\n", value->u.object.values[i].name);
        if (strcmp(value->u.object.values[i].name, "Folders") == 0)                 processFolders(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "ParentalControl") == 0)    processParentalControl(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "SyncFolders") == 0)        processSyncFolders(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "OperativeTables") == 0)    processOperativeTables(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "Apps") == 0)               processApps(value->u.object.values[i].value, depth + 1);
        else if (strcmp(value->u.object.values[i].name, "ChallengeEqGroups") == 0)  processChallengeEqGroups(value->u.object.values[i].value, depth + 1);
        else printf("El nombre %s por ahora no esta registrado y no se va a procesar.\n", value->u.object.values[i].name);
    }
}

/*int loadContextOLD() {

    char* filename;
    FILE* fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;

    //Carga del fichero json y lectura del contenido
    filename = "../../config.json";

    if (stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename, "rt");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if (fread(file_contents, file_size, 1, fp) != 1) {
        fprintf(stderr, "Unable t read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    //printf("%s\n", file_contents);

    //printf("--------------------------------\n\n");
    //--------------

    //Parseo del contenido del json
    json = (json_char*)file_contents;

    value = json_parse(json, file_size);

    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    //process_value(value, 0);
    processContext(value, 0);
    json_value_free(value);
    free(file_contents);
    return 0;
}*/

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

    return;
}

/**
* Translates the char pointers which hold identifiers refering to other structs into pointers to those corresponding structs. 
* Frees the identifier pointers so there is no memory leak. 
* More specifically modifies the following fields: 
*   (Folders->Protection->OpTable) ,  
*   (Folders->Protection->ChallengeEqGroups) , 
*   (Parental Control->ChallengeEqGroups).
* 
* @return
**/
void translateIdsToPointers() {
    void* tmp_ptr;

    PRINT("CLEANING: Folders  -->  Protection\n");
    for (int i = 0; i < _msize(ctx.folders) / sizeof(struct Folder*); i++) {

        // Fix ids from:    Folders  -->  Protection  -->  OpTable
        PRINT1("CLEANING: Folders  -->  Protection  -->  OpTable\n");

        PRINT1("ID before changes: %s\n", (char*)ctx.folders[i]->protection->op_table);
        // Get true pointer
        tmp_ptr = getOpTableById((char*)ctx.folders[i]->protection->op_table);
        // Free the char* of the ID
        free(ctx.folders[i]->protection->op_table);
        // Assign the true pointer
        ctx.folders[i]->protection->op_table = tmp_ptr;
        PRINT1("ID after changes: %s\n", ctx.folders[i]->protection->op_table->id);

        // Fix ids from:    Folders  -->  Protection  -->  ChallengeEqGroups
        PRINT1("CLEANING: Folders  -->  Protection  -->  ChallengeEqGroups: \n");
        for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_group_ids) / sizeof(char*); j++) {

            PRINT2("ID before changes: %s\n", (char*)ctx.folders[i]->protection->challenge_group_ids[j]);
            // Get true pointer
            tmp_ptr = getChallengeGroupById((char*)ctx.folders[i]->protection->challenge_group_ids[j]);
            // Free the char* of the ID
            free(ctx.folders[i]->protection->challenge_group_ids[j]);
            // Assign the true pointer
            ctx.folders[i]->protection->challenge_group_ids[j] = tmp_ptr;
            PRINT2("ID after changes: %s\n", ctx.folders[i]->protection->challenge_group_ids[j]->id);
        }
    }

    // Fix ids from:    Parental Control  -->  ChallengeEqGroups
    PRINT("CLEANING: Parental Control  -->  ChallengeEqGroups: \n");
    for (int i = 0; i < _msize(ctx.parental->challenge_group_ids) / sizeof(char*); i++) {

        PRINT1("ID before changes: %s\n", (char*)ctx.parental->challenge_group_ids[i]);
        // Get true pointer
        tmp_ptr = getChallengeGroupById((char*)ctx.parental->challenge_group_ids[i]);
        // Free the char* of the ID
        free((char*)ctx.parental->challenge_group_ids[i]);
        // Assign the true pointer
        ctx.parental->challenge_group_ids[i] = tmp_ptr;
        PRINT1("ID after changes: %s\n", ctx.parental->challenge_group_ids[i]->id);
    }
}
