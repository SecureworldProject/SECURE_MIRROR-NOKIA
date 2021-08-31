/*
SecureWorld file logic.c
Contains the pre and post logic functions for all operations.
Invokes functions like cypher(), decypher(), mark(), unmark(), etc

Nokia Febrero 2021
*/


/////  FILE INCLUDES  /////

#include "logic.h"
#include "keymaker.h"
#include <Lmcons.h>	// to get UNLEN
#include "huffman.h"




/////  DEFINITIONS  /////

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(N) ((N<0)?(-N):(N))

#define MARK_LENGTH 512
#define MAX_SIMULTANEOUS_DOWNLOADS 10		// OLD can be removed
#define FILE_MARK_INFO_TABLE_INITIAL_SIZE 256
#define FILE_MARK_INFO_TABLE_SIZE_INCREMENT 64


// This is a byte array of length 'MARK_LENGTH'
/*byte FILLING_SEQUENCE[] = {
	// Each line contains 8*8 = 64 zeros
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros ( 64)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (128)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (192)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (256)

	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (320)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (384)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (448)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0	// +64 zeros (512)
};*/

byte FILLING_SEQUENCE[] = {
	// Each line contains 8*8 = 64 bytes
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes ( 64)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (128)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (192)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (256)

	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (320)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (384)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M',	// +64 bytes (448)
	'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M', 'M','M','M','M','M','M','M','M'	// +64 bytes (512)
};


/*union UNION_UINT16 {
	struct {
		uint8_t low;
		uint8_t high;
	} part;
	uint16_t full;
};*/

WCHAR remote_marked_file_table[MAX_SIMULTANEOUS_DOWNLOADS][MAX_PATH] = { 0 };		// OLD can be removed

struct FileMarkInfo** file_mark_info_table = NULL;
size_t file_mark_info_table_size = 0;




/////  FUNCTION PROTOTYPES  /////
void removeFromTableOLD(WCHAR* file_path);
BOOL checkTableOLD(WCHAR* file_path);
void addToTableOLD(WCHAR* file_path);

struct FileMarkInfo* removeFMITableEntry(WCHAR* file_path);
struct FileMarkInfo* getFMITableEntry(WCHAR* file_path);
struct FileMarkInfo* addFMITableEntry(struct FileMarkInfo* file_mark_info);

struct FileMarkInfo* createFMI(WCHAR* file_path, int8_t orig_mark_lvl, int8_t curr_mark_lvl);	// Allocates memory
void destroyFMI(struct FileMarkInfo* file_mark_info);											// Frees memory




/////  FUNCTION IMPLEMENTATIONS  /////

enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]) {
	enum operation op = NOTHING;
	WCHAR* tmp_str = NULL;
	PRINT("Checking if path (%ws) is in a syncfolder\n", file_path);

	for (size_t i = 0; i < _msize(ctx.sync_folders) / sizeof(WCHAR*); i++) {
		PRINT1("Checking sync folder (%ws) \n", ctx.sync_folders[i]);
		tmp_str = wcsstr(file_path, ctx.sync_folders[i]);
		if (tmp_str != NULL && tmp_str == file_path) {
			// Match found
			PRINT("Match found - Irp op (%s) in syncfolder (%ws)\n", (irp_op == 0) ? "READ":"WRITE", file_path);
			if (irp_op == ON_READ) {
				op = DECIPHER;
			} else if (irp_op == ON_WRITE) {
				op = CIPHER;
			}
		}
	}
	return op;
}

enum Operation operationAddition(enum Operation op1, enum Operation op2) {
	switch (op1) {
		case NOTHING:
			switch (op2) {
				case NOTHING:
					return NOTHING;
				case CIPHER:
					return CIPHER;
				case DECIPHER:
					return DECIPHER;
				default:
					break;
			}
			break;
		case CIPHER:
			switch (op2) {
				case NOTHING:
					return CIPHER;
				case CIPHER:
					fprintf(stderr, "ERROR: case where chiphering has to be done on top of another ciphering is not allowed");
					return NOTHING;		// CIPHER 2 times. This should never happen
				case DECIPHER:
					return NOTHING;
				default:
					break;
			}
			break;
		case DECIPHER:
			switch (op2) {
				case NOTHING:
					return DECIPHER;
				case CIPHER:
					return NOTHING;
				case DECIPHER:
					fprintf(stderr, "ERROR: case where dechiphering has to be done on top of another deciphering is not allowed");
					return NOTHING;		// DECIPHER 2 times. This should never happen
				default:
					break;
			}
			break;
		default:
			break;
	}
	return NOTHING;
}

void removeFromTableOLD(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		if (0 == wcscmp(file_path, remote_marked_file_table[i])) {
			wcscpy(remote_marked_file_table[i], L"");
			return;
		}
	}
	return;
}

BOOL checkTableOLD(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		PRINT("Checking TABLE: %ws\n", remote_marked_file_table[i]);
		if (0 == wcscmp(file_path, remote_marked_file_table[i])) {
			return TRUE;
		}
	}
	return FALSE;
}

void addToTableOLD(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		if (0 == wcscmp(L"", remote_marked_file_table[i])) {
			wcscpy(remote_marked_file_table[i], file_path);
			return;
		}
	}
	return;
}

/**
* Removes from the the file_mark_info_table the struct FileMarkInfo* associated to the file_path passed as parameter.
* Note: the returned pointer should be freed when its use has finished.
*
* @param WCHAR* file_path
*		The file path associated to the struct FileMarkInfo* wanted to be removed.
*
* @return struct FileMarkInfo*
*		The (valid) pointer to the struct FileMarkInfo associated to the file_path passed as parameter. Remember to free after use.
**/
struct FileMarkInfo* removeFMITableEntry(WCHAR* file_path) {
	struct FileMarkInfo* result = NULL;

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL && 0 == wcscmp(file_path, file_mark_info_table[i]->file_path)) {
			result = file_mark_info_table[i];
			file_mark_info_table[i] = NULL;
			PRINT("FMI removed from the table\n");
			break;
		}
	}
	return result;
}

/**
* Finds the struct FileMarkInfo* associated to the file_path passed as parameter in the file_mark_info_table.
* Note: the returned pointer must not be freed in any case.
*
* @param WCHAR* file_path
*		The file path associated to the struct FileMarkInfo* wanted to be retrieved.
*
* @return struct FileMarkInfo*
*		The pointer to the struct FileMarkInfo associated to the file_path passed as parameter. Do not free on any circumstances.
**/
struct FileMarkInfo* getFMITableEntry(WCHAR* file_path) {
	struct FileMarkInfo* result = NULL;

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL && 0 == wcscmp(file_path, file_mark_info_table[i]->file_path)) {
			result = file_mark_info_table[i];
			PRINT("FMI found in the table\n");
			break;
		}
	}
	return result;
}

