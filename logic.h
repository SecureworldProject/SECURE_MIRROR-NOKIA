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

int preCreateLogic(int num, enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int preReadLogic(int num, enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int postReadLogic(int num, enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int preWriteLogic(int num, enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);


#endif //!WRAPPER_DOKAN_H
