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
        printf(L" ");
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
    int j;
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
    char** challenge_groups = NULL;
    char* cipher = "";

    dict_length = value->u.object.length;
    //printf("Longitud del objeto %d\n",length);
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);
        //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        if (strcmp(value->u.object.values[x].name, "Path") == 0) {
            //printf("He pillado el path: %s\n", value->u.object.values[x].value->u.string.ptr);
            path = value->u.object.values[x].value->u.string.ptr;
            context.folders[index].path = path;
        }
        else if (strcmp(value->u.object.values[x].name, "MountPoint") == 0) {
            //printf("He pillado el MOUNTPOINT: %s\n", value->u.object.values[x].value->u.string.ptr);
            mount_point = value->u.object.values[x].value->u.string.ptr;
            context.folders[index].mount_point = mount_point;
        }
        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            //printf("He pillado el driver: %s\n", value->u.object.values[x].value->u.string.ptr);
            driver_str = value->u.object.values[x].value->u.string.ptr; //Deberia ser el enum y es string
            if (strcmp(driver_str, "WinFSP") == 0) driver = WINFSP;
            else driver = DOKAN; //Todo lo que no sea WinFSP bien escrito es Dokan
            context.folders[index].driver = driver;
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
            //challenge_groups = ;
            //protection.op_table = op_table;
            //protection.challenge_groups = challenge_groups;
            //protection.cipher = cipher;
            context.folders[index].protection->op_table = op_table;
            context.folders[index].protection->challenge_groups = NULL;
            context.folders[index].protection->cipher = cipher;

        }*//*
    }
    printf("La carpeta %d tiene\n", index);
    printf("        path %s\n", context.folders[index].path);
    printf("        mount point %s\n", context.folders[index].mount_point);
    printf("        driver %d\n", context.folders[index].driver);
    //printf("        protection.optable %s\n", context.folders[index].protection->op_table);
    //printf("        protection.cipher %s\n", context.folders[index].protection->cipher);
}*/

static void processProtections(int index, json_value* value, int depth) {

    struct Protection protection;
    char* op_table = "";
    char** challenge_groups = NULL;
    char* cipher = "";
    int dict_length, x, num_groups;
    char* challenge_list;
    int long_str;

    dict_length = value->u.object.length;
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);
        if (strcmp(value->u.object.values[x].name, "OpTable") == 0) {
            //printf("OPTABLE: %s\n", value->u.object.values[x].value->u.string.ptr);
            context.folders[index]->protection->op_table = value->u.object.values[x].value->u.string.ptr;
        }
        if (strcmp(value->u.object.values[x].name, "ChallengeEqGroups") == 0) {
            //Este objeto es un array
            context.folders[index]->protection->challenge_groups = value->u.object.values[x].value->u.string.ptr;
            num_groups = value->u.object.values[x].value->u.array.length;
            context.folders[index]->protection->challenge_groups = malloc(num_groups * sizeof(char*));

            for (int i = 0;i < num_groups;i++) {
                long_str = strlen(value->u.object.values[x].value->u.array.values[i]->u.string.ptr);
                //context.folders[index]->protection->challenge_groups[i] = malloc(long_str * sizeof(char));
                context.folders[index]->protection->challenge_groups[i] = value->u.object.values[x].value->u.array.values[i]->u.string.ptr;
                //strcpy(context.folders[index]->protection->challenge_groups[i], value->u.object.values[x].value->u.array.values[i]->u.string.ptr);
            }
            //challenge_list = value->u.object.values[x].value->u.array;
        }
        if (strcmp(value->u.object.values[x].name, "Cipher") == 0) { context.folders[index]->protection->cipher = value->u.object.values[x].value->u.string.ptr; }
    }


}

