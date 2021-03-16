/*
* SecureWorld file config.c
* carga la configuración en el contexto usando un parser. contiene la función loadContext() y checkContext()
y otras posibles funciones auxiliares que necesite. El contexto incluye la carga operativa que luego condiciona
el comportamiento de la función logic().

Nokia Febrero 2021
*/

#include <sys/stat.h>
#include "json.h"
//#include "json.c"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "context.h"


static void print_depth_shift(int depth);

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth);

static void process_array(json_value* value, int depth);

static void process_value(json_value* value, int depth);

static void processProtection(int index, json_value* value, int depth);

static void processFolder(int index, json_value* value, int depth);

static void processFolders(json_value* value, int depth);

static void processParentalControl(json_value* value, int depth);

static void processSyncFolders(json_value* value, int depth);

static void processTableTuple(int table_index, int row_index, json_value* value, int depth);

static void processOperativeTables(json_value* value, int depth);

static void processApp(int index, json_value* value, int depth);

static void processApps(json_value* value, int depth);

static void processChallenge(int group_index, int challenge_index, json_value* value, int depth);

static void processChallengeEqGroup(int index, json_value* value, int depth);

static void processChallengeEqGroups(json_value* value, int depth);

static void processContext(json_value* value, int depth);

//int loadContextOLD();

void loadContext();