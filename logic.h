#ifndef WRAPPER_DOKAN_H
#define WRAPPER_DOKAN_H

/////  FILE INCLUDES  /////

#include "dokan.h"
#include "context.h"
#include "winnt.h"
#include <psapi.h>
#include "context.h"
#include "wrapper_dokan.h"
//#include "wrapper_winfsp.h"




/////  FUNCTION PROTOTYPES  /////

enum Operation operationAddition(enum Operation op1, enum Operation op2);
enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]);
BOOL preCreateLogic(WCHAR file_path_param[], WCHAR* full_app_path);
int preReadLogic(enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int postReadLogic(enum Operation op, WCHAR file_path[], LPCVOID* in_buffer, DWORD* buffer_length, LPDWORD* bytes_done, LONGLONG* offset, struct Protection* protection, LPCVOID out_buffer);
int preWriteLogic(enum Operation op, WCHAR file_path[], LPCVOID* in_buffer, DWORD* bytes_to_write, LPDWORD* bytes_written, LONGLONG* offset, struct Protection* protection, LPCVOID out_buffer);

#endif //!WRAPPER_DOKAN_H