static void processFolder(int index, json_value* value, int depth) {
    //cada value es un elemento del array, es decir un diccionario
    int dict_length, x, protection_length, i;
    char* path;
    char* mount_point;
    enum Driver driver;
    char* driver_str;
    json_value* protection_value;

    dict_length = value->u.object.length;
    //printf("Longitud del objeto %d\n",length);
    for (x = 0; x < dict_length; x++) {
        print_depth_shift(depth);
        //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        if (strcmp(value->u.object.values[x].name, "Path") == 0) {
            //printf("He pillado el path: %s\n", value->u.object.values[x].value->u.string.ptr);
            path = value->u.object.values[x].value->u.string.ptr;
            //folders[index].path = path;
            context.folders[index]->path = path;
        }
        else if (strcmp(value->u.object.values[x].name, "MountPoint") == 0) {
            //printf("He pillado el MOUNTPOINT: %s\n", value->u.object.values[x].value->u.string.ptr);
            mount_point = value->u.object.values[x].value->u.string.ptr;
            //folders[index].mount_point = mount_point;
            context.folders[index]->mount_point = mount_point;
        }
        else if (strcmp(value->u.object.values[x].name, "Driver") == 0) {
            //printf("He pillado el driver: %s\n", value->u.object.values[x].value->u.string.ptr);
            driver_str = value->u.object.values[x].value->u.string.ptr; //Deberia ser el enum y es string
            if (strcmp(driver_str, "WinFSP") == 0) driver = WINFSP;
            else driver = DOKAN; //Todo lo que no sea WinFSP bien escrito es Dokan
            //folders[index].driver = driver;
            context.folders[index]->driver = driver;
        }
        else if (strcmp(value->u.object.values[x].name, "Protections") == 0) {
            context.folders[index]->protection = malloc(sizeof(struct Protection));
            protection_value = value->u.object.values[x].value;
            processProtections(index, protection_value, depth + 1);
        }
    }
    //context.folders[index] = &folders[index];
}

static void processFolders(json_value* value, int depth) {
    int array_length, array_pos, in_folder_length, in_folder_pos;
    printf("Procesando el Folder\n");
    array_length = value->u.array.length; //El folder es un array
    printf("Hay %d carpetas\n", array_length);
    context.folders = malloc(array_length * sizeof(struct Folder)); //Por que tengo que hacer esto
    //struct Folder* folders = malloc(array_length * sizeof(struct Folder));
    for (int i = 0;i < array_length;i++) {
        context.folders[i] = malloc(sizeof(struct Folder));// Con esto lo mismo ya puedo pasar de la variable folders
        processFolder(i, value->u.array.values[i], depth);
    }
    //free(folders);
    for (int i = 0;i < array_length;i++) {
        printf("La carpeta %d tiene\n", i);
        printf("        path %s\n", context.folders[i]->path);
        printf("        mount point %s\n", context.folders[i]->mount_point);
        printf("        driver %d\n", context.folders[i]->driver);
        printf("        protection optable %s\n", context.folders[i]->protection->op_table);
        printf("        protection chgroup %s\n", context.folders[i]->protection->challenge_groups[0]);
        printf("        protection cipher %s\n", context.folders[i]->protection->cipher);
    }
    //free(folders);
}

static void processParentalControl(json_value* value, int depth) {
    int array_length, dict_length, users_array_length;
    json_value* array_value;
    array_length = value->u.array.length;
    context.parental = malloc(array_length * sizeof(struct ParentalControl));
    for (int i = 0;i < array_length;i++) {
        array_value = value->u.array.values[i];
        dict_length = array_value->u.object.length;
        for (int j = 0; j < dict_length;j++) {
            if (strcmp(array_value->u.object.values[j].name, "Folder") == 0) {
                context.parental[i].folder = array_value->u.object.values[j].value->u.string.ptr;
            }
            if (strcmp(array_value->u.object.values[j].name, "Users") == 0) {
                //Es un array
                users_array_length = array_value->u.object.values[j].value->u.array.length;
                context.parental[i].users = malloc(users_array_length * sizeof(char*));
                for (int k = 0; k < users_array_length;k++) {
                    context.parental[i].users[k] = array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr;
                }
            }
            if (strcmp(array_value->u.object.values[j].name, "ChallengeEqGroups") == 0) {
                //Es un array
                users_array_length = array_value->u.object.values[j].value->u.array.length;
                context.parental[i].challenge_groups = malloc(users_array_length * sizeof(char*));
                for (int k = 0; k < users_array_length;k++) {
                    context.parental[i].challenge_groups[k] = array_value->u.object.values[j].value->u.array.values[k]->u.string.ptr;
                }
            }
        }
        

    }
    printf("Controlparental: users: %s\n", context.parental[0].challenge_groups[0]);
}

