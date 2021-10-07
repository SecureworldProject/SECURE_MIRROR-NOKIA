#ifndef LOGIC_H
#define LOGIC_H

/////  FILE INCLUDES  /////

#include "dokan.h"
#include "context.h"
#include "winnt.h"
#include <psapi.h>
#include "context.h"
#include "wrapper_dokan.h"
//#include "wrapper_winfsp.h"




/////  DEFINITIONS  /////

#define MARK_LENGTH 512

struct FileMarkInfo {
	WCHAR file_path[MAX_PATH];	// Contains the path to the file
	int8_t write_buffer_mark_lvl;		// Contains the original ciphering level (-1, 0, 1) of the mark or unknown (represented by -128)
	int8_t file_mark_lvl;		// Contains the current ciphering level (-1, 0, 1) of the mark or unknown (represented by -128)
};

#define INVALID_MARK_LEVEL -128



/////  FUNCTION PROTOTYPES  /////

enum Operation operationAddition(enum Operation op1, enum Operation op2);
enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]);
DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path);

void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key);
void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key);

BOOL checkMarkOLD(uint8_t* input);
BOOL markOLD(uint8_t* input);
BOOL unmarkOLD(uint8_t* input);

//int8_t checkMark(uint8_t* input);
int8_t mark(uint8_t* input, int8_t level);
int8_t unmark(uint8_t* input);
void testFMItable();


// Logic functions

BOOL preCreateLogic(WCHAR file_path_param[], WCHAR* full_app_path);

int preReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
);

int postReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
);

int preWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof, BOOL* mark_at_the_end,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
);

int postWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof, BOOL* mark_at_the_end,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
);


#endif //!LOGIC_H