/**
* Adds the struct FileMarkInfo* passed as parameter in the file_mark_info_table.
* If the file_path specified in the parameter already exists in the table the value is overwritten, if not, the value is inserted on a free space.
* If there are not available slots for the struct to be added to the table, more space is allocated.
* In the case that no more space is allocable, NULL is returned.
* Note: the returned pointer must not be freed in any case.
*
* @param struct FileMarkInfo* file_mark_info
*		The struct FileMarkInfo* to add to the table.
*
* @return struct FileMarkInfo*
*		The struct FileMarkInfo* added to the table. Do not free on any circumstances. If an error occurs NULL is returned.
**/
struct FileMarkInfo* addFMITableEntry(struct FileMarkInfo* file_mark_info) {
	BOOL found_empty = FALSE;
	size_t empty_index = 0;
	struct FileMarkInfo* tmp_table = NULL;

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] == NULL) {
			if (!found_empty) {
				empty_index = i;
				found_empty = TRUE;
			}
		} else if (0 == wcscmp(file_mark_info->file_path, file_mark_info_table[i]->file_path)) {
			destroyFMI(file_mark_info_table[i]);
			file_mark_info_table[i] = file_mark_info;
			PRINT("FMI overwritten in the table\n");
			return file_mark_info;
		}
	}

	// If an empty slot was not found, extend the table and save the index to the first empty slot
	if (!found_empty) {
		size_t fmi_table_size_increment = (file_mark_info_table_size == 0) ? FILE_MARK_INFO_TABLE_INITIAL_SIZE : FILE_MARK_INFO_TABLE_SIZE_INCREMENT;
		tmp_table = realloc(file_mark_info_table, file_mark_info_table_size + fmi_table_size_increment);
		if (tmp_table == NULL) {
			printf("ERROR: allocating memory for file_mark_info_table.\n");
			return NULL;
		} else {
			// Save the first empty slot index
			empty_index = file_mark_info_table_size;

			// Update the table pointer and size, and set new slots to NULL
			file_mark_info_table = tmp_table;
			file_mark_info_table_size += fmi_table_size_increment;
			memset(&(file_mark_info_table[file_mark_info_table_size]), NULL, fmi_table_size_increment * sizeof(struct FileMarkInfo*));
		}
	}// At this point an empty slot always exist in the position empty_index

	// Assign parameter to the empty slot
	file_mark_info_table[empty_index] = file_mark_info;
	PRINT("FMI written in the table\n");

	return file_mark_info;
}

// Allocates memory
struct FileMarkInfo* createFMI(WCHAR* file_path, int8_t orig_mark_lvl, int8_t curr_mark_lvl) {
	if (file_path == NULL) {
		return NULL;
	}

	struct FileMarkInfo* fmi = NULL;
	fmi = malloc(1 * sizeof(struct FileMarkInfo));
	if (fmi != NULL) {
		wcscpy(fmi->file_path, file_path);
		fmi->orig_mark_lvl = orig_mark_lvl;
		fmi->curr_mark_lvl = curr_mark_lvl;
	}
	return fmi;
}

// Frees memory
void destroyFMI(struct FileMarkInfo* file_mark_info) {
	free(file_mark_info);
}


void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key) {
	PRINT("Calling cipher dll......\n");

	PRINT("dst_buf = %p, src_buf = %p, buf_size = %d\n", dst_buf, src_buf, buf_size);

	// FOR TESTING
	uint16_t current_value = 0;
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value + 1) % 255);
	}

	/*typedef int(__stdcall* cipher_func_type)(LPVOID, LPVOID, DWORD, size_t, struct KeyData*);

	cipher_func_type cipher_func;
	int result;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");
	if (cipher_func != NULL) {
		result = cipher_func(dst_buf, src_buf, buf_size, offset, composed_key);
		if (result != 0) {
			PRINT("WARNING: error \n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}*/

	PRINT("Done cipher dll......\n");
}

void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key) {
	PRINT("Calling decipher dll......\n");

	// FOR TESTING
	uint16_t current_value = 0;
	PRINT("\tbuf_size = %lu \n", buf_size);
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value - 1) % 255);
	}

	/*typedef int(__stdcall* decipher_func_type)(LPVOID, LPVOID, DWORD, size_t, struct KeyData*);

	decipher_func_type decipher_func;
	int result;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");
	if (decipher_func != NULL) {
		result = decipher_func(dst_buf, src_buf, buf_size, offset, composed_key);
		if (result != 0) {
			PRINT("WARNING: error \n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}*/

	PRINT("Done decipher dll......\n");
}


DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path) {
	BOOL opened = FALSE;
	DWORD error_code = ERROR_SUCCESS;

	// Ensure handle is valid (reopen the file if necessary)
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		PRINT("Invalid file handle!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		handle = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			error_code = GetLastError();
			PRINT("\tERROR creating handle to get file size (%d)\n", error_code);
			goto CLEANUP_GET_FILE_SIZE;
		}
		opened = TRUE;
	}

	// Maybe should check file_size > 0 (although that would mean that file_size > 8 EiB = 2^63 Bytes)
	if (!GetFileSizeEx(handle, file_size)) {
		error_code = GetLastError();
		PRINT("\tERROR: cannot get file size (%d)\n", error_code);
		goto CLEANUP_GET_FILE_SIZE;
	};

	CLEANUP_GET_FILE_SIZE:
	if (opened)
		CloseHandle(handle);

	return error_code;	// 0 = Success
}

