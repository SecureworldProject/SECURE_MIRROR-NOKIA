#ifndef LOGIC_H
#define LOGIC_H

/////  FILE INCLUDES  /////

#include "context.h"	// This include has to be on the top
#include <winnt.h>
#include <psapi.h>
#include "dokan/DokanFiles/dokan.h"
#include "main.h"

#include "dokan/wrapper_dokan.h"
#ifdef ENABLE_WINFSP
	#include "winfsp/wrapper_winfsp.h"
#endif // ENABLE_WINFSP




/////  DEFINITIONS  /////

#define MARK_LENGTH 512

/**
Structure that contains mark information in memory associated to a pair {file_path, app_path} to have context for writes and speed up reads.
Remarks:
 - Both file_path and app_path must be formatted with formatPath() before being inserted.
 - A special app_path when it could not be retrieved is "Memory Mapping".
 - FRN is a file-specific 4-byte number. It is used to enhance robustness of the ciphers against internal attacks based on file generation.
 - Mark level is the byte from a mark that indicates the ciphering level of a file (or buffers that come from or go into a file)
*/
struct FileMarkInfo {
	WCHAR file_path[MAX_PATH];		// The path to the file
	WCHAR app_path[MAX_PATH];		// The path of the application that used the file
	int8_t buffer_mark_lvl;			// The ciphering level of an application buffer		Values: -1, 0, 1 or UNKNOWN_MARK_LEVEL (unknown)
	uint32_t buffer_frn;			// The File Random Number of an application buffer	Values: Any number. INVALID_FRN means unknown/irrelevant
	int8_t file_mark_lvl;			// The ciphering level of a file					Values: -1, 0, 1 or UNKNOWN_MARK_LEVEL (unknown)
	uint32_t file_frn;				// The File Random Number of a file					Values: Any number. INVALID_FRN means unknown/irrelevant
	time_t last_closed;				// The time of the last cleanup operation.			Values: Any number. TIME_CURRENTLY_OPEN means currently open
};

#define UNKNOWN_MARK_LEVEL -128
#define INVALID_FRN 0
#define TIME_CURRENTLY_OPEN 0



/////  FUNCTION PROTOTYPES  /////

enum Operation operationAddition(enum Operation op1, enum Operation op2);
enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]);
DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path);

void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPCVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key, uint32_t frn);
void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPCVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key, uint32_t frn);
uint32_t createFRN();
void getRandom(uint8_t* buffer, int buf_size);

int8_t mark(uint8_t* input, int8_t level, uint32_t frn);
int8_t unmark(uint8_t* input, uint32_t* frn);

void testFMItable();
void printFMITable();
int purgeFMITable();
void threadPurge();


// Logic functions

BOOL preCreateLogic(WCHAR file_path_param[], WCHAR* full_app_path, ULONG pid);

int preReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, BOOL use_overlapped,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
);

int postReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
);

int preWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped, UCHAR write_to_eof,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
);

int postWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped, UCHAR write_to_eof,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
);

int postCleanupLogic(WCHAR* file_path, WCHAR* app_path);
#endif //!LOGIC_H