static void processSyncFolders(json_value* value, int depth) {
    int array_length;
    array_length = value->u.array.length;
    context.sync_folders = malloc(array_length * sizeof(char*));
    for (int i = 0; i < array_length;i++) {
        context.sync_folders[i] = value->u.array.values[i]->u.string.ptr;
        printf("sync folder %d es %s\n",i,context.sync_folders[i]);
    }
}

static void processTableTuple(int table_index, int row_index, json_value* value, int depth) {
    int num_tuples, i;
    enum AppType app_type;
    char* app_type_str;//, disk_str;
    char* disk_str;
    char* op_str;
    enum Operation op;
    num_tuples = value->u.object.length;
    printf("La fila %d tiene %d elementos\n", row_index, num_tuples);
    for (i = 0;i < num_tuples;i++) {
        //if (strcmp(value->u.object.values[i].name,"")==0)
        printf("Nombre de elemento: %s\n", value->u.object.values[i].name);
        context.table[table_index]->table_tuple[row_index] = malloc(sizeof(struct TableTuple));
        if (strcmp(value->u.object.values[i].name, "AppType") == 0) {
            app_type_str = value->u.object.values[i].value->u.string.ptr;
            printf("El tipo de app es %s\n", app_type_str);
            //printf("La tabla %d : fila %d tiene un apptype: %s\n", table_index, row_index, app_type_str);
            if (strcmp(app_type_str, "BROWSER") == 0) app_type = BROWSER;
            else if (strcmp(app_type_str, "MAILER") == 0) app_type = MAILER;
            else if (strcmp(app_type_str, "BLOCKED") == 0) app_type = BLOCKED;
            context.table[table_index]->table_tuple[row_index]->app_type = app_type;
            printf("====La tabla %d : fila %d tiene un apptype: %d\n", table_index, row_index, context.table[table_index]->table_tuple[row_index]->app_type);
            //context.table[0]->table_tuple[0]->app_type = BROWSER;
        }
        if (strcmp(value->u.object.values[i].name, "Disk") == 0) {
            disk_str = value->u.object.values[i].value->u.string.ptr;
            printf("El disco es: %s\n", disk_str);
            context.table[table_index]->table_tuple[row_index]->disk = disk_str;
            printf("====La tabla %d : fila %d tiene un disk: %s\n", table_index, row_index, context.table[table_index]->table_tuple[row_index]->disk);
        }
        if (strcmp(value->u.object.values[i].name, "READ") == 0){
            op_str = value->u.object.values[i].value->u.string.ptr;
            printf("op read es: %s\n", op_str);
            if (strcmp(op_str, "NOTHING") == 0) op = NOTHING;
            else if (strcmp(op_str, "CIPHER") == 0) op = CIPHER;
            else if (strcmp(op_str, "DECIPHER") == 0) op = DECIPHER;
            else if (strcmp(op_str, "MARK") == 0) op = MARK;
            else if (strcmp(op_str, "UNMARK") == 0) op = UNMARK;
            else if (strcmp(op_str, "IF_MARK_UNMARK_ELSE_CIPHER") == 0) op = IF_MARK_UNMARK_ELSE_CIPHER;
            else if (strcmp(op_str, "IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING") == 0) op = IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING;
            context.table[table_index]->table_tuple[row_index]->on_read = op;
            printf("====La tabla %d : fila %d tiene un onread: %d\n", table_index, row_index, context.table[table_index]->table_tuple[row_index]->on_read);
        }
        if (strcmp(value->u.object.values[i].name, "WRITE") == 0){
            op_str = value->u.object.values[i].value->u.string.ptr;
            printf("op write es: %s\n", op_str);
            if (strcmp(op_str, "NOTHING") == 0) op = NOTHING;
            else if (strcmp(op_str, "CIPHER") == 0) op = CIPHER;
            else if (strcmp(op_str, "DECIPHER") == 0) op = DECIPHER;
            else if (strcmp(op_str, "MARK") == 0) op = MARK;
            else if (strcmp(op_str, "UNMARK") == 0) op = UNMARK;
            else if (strcmp(op_str, "IF_MARK_UNMARK_ELSE_CIPHER") == 0) op = IF_MARK_UNMARK_ELSE_CIPHER;
            else if (strcmp(op_str, "IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING") == 0) op = IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING;
            context.table[table_index]->table_tuple[row_index]->on_write = op;
            printf("====La tabla %d : fila %d tiene un onwrite: %d\n", table_index, row_index, context.table[table_index]->table_tuple[row_index]->on_write);
        }

        //printf("====La tabla %d : fila %d tiene un apptype: %d\n", table_index, row_index, context.table[table_index]->table_tuple[row_index]->app_type);
    }
}