/**
* Returns if the input buffer is marked.
* Assumes that the input buffer is long enough.
*
* @param uint8_t* input
*		The buffer to be checked.
*
* @return BOOL
*		TRUE if the buffer is marked. FALSE otherwise.
**/
BOOL checkMarkOLD(uint8_t* input) {
	uint16_t decompressed_length = ((uint16_t*)(input))[0];	//*(uint32_t*)&input[0];
	uint16_t compressed_length = ((uint16_t*)(input))[1];	//*(uint32_t*)&input[1];
	uint16_t header_bit_length = ((uint16_t*)(input))[2] + (6 /*HEADER_BASE_SIZE*/ << 3); //*(uint16_t*)&input[4] + (6 /*HEADER_BASE_SIZE*/ << 3);

	PRINT("Checking mark. decompressed_length=%u, compressed_length=%u, header_bit_length=%u (%ubytes)\n", decompressed_length, compressed_length, header_bit_length, header_bit_length/8 + ((header_bit_length%8)?0:1));

	if (decompressed_length != (uint16_t)MARK_LENGTH) {
		return FALSE;
	}

	if (compressed_length > MARK_LENGTH) {
		return FALSE;
	}

	if (memcmp(&(input[compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - compressed_length) != 0) {
		return FALSE;
	}

	return TRUE;
}

/**
* Marks the buffer if it is possible. Returns if the resulting input buffer was marked.
* Assumes that the input buffer is long enough and it is not marked.
*
* @param uint8_t* input
*		The buffer to be marked. In case it cannot be marked, it is not modified.
* @return BOOL
*		TRUE if the buffer was marked. FALSE if it could not.
**/
BOOL markOLD(uint8_t* input) {
	uint8_t* output = NULL;		// Allocated inside huffman_encode
	uint32_t total_compressed_length;

	PRINT("Trying to mark...\n");

	// Get first M bytes (stream) from file and compress them (resulting a compressed stream of C bytes).
	// Check if (C > M - 2), in that case, mark is omitted. Else:
	if (huffman_encode(input, &output, (uint32_t)MARK_LENGTH, &total_compressed_length) != 0 || total_compressed_length >= MARK_LENGTH) {
		free(output);
		PRINT("Could not be marked\n");
		return FALSE;
	}
	PRINT("Marked\n");

	/*// Substitute first 2 bytes with a codification of the C number
	union UNION_UINT16 dummy;
	dummy.full = compressed_length;
	input[0] = dummy.part.high;
	input[1] = dummy.part.low;*/

	// Then, copy compressed stream
	memcpy(input, output, total_compressed_length);

	// Fill the rest of the bytes until completing the M bytes with the filling sequence
	memcpy(&(input[total_compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - total_compressed_length);

	free(output);
	return TRUE;
}

/**
* Unmarks the buffer if it is possible. Returns if the resulting input buffer was unmarked.
* Assumes that the input buffer is long enough and it is marked.
*
* @param uint8_t* input
*		The buffer to be unmarked. In case it cannot be unmarked, it is not modified.
* @return BOOL
*		TRUE if the buffer was marked and has been unmarked. FALSE if it could not.
**/
BOOL unmarkOLD(uint8_t* input) {
	uint8_t* output = NULL;		// Allocated inside huffman_decode
	PRINT("Trying to unmark...\n");

	// Uncompress bytes the input
	if (huffman_decode(input, &output) != 0) {
		PRINT("Could not be unmarked\n");
		return FALSE;
	}
	PRINT("Unmarked\n");

	// Copy decoded buffer into input buffer
	memcpy(input, output, MARK_LENGTH);

	free(output);

	return TRUE;
}


/**
* Marks the buffer if it is possible with the specified mark level.
* If the input buffer cannot be marked, it is not modified and function returns INVALID_MARK_LEVEL.
* Assumes that the input buffer is long enough (at least MARK_LENGTH bytes) and it is not marked.
* Allows any mark level from -127 to 127, but not INVALID_MARK_LEVEL.
*
* @param uint8_t* input
*		The buffer to be marked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be marked, it is not modified.
* @param int8_t mark_level
*		The mark level to set in the mark. Its value cannot be INVALID_MARK_LEVEL.
*
* @return int8_t
*		'mark_level' if the buffer was marked. INVALID_MARK_LEVEL otherwise.
**/
int8_t mark(uint8_t* input, int8_t mark_level) {
	// Check valid mark level
	if (mark_level == INVALID_MARK_LEVEL) {
		return INVALID_MARK_LEVEL;
	}

	uint8_t* output = NULL;		// Allocated inside huffman_encode
	uint32_t total_compressed_length = 0;

	PRINT("Trying to mark...\n");

	// Compress the first MARK_LENGTH bytes of the stream and put them into the output buffer.
	// Mark is omitted if the compressed length is bigger or equal to MARK_LENGTH - 1 (this is due to the mark level needs one byte to be stored).
	if (huffman_encode(input, &output, (uint32_t)MARK_LENGTH, &total_compressed_length) != 0 || total_compressed_length >= MARK_LENGTH - 1) {
		free(output);
		PRINT("Could not be marked\n");
		return INVALID_MARK_LEVEL;
	}
	PRINT("Marked\n");

	// Set the mark level specified in the last byte
	input[MARK_LENGTH - 1] = mark_level;

	// Copy the compressed stream to the input buffer
	memcpy(input, output, total_compressed_length);

	// Fill the rest of the bytes with the filling sequence
	memcpy(&(input[total_compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - 1 - total_compressed_length); // -1 due to the mark level needs one byte to be stored

	// Free output buffer
	free(output);

	return mark_level;
}

/**
* Unmarks the buffer if it is possible and returns the mark level if the input buffer was marked.
* Allows any mark level from -127 to 127, but not INVALID_MARK_LEVEL.
*
* @param uint8_t* input
*		The buffer to be unmarked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be unmarked, it is not modified.
*
* @return int8_t
*		The level of the mark if the buffer is marked. INVALID_MARK_LEVEL otherwise.
**/
int8_t unmark(uint8_t* input) {
	uint16_t decompressed_length = ((uint16_t*)(input))[0];	//*(uint32_t*)&input[0];
	uint16_t compressed_length = ((uint16_t*)(input))[1];	//*(uint32_t*)&input[1];
	uint16_t header_bit_length = ((uint16_t*)(input))[2] + (6 /*HEADER_BASE_SIZE*/ << 3); //*(uint16_t*)&input[4] + (6 /*HEADER_BASE_SIZE*/ << 3);
	int8_t mark_level = (int8_t)input[MARK_LENGTH - 1];

	PRINT("Checking mark. decompressed_length=%u, compressed_length=%u, header_bit_length=%u (%u bytes)\n", decompressed_length, compressed_length, header_bit_length, header_bit_length / 8 + ((header_bit_length % 8) ? 0 : 1));

	if (mark_level == INVALID_MARK_LEVEL) {
		return INVALID_MARK_LEVEL;
	}

	if (decompressed_length != (uint16_t)MARK_LENGTH) {
		return INVALID_MARK_LEVEL;
	}

	if (compressed_length > MARK_LENGTH - 1) {	// -1 due to the mark level requires 1 Byte
		return INVALID_MARK_LEVEL;
	}

	if (memcmp(&(input[compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - 1 - compressed_length) != 0) {	// -1 due to the mark level requires 1 Byte
		return INVALID_MARK_LEVEL;
	}

	uint8_t* output = NULL;		// Allocated inside huffman_decode
	PRINT("Trying to unmark...\n");

	// Uncompress input
	if (huffman_decode(input, &output) != 0) {
		PRINT("Could not be unmarked\n");
		return INVALID_MARK_LEVEL;
	}
	PRINT("Unmarked\n");

	// Copy decoded buffer into input buffer
	memcpy(input, output, MARK_LENGTH);

	// Free output buffer
	free(output);

	return mark_level;	// Allows mark_level to be any number from -127 to 127 and the value INVALID_MARK_LEVEL=-128
}



BOOL preCreateLogic(WCHAR file_path_param[], WCHAR* full_app_path) {
	WCHAR* tmp_str;

	//PRINT("preCreateLogic!!\n");
	//PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld \n", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);
	WCHAR* user = NULL;
	DWORD user_size = 0;
	LPDWORD p_user_size = &user_size;
	struct App* app = NULL;
	int result = 0;
	BOOL block_access = FALSE;	// Allow by default

	// This is much slower but more secure
	//WCHAR* file_path = malloc(MAX_PATH * sizeof(WCHAR));
	//wcscpy(file_path, file_path_param);
	//formatPath(&file_path);
	// If we format the string like this, save it in PDOKAN_FILE_INFO DokanFileInfo->Context (so it is done only once per handle)

	// This is much faster but less secure
	WCHAR* file_path = file_path_param;
	size_t len = wcslen(file_path);
	if (file_path[len-1] == L'\\') {
		file_path[len-1] = L'\0';
	}

	// Check blocked apps
	app = getApp(full_app_path);
	if (app->type == BLOCKED) {
		return TRUE;	// Block access
	}

	// Check parental control
	for (size_t i = 0; i < _msize(ctx.parentals) / sizeof(struct ParentalControl*); i++) {
		//PRINT("comparing parental: %ws \n", ctx.parentals[i]->folder);
		tmp_str = wcsstr(file_path, ctx.parentals[i]->folder);
		if (tmp_str && tmp_str == file_path) {
			// Initialize user name if not done yet
			if (user == NULL){
				user_size = UNLEN + 1;
				user = malloc(sizeof(WCHAR) * user_size);
				if (user == NULL || GetUserNameW(user, p_user_size) == 0) {		// If the function fails, the return value is zero.
					PRINT("ERROR getting user name. Blocking access by default...\n");
					free(user);
					return TRUE;	// Block access
				} else {
					PRINT("UserName is: %ws\n", user);
				}
			}

			// Check if user name is allowed for this folder and challenges are correct.
			for (size_t j = 0; j < _msize(ctx.parentals[i]->users) / sizeof(WCHAR*); j++) {
				if (wcscmp(ctx.parentals[i]->users[j], user) == 0) {
					// The path is a parental controlled folder and the user matches, then check the challenges to block or allow.
					result = makeParentalKey(ctx.parentals[i]->challenge_groups, &block_access);
					if (result != 0) {
						fprintf(stderr, "ERROR in preCreateLogic (%d)", result);
						return TRUE;	// Something went wrong, block access (should never happen)
					}
					return block_access;
				}
			}

			// The path is a parental controlled folder but the user does not match with any of the ones with access, then block.
			//PRINT("returning TRUE for folder %ws\n", tmp_str);
			free(user);
			return TRUE;	// Block access
		}
	}

	// The path is not a parental controlled folder, then allow.
	return FALSE;	// Allow access
}

// This function allocates buffers to a memory size adjusted to the blocksize and other parameters. If blocksize is 0, allocates buffers of the same size.
int preReadLogic(
		uint64_t file_size, enum Operation op,
		LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
		LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
	) {

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_buffer_length == NULL || orig_read_length == NULL || *orig_read_length == NULL ||
		orig_offset == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("preReadLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n",
		file_size, op);
	PRINT(" - Orig buffer: %p\n - Orig buffer length: %lu\n - Orig bytes done: %lu\n - Orig offset: %lld\n",
		*orig_buffer, *orig_buffer_length, **orig_read_length, *orig_offset);
	/*PRINT(" - Aux buffer: %p\n - Aux buffer length: %lu\n - Aux bytes done: %lu\n - Aux offset: %lld\n",
		*aux_buffer, *aux_buffer_length, **aux_read_length, *aux_offset);*/


	BOOL small_file = file_size < MARK_LENGTH;

	*aux_buffer = *orig_buffer;
	*aux_read_length = *orig_read_length;

	// Check file size and buffer position/size and fix buffer limits in consequence
	if (small_file) {
		// Buffer starts and ends on same place
		*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
		*aux_buffer_length = *orig_buffer_length;		//aux_fin = orig_fin;		// Buffer ends in same place
		//mark_at_the_end = FALSE;						// Already false by default
	} else {	//file_size >= MARK_LENGTH
		if (*orig_offset >= MARK_LENGTH) {
			// Buffer starts and ends on same place
			*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
			*aux_buffer_length = *orig_buffer_length;		//aux_fin = orig_fin;		// Buffer ends in same place
		} else {	//inicio < MARK_LENGTH
			if (*orig_buffer_length + *orig_offset < MARK_LENGTH) {
				// Buffer starts on 0 and ends on MARK_LENGTH
				*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
				*aux_buffer_length = MARK_LENGTH;							//aux_fin = MARK_LENGTH;	// Buffer ends at MARK_LENGTH
			} else {	// fin >= MARK_LENGTH
				// Buffer starts on 0 and ends on same place
				*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
				*aux_buffer_length = *orig_buffer_length + *orig_offset;	//aux_fin = orig_fin;		// Buffer ends in same place
			}

			// Allocate space for the aux buffer
			*aux_buffer = malloc(*aux_buffer_length);
			if (*aux_buffer == NULL) {
				return ERROR_NOT_ENOUGH_MEMORY;
			}
		}
	}
	PRINT("Ending preReadLogic\n");

	return 0;
}

int postReadLogic(
		uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle,
		LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
		LPVOID*  aux_buffer, DWORD*  aux_buffer_length, LPDWORD*  aux_read_length, LONGLONG*  aux_offset
	){

	// Parameter checking
	if (orig_buffer == NULL || *orig_buffer == NULL || orig_buffer_length == NULL || orig_read_length == NULL || *orig_read_length == NULL ||
		orig_offset == NULL || protection == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("postReadLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - Protection: %p   (cipher->id: %s, key:%p)\n - Handle: %p\n",
		file_size, op, protection, protection->cipher->id, protection->key, handle);
	PRINT(" - Orig buffer: %p\n - Orig buffer length: %lu\n - Orig bytes done: %lu\n - Orig offset: %lld\n",
		*orig_buffer, *orig_buffer_length, **orig_read_length, *orig_offset);
	PRINT(" - Aux buffer: %p\n - Aux buffer length: %lu\n - Aux bytes done: %lu\n - Aux offset: %lld\n",
		*aux_buffer, *aux_buffer_length, **aux_read_length, *aux_offset);


	int result = 0;
	struct KeyData* composed_key = protection->key;

	BOOL marked = FALSE;
	BOOL mark_at_the_end = FALSE;
	BOOL small_file = file_size < MARK_LENGTH;

	LPVOID extra_read_buffer = NULL;
	DWORD extra_bytes_read = 0;
	DWORD error_code = 0;

	LPVOID aux_buffer_copy = NULL;
	LARGE_INTEGER distanceToMove = { 0 };

	// Remove file in the remote-marked-file-table if it is there
	removeFromTableOLD(file_path);

	// Read and Check mark if necessary
	if (!small_file) {
		if (*orig_offset >= MARK_LENGTH) {
			if (op == DECIPHER) {
				// Make handle point to file begin (distanceToMove = 0)
				if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
					error_code = GetLastError();
					PRINT(L"ERROR seeking in postRead (error=%lu)\n", error_code);
					//return error_code;
					goto POST_READ_CLEANUP;
				}

				// Allocate read buffer
				extra_read_buffer = malloc(MARK_LENGTH * sizeof(byte));
				if (extra_read_buffer == NULL) {
					error_code = ERROR_NOT_ENOUGH_MEMORY;
					goto POST_READ_CLEANUP;
				}

				// Read header of file
				if (!ReadFile(
					handle,
					extra_read_buffer,
					MARK_LENGTH,
					&extra_bytes_read,
					NULL)
					) {
					error_code = 0xFFFF;
					goto POST_READ_CLEANUP;
				}
				if (extra_bytes_read != MARK_LENGTH) {
					error_code = ERROR_CLUSTER_PARTIAL_READ;
					goto POST_READ_CLEANUP;
				}

				// Get if buffer is marked an unmark it
				marked = checkMarkOLD(extra_read_buffer);
				if (marked) {
					marked = unmarkOLD(extra_read_buffer);
				}
				free(extra_read_buffer);
				extra_read_buffer = NULL;
			}
		} else {	// Case where *orig_offset < MARK_LENGTH
			// Get if buffer is marked an unmark it
			marked = checkMarkOLD(*aux_buffer);
			if (marked) {
				marked = unmarkOLD(*aux_buffer);
			}
		}


		PRINT("The file %s marked\n", marked ? "IS" : "is NOT");

		// Nothing/Cipher/Decipher operations. Also in NOTHING case set to (re)mark if needed to leave as it was
		switch (op) {
			case NOTHING:
				mark_at_the_end = marked;	// Decide to mark or not to leave it as it was
				break;
			case CIPHER:	// IF marked unmark THEN cipher (no marking) ELSE cipher (marking)
				if (!marked && *orig_offset < MARK_LENGTH) {
					mark_at_the_end = TRUE;
				}
				result = makeComposedKey(protection->challenge_groups, composed_key);
				if (result == 0) {
					aux_buffer_copy = malloc(*aux_buffer_length);
					if (aux_buffer_copy == NULL) {
						error_code = ERROR_NOT_ENOUGH_MEMORY;
						goto POST_READ_CLEANUP;
					}
					memcpy(aux_buffer_copy, *aux_buffer, *aux_buffer_length);
					invokeCipher(protection->cipher, *aux_buffer, aux_buffer_copy, *aux_buffer_length, *aux_offset, composed_key);
					free(aux_buffer_copy);
					aux_buffer_copy = NULL;
				} else {
					fprintf(stderr, "ERROR composing key in postReadLogic (%d)", result);
					error_code = -1;
					goto POST_READ_CLEANUP;
				}
				break;
			case DECIPHER:	// IF marked unmark THEN decipher (omitting mark) ELSE (is cleartext) nothing
				result = makeComposedKey(protection->challenge_groups, composed_key);
				if (result == 0) {
					if (marked) {	// && !small_file   but this is always true for marked because marked is false by default
						aux_buffer_copy = malloc(*aux_buffer_length);
						if (aux_buffer_copy == NULL) {
							error_code = ERROR_NOT_ENOUGH_MEMORY;
							goto POST_READ_CLEANUP;
						}
						memcpy(aux_buffer_copy, *aux_buffer, *aux_buffer_length);
						invokeDecipher(protection->cipher, *aux_buffer, aux_buffer_copy, *aux_buffer_length, *aux_offset, composed_key);
						free(aux_buffer_copy);
						aux_buffer_copy = NULL;
					} // else --> do nothing, but copy buffer to orig at the end
				} else {
					fprintf(stderr, "ERROR composing key in postReadLogic (%d)", result);
					error_code = 0xFFFF;
					goto POST_READ_CLEANUP;
				}
				break;
			default:
				break;
		}
	}

	// Mark if needed
	if (!small_file && mark_at_the_end) {
		markOLD(*aux_buffer);
	}

	// Copy buffer aux to orig
	if (*orig_buffer != *aux_buffer) {
		memcpy(*orig_buffer, &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer_length);
	}

	// Copy aux_read_length to orig (the corresponding number, may be less)
	**orig_read_length = MIN(*orig_buffer_length, **aux_read_length - (*orig_offset - *aux_offset));

	// Make handle point to where it should based on the read the application asked to do
	distanceToMove.QuadPart = *orig_offset + **orig_read_length;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		PRINT(L"ERROR: SetFilePointerEx in postRead (error_code = %lu)\n", error_code);
		goto POST_READ_CLEANUP;
	}

	POST_READ_CLEANUP:
	if (extra_read_buffer != NULL) {
		free(extra_read_buffer);
	}
	if (aux_buffer_copy != NULL) {
		free(aux_buffer_copy);
	}

	return error_code;	// 0 = Success
}


int preWriteLogic(
		uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof, BOOL *mark_at_the_end,
		LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
		LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
	){

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL || aux_buffer==NULL || aux_bytes_to_write==NULL || aux_bytes_written==NULL || aux_offset==NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("preWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - Protection: %p   (cipher->id: %s, key:%p)\n - Handle: %p\n - Write_to_eof: %u\n",
		file_size, op, file_path, protection, protection->cipher->id, protection->key, handle, write_to_eof);
	PRINT(" - Orig buffer: %p\n - Orig bytes to write: %lu\n - Orig bytes written: %lu\n - Orig offset: %lld\n",
		*orig_buffer, *orig_bytes_to_write, **orig_bytes_written, *orig_offset);
	/*PRINT(" - Aux buffer: %p\n - Aux bytes to write: %lu\n - Aux bytes written: %lu\n - Aux offset: %lld\n",
		*aux_buffer, *aux_bytes_to_write, **aux_bytes_written, *aux_offset);*/

	// Create other temporal variables
	struct KeyData* composed_key = protection->key;
	int result = 0;

	//HANDLE read_handle = INVALID_HANDLE_VALUE;
	LPVOID read_buffer = NULL;
	DWORD bytes_read = 0;

	BOOL marked = FALSE;
	//BOOL *mark_at_the_end = FALSE;
	*mark_at_the_end = FALSE;
	BOOL small_file = FALSE;
	LARGE_INTEGER distanceToMove = { 0 };
	DWORD error_code = ERROR_SUCCESS;


	// Check file in the remote-marked-file-table
	BOOL marked_in_table = FALSE;
	marked_in_table = checkTableOLD(file_path);
	marked = marked_in_table;


	// Case when taking into account the mark in the buffer instead of in the file
	/*if (file_size == 0 && *orig_offset == 0 && *orig_bytes_to_write >= MARK_LENGTH) {
		PRINT("SPECIAL CASE --> Looking for mark in the buffer instead of in the file\n");
		*aux_offset = 0;	// = *orig_offset;			//aux_inicio = orig_inicio = 0;		// Buffer starts on same place (0)
		*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;				// Buffer ends in same place
		*aux_buffer = *orig_buffer;
		*aux_bytes_written = *orig_bytes_written;

		if (op != NOTHING) {
			// Virtual read (just a copy of first MARK_LENGTH bytes the writing buffer)
			read_buffer = malloc(MARK_LENGTH * sizeof(byte));
			if (read_buffer == NULL) {
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			memcpy(read_buffer, *orig_buffer, MARK_LENGTH);

			// Get if buffer is marked an unmark it
			marked = checkMarkOLD(read_buffer);
			if (marked) {
				marked = unmarkOLD(read_buffer);
			}

			// Allocate aux_buffer
			if (marked || op == CIPHER) {
				*aux_buffer = malloc(*aux_bytes_to_write);
				PRINT("Allocation for aux_buffer\n");
				if (*aux_buffer == NULL) {
					return ERROR_NOT_ENOUGH_MEMORY;
				}
			}
		}

		switch (op) {
			case NOTHING:
				break;
			case CIPHER:	// if marked == FALSE no need to use read_buffer, and then copy can be done in one
				if (marked) {
					// Cipher and copy first MARK_LENGTH bytes
					invokeCipher(protection->cipher, *aux_buffer, read_buffer, MARK_LENGTH, 0, composed_key);
					// Cipher and copy the rest of bytes after MARK_LENGTH position
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[MARK_LENGTH]), &(((byte*)*orig_buffer)[MARK_LENGTH]), *orig_bytes_to_write - MARK_LENGTH, MARK_LENGTH, composed_key);
				} else {
					// Cipher all and copy to *aux_buffer
					invokeCipher(protection->cipher, *aux_buffer, *orig_buffer, *orig_bytes_to_write, *orig_offset, composed_key);
					PRINT("Marking buffer in prewrite\n");
					markOLD(*aux_buffer);
				}
				//                              read_buf  +  &(((byte*)*orig_buf)[MARK_LENGTH])
				// { orig_buf }  ---unmark-->  { unmarked_buf + rest_of buf }  ---cipher-->  { ciphered_buf }  ---mark?-->  { ciphered_and_marked_buf }

				break;
			case DECIPHER:
				if (marked) {
					// Decipher and copy first MARK_LENGTH bytes
					invokeDecipher(protection->cipher, *aux_buffer, read_buffer, MARK_LENGTH, 0, composed_key);
					// Decipher and copy the rest of bytes after MARK_LENGTH position
					invokeDecipher(protection->cipher, &(((byte*)*aux_buffer)[MARK_LENGTH]), &(((byte*)*orig_buffer)[MARK_LENGTH]), *orig_bytes_to_write - MARK_LENGTH, MARK_LENGTH, composed_key);
				} // else --> NOTHING (*aux_buffer still pointing to *orig_buffer)
				break;
			default:
				break;
		}

		if (read_buffer != NULL) {
			free(read_buffer);
		}

		return 0;
	}*/

	// When writting to eof it is the same as the offset being the original file size
	if (write_to_eof) {
		*orig_offset = file_size;
		PRINT("Write to EOF translated to new orig offset: %lld\n", *orig_offset);
	}
	*aux_buffer = *orig_buffer;
	*aux_bytes_written = *orig_bytes_written;

	small_file = file_size < MARK_LENGTH;
	PRINT("The file %s small\n", small_file ? "IS" : "is NOT");

	// Check file size and buffer position/size and fix buffer limits in consequence
	if (small_file) {
		// Buffer starts and ends on same place (to do a normal write with no operative)
		*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
		*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;		// Buffer ends in same place
		//*mark_at_the_end = FALSE;						// Already false by default
		return error_code;
	}

	// From now onwards assume file is big enough to contain a mark --> file_size >= MARK_LENGTH
	if (*orig_offset >= MARK_LENGTH) {;
		// Buffer starts and ends on same place
		*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
		*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;		// Buffer ends in same place
	} else {	// orig_offset < MARK_LENGTH
		// Buffer starts on 0 and ends on the same place or MARK_LENGTH (whichever is higher)
		*aux_offset = 0;																//aux_inicio = 0;							// Buffer starts on file beginning
		*aux_bytes_to_write = MAX(MARK_LENGTH, *orig_bytes_to_write + *orig_offset);	//aux_fin = MAX(MARK_LENGTH, orig_fin);		// Buffer ends at MARK_LENGTH
		/*if (*orig_bytes_to_write + *orig_offset -1 < MARK_LENGTH) {
			// Buffer starts on 0 and ends on MARK_LENGTH
			*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
			*aux_bytes_to_write = MARK_LENGTH;							//aux_fin = MARK_LENGTH;	// Buffer ends at MARK_LENGTH
		} else {	// orig_end > MARK_LENGTH
			// Buffer starts on 0 and ends on same place
			*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
			*aux_bytes_to_write = *orig_bytes_to_write + *orig_offset;	//aux_fin = orig_fin;		// Buffer ends in same place
		}*/

		// Allocate space for the aux buffer
		*aux_buffer = malloc(*aux_bytes_to_write);
		PRINT("Allocation for aux_buffer\n");
		if (*aux_buffer == NULL) {
			return ERROR_NOT_ENOUGH_MEMORY;
		}
	}


	/*if (handle || handle != INVALID_HANDLE_VALUE) {
		CloseHandle(handle);
	}
	handle = CreateFileW(
		file_path,				// Name of the file
		GENERIC_READ + GENERIC_WRITE,	// Open for read/write
		FILE_SHARE_READ + FILE_SHARE_WRITE,		// Do not share
		NULL,					// Default security
		OPEN_EXISTING,			// Open existing file only
		FILE_ATTRIBUTE_NORMAL,	// Normal file
		NULL);					// No attr. template

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		printf("ERROR: opening read_handle.\n");
		error_code = ERROR_OPEN_FAILED;
		goto READ_INSIDE_WRITE_CLEANUP;
	}*/

	// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		PRINT("ERROR handle seeking (error=%lu)\n", error_code);
		goto READ_INSIDE_WRITE_CLEANUP;
	}

	// Allocate read buffer
	read_buffer = malloc(MARK_LENGTH * sizeof(byte));
	if (read_buffer == NULL) {
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto READ_INSIDE_WRITE_CLEANUP;
	}

	// Read header of file
	if (!ReadFile(
		handle,
		read_buffer,
		MARK_LENGTH,
		&bytes_read,
		NULL)
		) {
		printf("ERROR reading mark inside preWrite!!!\n");
		error_code = ERROR_READ_FAULT;
		goto READ_INSIDE_WRITE_CLEANUP;
	}
	if (bytes_read != MARK_LENGTH) {
		error_code = ERROR_READ_FAULT;
		goto READ_INSIDE_WRITE_CLEANUP;
	}

	if (!marked_in_table) {
		// Get if buffer is marked an unmark it
		marked = checkMarkOLD(read_buffer);
		if (marked) {
			marked = unmarkOLD(read_buffer);
		}
	}

	READ_INSIDE_WRITE_CLEANUP:
	/*if (handle && handle != INVALID_HANDLE_VALUE) {
		CloseHandle(handle);
	}*/
	if (error_code != ERROR_SUCCESS) {
		free(read_buffer);
		return error_code;
	}


	PRINT("The file %s marked\n", marked ? "IS" : "is NOT");

	// Nothing/Cipher/Decipher operations. Also in NOTHING case set to (re)mark if needed to leave as it was
	switch (op) {
		case NOTHING:
			// If marcado
			//	If escribimos dentro de la marca:
			//		Poner mark_at_the_end a true
			//		Copiar de read_buf a aux_buf
			//	Copiar de orig_buf a aux_buf
			// Else (no marcado)
			//	Deshacer cosas raras y dejar un write normal
			PRINT("case NOTHING:\n");
			if (marked) {
				PRINT("if marked:\n");
				if (*orig_offset < MARK_LENGTH) {
					*mark_at_the_end = TRUE;
					memcpy(*aux_buffer, read_buffer, MARK_LENGTH);
				}
				// If *orig_offset < MARK_LENGTH	--> *aux_offset = 0				--> Copy orig_buffer into aux_buffer with an offset of orig_offset
				// If *orig_offset >= MARK_LENGTH	--> *aux_offset = *orig_offset	--> Copy orig_buffer to aux_buffer (same size)
				memcpy(&(((byte*)*aux_buffer)[*orig_offset - *aux_offset]), *orig_buffer, *orig_bytes_to_write);
			} else {
				PRINT("else (no marked):\n");
				if (*orig_offset < MARK_LENGTH) {	// Otherwise *aux_buffer is not allocated
					free(*aux_buffer);
				}
				PRINT("after free\n");
				*aux_buffer = *orig_buffer;
				*aux_bytes_to_write = *orig_bytes_to_write;
				*aux_offset = *orig_offset;
			}
			PRINT("going to break\n");
			break;
		case CIPHER:	// IF marked unmark THEN cipher (no marking) ELSE cipher (marking)
			if (!marked && *orig_offset < MARK_LENGTH) {
				*mark_at_the_end = TRUE;
			}
			// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (read_buffer != NULL) {// && bytes_read>=512) {
				PRINT_HEX(read_buffer, bytes_read);
			}
			if (*aux_offset == 0) {
				memcpy(*aux_buffer, read_buffer, bytes_read);
			}
			if (*aux_buffer != NULL) {// && *aux_bytes_to_write >= 1024) {
				PRINT_HEX(*aux_buffer, *aux_bytes_to_write);
			}
			if (*orig_buffer != NULL) {// && *orig_bytes_to_write >= 1024) {
				PRINT_HEX(*orig_buffer, *orig_bytes_to_write);
			}
			// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				PRINT("ANTES: *orig_buffer = %.*s\n", *orig_bytes_to_write, (char*)*orig_buffer);
				PRINT_HEX(*orig_buffer, *orig_bytes_to_write);
				// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (*aux_offset == 0 || !(*orig_buffer == &(((byte*)*aux_buffer)[*orig_offset - *aux_offset]))) {
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset - *aux_offset]), *orig_buffer, *orig_bytes_to_write, *orig_offset, composed_key);
				} else {
					LPVOID orig_buffer_copy = malloc(*orig_bytes_to_write);
					if (orig_buffer_copy == NULL) {
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					memcpy(orig_buffer_copy, *orig_buffer, *orig_bytes_to_write);
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset - *aux_offset]), orig_buffer_copy, *orig_bytes_to_write, *orig_offset, composed_key);
					free(orig_buffer_copy);
				}
				// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				PRINT("DESPUES: *aux_buffer = %.*s\n", *aux_bytes_to_write, (char*)*aux_buffer);
				PRINT_HEX(*aux_buffer, *aux_bytes_to_write);
				// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			} else {
				fprintf(stderr, "ERROR composing key in preWriteLogic (%d)", result);
				return result;
			}
			break;
		case DECIPHER:	// IF marked unmark THEN decipher (omitting mark) ELSE (is cleartext) nothing
			PRINT("case DECIPHER:\n");
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				// If marked:
				//	If escribimos dentro de la marca:
				//		Copiar descifrando de read_buf a aux_buf
				//	Copiar descifrando de orig_buf a aux_buf
				// Else (no marcado)
				//	Deshacer cosas raras y dejar un write normal
				if (marked) {
					PRINT("if marked:\n");
					if (*orig_offset < MARK_LENGTH) {
						invokeDecipher(protection->cipher, *aux_buffer, read_buffer, bytes_read, 0, composed_key);
					}
					// If *orig_offset < MARK_LENGTH	--> *aux_offset = 0				--> Copy orig_buffer into aux_buffer with an offset of orig_offset
					// If *orig_offset >= MARK_LENGTH	--> *aux_offset = *orig_offset	--> Copy orig_buffer to aux_buffer (same size)
					invokeDecipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset - *aux_offset]), *orig_buffer, *orig_bytes_to_write, *orig_offset, composed_key);
				} else {
					PRINT("else (no marked):\n");
					if (*orig_offset < MARK_LENGTH) {	// Otherwise *aux_buffer is not allocated
						free(*aux_buffer);
					}
					PRINT("after free\n");
					*aux_buffer = *orig_buffer;
					*aux_bytes_to_write = *orig_bytes_to_write;
					*aux_offset = *orig_offset;
				}
			} else {
				fprintf(stderr, "ERROR composing key in preWriteLogic (%d)", result);
				return result;
			}
			PRINT("going to break\n");
			break;
		default:
			break;
	}

	PRINT("will free READ BUFFER\n");
	// Free the read buffer
	free(read_buffer);

	// Mark if needed
	if (*mark_at_the_end) {
		PRINT("Marking buffer in prewrite\n");
		markOLD(*aux_buffer);
	}

	return error_code;
}

int postWriteLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof, BOOL *mark_at_the_end,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
) {
	DWORD error_code = ERROR_SUCCESS;
	uint64_t new_file_size = 0;
	LPVOID buffer1 = NULL;
	LPVOID buffer2 = NULL;
	LPVOID buffer3 = NULL;
	struct KeyData* composed_key = protection->key;
	int result = 0;

	DWORD bytes_done = 0;
	LARGE_INTEGER distanceToMove = { 0 };
	BOOL marked = FALSE;

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("postWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - Protection: %p   (cipher->id: %s, key:%p)\n - Handle: %p\n - write_to_eof: %u\n",
		file_size, op, file_path, protection, protection->cipher->id, protection->key, handle, write_to_eof);
	PRINT(" - Orig buffer: %p\n - Orig bytes to write: %lu\n - Orig bytes written: %lu\n - Orig offset: %lld\n",
		*orig_buffer, *orig_bytes_to_write, **orig_bytes_written, *orig_offset);
	PRINT(" - Aux buffer: %p\n - Aux bytes to write: %lu\n - Aux bytes written: %lu\n - Aux offset: %lld\n",
		*aux_buffer, *aux_bytes_to_write, **aux_bytes_written, *aux_offset);


	// This should work for every case
	**orig_bytes_written = MIN(*orig_bytes_to_write, **aux_bytes_written - (*orig_offset - *aux_offset));
	PRINT("Result 'bytes written' given to the application: %lu\n", **orig_bytes_written);


	// Get the new file size
	error_code = getFileSize(&new_file_size, handle, file_path);
	if (error_code != 0) {
		PRINT("ERROR getting file size in postWrite (error_code = %d)\n", error_code);
		return error_code;
	}

	// Check files that become larger or smaller than MARK_LENGTH
	if (file_size < MARK_LENGTH && new_file_size >= MARK_LENGTH && op != NOTHING) {
		PRINT("File has grown enough to admit mark (prev file_size = %llu, new file_size = %llu)\n", file_size, new_file_size);

		// Create new handle
		/*CloseHandle(handle);
		handle = CreateFileW(
			file_path,				// Name of the file
			GENERIC_READ + GENERIC_WRITE,	// Open for read/write
			0,						// Do not share
			NULL,					// Default security
			OPEN_EXISTING,			// Open existing file only
			FILE_ATTRIBUTE_NORMAL,	// Normal file
			NULL);					// No attr. template

		if (!handle || handle == INVALID_HANDLE_VALUE) {
			error_code = GetLastError();
			printf("ERROR in postWrite: opening handle (error = %lu)\n", error_code);
			goto POST_WRITE_CLEANUP;
		}*/

		// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
		if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
			error_code = GetLastError();
			PRINT("ERROR handle seeking in postWrite (error=%lu)\n", error_code);
			goto POST_WRITE_CLEANUP;
		}

		// Allocate read buffer
		buffer1 = malloc(MARK_LENGTH * sizeof(byte));
		if (buffer1 == NULL) {
			error_code = ERROR_NOT_ENOUGH_MEMORY;
			goto POST_WRITE_CLEANUP;
		}

		// Read first part of the file (MARK_LENGTH bytes)
		if (!ReadFile(
			handle,
			buffer1,
			MARK_LENGTH,
			&bytes_done,
			NULL)
			) {
			printf("ERROR reading mark in postWrite!!!\n");
			error_code = ERROR_READ_FAULT;
			goto POST_WRITE_CLEANUP;
		}
		if (bytes_done != MARK_LENGTH) {
			error_code = ERROR_READ_FAULT;
			goto POST_WRITE_CLEANUP;
		}

		// Check mark and unmark the content
		marked = checkMarkOLD(buffer1);
		if (marked) {
			marked = unmarkOLD(buffer1);
		}

		// If the operation was DECIPHER and the file was marked:
		//		Read the rest of the file
		//		Decipher the content
		//		Write new content to file
		//		Add file to table
		if ((op == DECIPHER && marked) || (op == CIPHER)) {
			// Allocate buffer for reading the rest of the file and another one for writting everything (in 1 operation instead of 2)
			buffer2 = malloc((new_file_size - MARK_LENGTH) * sizeof(byte));
			if (buffer2 == NULL) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				goto POST_WRITE_CLEANUP;
			}
			buffer3 = malloc(new_file_size * sizeof(byte));
			if (buffer3 == NULL) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				goto POST_WRITE_CLEANUP;
			}

			// Read the rest of file (no need to update position of handle because we have already read first MARK_LENGTH bytes, so it is pointing correctly)
			if (!ReadFile(
				handle,
				buffer2,
				(new_file_size - MARK_LENGTH),
				&bytes_done,
				NULL)
				) {
				printf("ERROR reading mark in postWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto POST_WRITE_CLEANUP;
			}
			if (bytes_done != (new_file_size - MARK_LENGTH)) {
				error_code = ERROR_READ_FAULT;
				goto POST_WRITE_CLEANUP;
			}

			// Make the composed key if necessary
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				if (op == DECIPHER) {
					// Decipher all the current content of the file
					invokeDecipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key);
					invokeDecipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key);
					// Add file to table
					addToTableOLD(file_path);	// Is it needed to change the table to save also unmarked files? No, because mark only modifies decipher operative
					PRINT("ADDED TO TABLE: %ws\n", file_path);
				} else if (op == CIPHER) {
					// Cipher all the current content of the file
					invokeCipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key);
					invokeCipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key);
					if (!marked) {
						markOLD(buffer3);	// if it fails, we can do nothing but leave it unmarked
					}
				}
			} else {
				fprintf(stderr, "ERROR composing key in postWriteLogic (%d)", result);
				error_code = result;
				goto POST_WRITE_CLEANUP;
			}

			// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
			if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				PRINT("ERROR handle seeking in postWrite (error=%lu)\n", error_code);
				goto POST_WRITE_CLEANUP;
			}

			// Write new content to file
			if (!WriteFile(
				handle,
				buffer3,
				new_file_size,
				&bytes_done,
				NULL)
				) {
				printf("ERROR writing mark in postWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto POST_WRITE_CLEANUP;
			}
			if (bytes_done != new_file_size) {
				error_code = ERROR_READ_FAULT;
				goto POST_WRITE_CLEANUP;
			}
		}
		POST_WRITE_CLEANUP:
		/*if (handle && handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
		}*/
		if (buffer1 != NULL) {
			free(buffer1);
		}
		if (buffer2 != NULL) {
			free(buffer2);
		}
		if (buffer3 != NULL) {
			free(buffer3);
		}
		if (error_code != ERROR_SUCCESS) {
			return error_code;
		}
	} else {
		PRINT("File as already big or it is still small (prev file_size = %llu, new file_size = %llu)\n", file_size, new_file_size);
	}

	// Make handle point to where it should based on the read the application asked to do
	distanceToMove.QuadPart = *orig_offset + **orig_bytes_written;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		PRINT("ERROR: SetFilePointerEx in postWrite(error_code = %lu)\n", error_code);
	}

	return error_code;
}


