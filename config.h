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

static void print_depth_shift(int depth);

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth);

static void process_array(json_value* value, int depth);

static void process_value(json_value* value, int depth);

int read_json();