static void processOperativeTables(json_value* value, int depth) {
    int num_tables, i, j, num_rows;
    char* id_table;
    json_value* row_value;
    //Es un diccionario de tablas (que son listas de diccionarios)
    num_tables = value->u.object.length;
    context.table = malloc(num_tables * sizeof(struct OpTable));
    for (i = 0;i < num_tables;i++) {
        id_table = value->u.object.values[i].name;
        context.table[i] = malloc(sizeof(struct OpTable));
        context.table[i]->ID_table = id_table;
        printf("La tabla operativa %d se llama %s\n",i, context.table[i]->ID_table);
        //Cada tupla de la tabla es una fila
        num_rows = value->u.object.values[i].value->u.array.length;
        printf("Hay %d filas en la tabla\n", num_rows);
        context.table[i]->table_tuple = malloc(num_rows * sizeof(struct TableTuple));
        for (j = 0;j < num_rows;j++) {
            row_value = value->u.object.values[i].value->u.array.values[j];
            processTableTuple(i, j, row_value, depth + 1);
            //row_value = value->u.object.values[i].value->u.object.values[j].name;
            //printf("    La fila %d vale %s\n",j,row_value);
            
        }
    }
}

static void processApp(int index, json_value* value, int depth) {
    int num_elem, i;
    char* app_path;
    char* app_name;
    char* app_type_str = "";
    enum AppType app_type;
    //Cada app es un diccionario, una tupla con nombre de tres elementos
    context.app[index] = malloc(sizeof(struct App));
    //recorro los componentes de la tupla  asigno cada uno
    num_elem = value->u.object.length;
    for (i = 0;i < num_elem;i++) {
        if (strcmp(value->u.object.values[i].name, "AppPath") == 0) {
            app_path = value->u.object.values[i].value->u.string.ptr;
            context.app[index]->app_path = app_path;
        }
        if (strcmp(value->u.object.values[i].name, "AppName") == 0) {
            app_name = value->u.object.values[i].value->u.string.ptr;
            context.app[index]->app_name = app_name;
        }
        if (strcmp(value->u.object.values[i].name, "AppType") == 0) {
            app_type_str = value->u.object.values[i].value->u.string.ptr;
            if (strcmp(app_type_str, "BROWSER")==0) app_type = BROWSER;
            else if (strcmp(app_type_str, "MAILER") == 0) app_type = MAILER;
            else if (strcmp(app_type_str, "BLOCKED") == 0) app_type = BLOCKED;
            else if (strcmp(app_type_str, "ANY") == 0) app_type = ANY;
            context.app[index]->app_type = app_type;
        }
    }
    printf("La app %d, tiene path: %s, name: %s, y tipo: %d que es %s\n", index, context.app[index]->app_path, context.app[index]->app_name, context.app[index]->app_type, app_type_str);
}

static void processApps(json_value* value, int depth) {
    //Lista de diccionarios
    int i, num_apps;
    json_value* app_value;
    num_apps = value->u.array.length;
    context.app = malloc(num_apps * sizeof(struct App*));
    for (i = 0;i < num_apps;i++) {
        app_value = value->u.array.values[i];
        processApp(i, app_value, depth + 1);
    }
}

static void processChallenge(int group_index, int challenge_index, json_value* value, int depth) {
    int i, num_fields;
    char* id_challenge;
    char* properties;

    num_fields = value->u.object.length;
    //printf("Un challenge tiene %d campos\n", num_fields);
    context.group[group_index]->challenge[challenge_index] = malloc(sizeof(struct Challenge));
    for (i = 0;i < num_fields;i++) {
        if (strcmp(value->u.object.values[i].name, "ID") == 0) {
            id_challenge = value->u.object.values[i].value->u.string.ptr;
            context.group[group_index]->challenge[challenge_index]->id_challenge = id_challenge;
        }
        else if (strcmp(value->u.object.values[i].name, "Props") == 0) {
            properties = value->u.object.values[i].value->u.string.ptr;
            context.group[group_index]->challenge[challenge_index]->properties = properties;
        }
        //Los campos Description y Requirements no se tienen en cuenta
    }
    printf("El challenge %d del grupo %d tiene ID: %s y Props: %s\n", challenge_index, group_index, context.group[group_index]->challenge[challenge_index]->id_challenge, context.group[group_index]->challenge[challenge_index]->properties);
}