/**
* Marks, checks and unmarks a predefined buffer and prints it in the screen to see if mark and internal huffman functions are working well.
**/
void testMark(){
	PRINT("MARK TEST STARTS\n");
	uint8_t* orig_buffer = NULL;
	uint8_t* result_buffer = NULL;
	BOOL marked = FALSE;

	orig_buffer = calloc(520, sizeof(uint8_t));
	if (orig_buffer == NULL) {
		PRINT("ERROR!!\n");
		goto LABEL_END;
	}

	result_buffer = calloc(520, sizeof(uint8_t));
	if (result_buffer == NULL) {
		PRINT("ERROR!!\n");
		goto LABEL_END;
	}

	// FILL ORIG_BUFFER WITH SOMETHING
	//char data[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char data[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa5555555555555555aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	//char data[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa11111111111111111111111111111111aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa11111111111111111111111111111111aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa11111111111111111111111111111111aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa11111111111111111111111111111111________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAA________AAAAAAAAuuuuuuuuuuuuuuuuuuuuuuuu";
	memcpy(orig_buffer, data, MIN(strlen(data), 520));

	memcpy(result_buffer, orig_buffer, 520);

	PRINT_HEX(orig_buffer, 520);
	marked = markOLD(result_buffer);
	PRINT("The buffer %s marked\n", (marked ? "HAS BEEN" : "has NOT been"));
	PRINT_HEX(result_buffer, 520);

	if (!marked) {
		goto LABEL_END;
	}

	PRINT("-----------------------------\n");

	marked = FALSE;
	marked = checkMarkOLD(result_buffer);
	if (marked) {
		marked = unmarkOLD(result_buffer);
	}

	PRINT("The buffer %s unmarked\n", (marked ? "HAS BEEN" : "has NOT been"));
	PRINT_HEX(result_buffer, 520);

	PRINT("The result %s equal to the original\n", (0==memcmp(orig_buffer, result_buffer, 520) ? "IS" : "is NOT"));

	LABEL_END:
	free(orig_buffer);
	free(result_buffer);

	PRINT("MARK TEST ENDS\n");
}