static void processChallengeEqGroup(int index, json_value* value, int depth) {
    int i, j, num_elems, num_challenges;
    char* subkey;
    char* expires;
    json_value* challenge_value;

    num_elems = value->u.object.length;
    //printf("El grupo de equivalencia %d, tiene %d elementos\n", index, num_elems);
    for (i = 0;i < num_elems;i++) {
        if (strcmp(value->u.object.values[i].name, "SubKey") == 0) {
            subkey = value->u.object.values[i].value->u.string.ptr;
            context.group[index]->subkey = subkey;
        }
        if (strcmp(value->u.object.values[i].name, "Expires") == 0) {
            expires = value->u.object.values[i].value->u.string.ptr;
            context.group[index]->expires = expires;
        }
        if (strcmp(value->u.object.values[i].name, "ChallengeList") == 0) {
            //Es una lista de diccionarios
            num_challenges = value->u.object.values[i].value->u.array.length;
            context.group[index]->challenge = malloc(num_challenges * sizeof(struct Challenge));
            for (j = 0;j < num_challenges;j++) {
                challenge_value = value->u.object.values[i].value->u.array.values[j];
                processChallenge(index, j, challenge_value, depth + 1);
            }

        }
    }
    printf("El grupo de equivalencia %d: expires= %s, subkey= %s y %d challenges\n",index, context.group[index]->expires, context.group[index]->subkey, num_challenges);
}

static void processChallengeEqGroups(json_value* value, int depth) {
    //Esta estructura es un diccionario de diccionarios (y dentro una lista de diccionarios)
    int i, num_groups;
    json_value* group_value;
    num_groups = value->u.object.length;
    context.group = malloc(num_groups * sizeof(struct ChallengeEquivalenceGroup));
    for (i = 0;i < num_groups;i++) {
        context.group[i] = malloc(sizeof(struct ChallengeEquivalenceGroup));
        context.group[i]->id = value->u.object.values[i].name; //Ojo el id del grupo lo proceso aqui, porque es el nombre del diccionario, el resto de datos estan DENTRO del diccionario
        printf("El grupo %d se llama %s\n", i, context.group[i]->id);
        group_value = value->u.object.values[i].value;
        processChallengeEqGroup(i, group_value, depth+1);
    }
}

static void processContext(json_value* value, int depth) {
    int num_main_fields;
    printf("Procesando el contexto\n");
    num_main_fields = value->u.object.length;
    printf("Hay %d campos principales\n", num_main_fields);
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

int loadContext() {

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
}


struct Context load_Context() {
    char* filename;
    FILE* fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;
    //struct Context context;
    //struct Folder folder;
    //struct Protection protection;

    //Carga del fichero json y lectura del contenido
    filename = "../../config.json";

    if (stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        //return 1;
        exit(1);
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        //return 1;
        exit(1);
    }

    fp = fopen(filename, "rb");//read en modo binario, de lo contrario da problemas fread porque no retorna
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        //return 1;
        exit(1);
    }
    if (fread(file_contents, file_size, 1, fp) !=1) {
        fprintf(stderr, "Unable t read content of %s\n", filename);
        fprintf(stderr, "size %d\n", file_size);
        fclose(fp);
        free(file_contents);
        //return 1;
        exit(1);
    }
    fclose(fp);

    //printf("%s\n", file_contents);

    //printf("--------------------------------\n\n");
    //--------------

    //Parseo del contenido del json
    json = (json_char*)file_contents;

    // Validacion sintaxis
    value = json_parse(json, file_size);

    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    //process_value(value, 0);
    processContext(value, 0);
    //json_value_free(value);     // Esto hay que tener cuidado, porque puede liberar todos los punteros internos
    free(file_contents);

    //return 0;
    return context;
}