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
#define MAX_SIMULTANEOUS_DOWNLOADS 10

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


WCHAR remote_marked_file_table[MAX_SIMULTANEOUS_DOWNLOADS][MAX_PATH] = { 0 };


/////  FUNCTION PROTOTYPES  /////
void removeFromTable(WCHAR* file_path);
BOOL checkTable(WCHAR* file_path);
void addToTable(WCHAR* file_path);

void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, struct KeyData* composed_key);
void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, struct KeyData* composed_key);

BOOL checkMark(uint8_t* input);
BOOL mark(uint8_t* input);
BOOL unmark(uint8_t* input);




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

void removeFromTable(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		if (wcscmp(file_path, remote_marked_file_table[i])) {
			wcscpy(remote_marked_file_table[i], L"");
			return;
		}
	}
	return;
}

BOOL checkTable(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		if (wcscmp(file_path, remote_marked_file_table[i])) {
			return TRUE;
		}
	}
	return FALSE;
}

void addToTable(WCHAR* file_path) {
	for (size_t i = 0; i < MAX_SIMULTANEOUS_DOWNLOADS; i++) {
		if (wcscmp(L"", remote_marked_file_table[i])) {
			wcscpy(remote_marked_file_table[i], file_path);
			return;
		}
	}
	return;
}


void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, struct KeyData* composed_key) {
	PRINT("Calling cipher dll......\n");

	PRINT("dst_buf = %p, src_buf = %p, buf_size = %d\n", dst_buf, src_buf, buf_size);

	// FOR TESTING
	uint16_t current_value = 0;
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value + 1) % 255);
	}

	/*typedef int(__stdcall* cipher_func_type)(LPVOID, LPVOID, DWORD, struct KeyData*);

	cipher_func_type cipher_func;
	int result;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");
	if (cipher_func != NULL) {
		result = cipher_func(dst_buf, src_buf, buf_size, composed_key);
		if (result != 0) {
			PRINT("WARNING: error \n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}*/

	PRINT("Done cipher dll......\n");
}

void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPVOID src_buf, DWORD buf_size, struct KeyData* composed_key) {
	PRINT("Calling decipher dll......\n");

	// FOR TESTING
	uint16_t current_value = 0;
	PRINT("\tbuf_size = %lu \n", buf_size);
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value - 1) % 255);
	}

	/*typedef int(__stdcall* decipher_func_type)(LPVOID, LPVOID, DWORD, struct KeyData*);

	decipher_func_type decipher_func;
	int result;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");
	if (decipher_func != NULL) {
		result = decipher_func(dst_buf, src_buf, buf_size, composed_key);
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
	DWORD error_code = 0;

	// Ensure handle is valid (reopen the file if necessary)
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		PRINT("Invalid file handle\n");
		handle = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			error_code = GetLastError();
			PRINT("\tERROR creating handle to get file size (%d)\n", error_code);
			return error_code;
		}
		opened = TRUE;
	}

	// Maybe should check file_size > 0 (although that would mean that file_size > 8 EiB = 2^63 Bytes)
	if (!GetFileSizeEx(handle, file_size)) {
		error_code = GetLastError();
		PRINT("\tERROR: cannot get file size (%d)\n", error_code);
		if (opened)
			CloseHandle(handle);
		return error_code;
	};

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
BOOL checkMark(uint8_t* input) {

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
BOOL mark(uint8_t* input) {
	uint8_t* output;
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
BOOL unmark(uint8_t* input) {
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

	// Remove file in the remote-marked-file-table if it is there
	removeFromTable(file_path);

	// Read and Check mark if necessary
	if (!small_file) {
		if (*orig_offset >= MARK_LENGTH) {
			if (op == DECIPHER) {
				// TO DO create new handle
				// TO DO adjust offset of handle to position 0
				PRINT("TO DO!!!!! create handle and adjust its offset to read correctly");

				LARGE_INTEGER distanceToMove;
				distanceToMove.QuadPart = 0;
				if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
					error_code = GetLastError();
					PRINT(L"ERROR en seek. error=%lu\n", error_code);
					//return error_code;
					goto PRE_READ_CLEANUP;
				}

				// Allocate read buffer
				extra_read_buffer = malloc(MARK_LENGTH * sizeof(byte));
				if (extra_read_buffer == NULL) {
					error_code = ERROR_NOT_ENOUGH_MEMORY;
					goto PRE_READ_CLEANUP;
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
					goto PRE_READ_CLEANUP;
				}
				if (extra_bytes_read != MARK_LENGTH) {
					error_code = ERROR_CLUSTER_PARTIAL_READ;
					goto PRE_READ_CLEANUP;
				}

				// Get if buffer is marked an unmark it
				marked = checkMark(extra_read_buffer);
				if (marked) {
					marked = unmark(extra_read_buffer);
				}
				free(extra_read_buffer);
				extra_read_buffer = NULL;
			}
		} else {	// Case where *orig_offset < MARK_LENGTH
			// Get if buffer is marked an unmark it
			marked = checkMark(*aux_buffer);
			if (marked) {
				marked = unmark(*aux_buffer);
			}
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
					goto PRE_READ_CLEANUP;
				}
				memcpy(aux_buffer_copy, *aux_buffer, *aux_buffer_length);
				invokeCipher(protection->cipher, *aux_buffer, aux_buffer_copy, *aux_buffer_length, composed_key);
				free(aux_buffer_copy);
				aux_buffer_copy = NULL;
			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
				error_code = -1;
				goto PRE_READ_CLEANUP;
			}
			break;
		case DECIPHER:	// IF marked unmark THEN decipher (omitting mark) ELSE (is cleartext) nothing
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				if (marked) {	// && !small_file   but this is always true for marked because marked is false by default
					aux_buffer_copy = malloc(*aux_buffer_length);
					if (aux_buffer_copy == NULL) {
						error_code = ERROR_NOT_ENOUGH_MEMORY;
						goto PRE_READ_CLEANUP;
					}
					memcpy(aux_buffer_copy, *aux_buffer, *aux_buffer_length);
					invokeDecipher(protection->cipher, *aux_buffer, aux_buffer_copy, *aux_buffer_length, composed_key);
					free(aux_buffer_copy);
					aux_buffer_copy = NULL;
				} // else --> do nothing, but copy buffer to orig at the end
			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
				error_code = 0xFFFF;
				goto PRE_READ_CLEANUP;
			}
			break;
		default:
			break;
	}

	// Mark if needed
	if (!small_file && mark_at_the_end) {
		mark(*aux_buffer);
	}

	// Copy buffer aux to orig
	if (*orig_buffer != *aux_buffer) {
		memcpy(*orig_buffer, &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer_length);
	}

	// Copy aux_read_length to orig (the corresponding number, may be less)
	**orig_read_length = MIN(*orig_buffer_length, **aux_read_length - (*orig_offset - *aux_offset));

	PRE_READ_CLEANUP:
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

	HANDLE read_handle = INVALID_HANDLE_VALUE;
	LPVOID read_buffer = NULL;
	DWORD bytes_read = 0;

	BOOL marked = FALSE;
	//BOOL *mark_at_the_end = FALSE;
	*mark_at_the_end = FALSE;
	BOOL small_file = FALSE;


	// Check file in the remote-marked-file-table
	BOOL marked_in_table = FALSE;
	marked_in_table = checkTable(file_path);
	marked = marked_in_table;


	// Case when taking into account the mark in the buffer instead of in the file
	if (file_size == 0 && *orig_offset == 0 && *orig_bytes_to_write >= MARK_LENGTH) {
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
			marked = checkMark(read_buffer);
			if (marked) {
				marked = unmark(read_buffer);
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
					invokeCipher(protection->cipher, *aux_buffer, read_buffer, MARK_LENGTH, composed_key);
					// Cipher and copy the rest of bytes after MARK_LENGTH position
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[MARK_LENGTH]), &(((byte*)*orig_buffer)[MARK_LENGTH]), *orig_bytes_to_write - MARK_LENGTH, composed_key);
				} else {
					// Cipher all and copy to *aux_buffer
					invokeCipher(protection->cipher, *aux_buffer, *orig_buffer, *orig_bytes_to_write, composed_key);
					PRINT("Marking buffer in prewrite\n");
					mark(*aux_buffer);
				}
				/*LPVOID tmp_buffer = malloc(*orig_bytes_to_write);
				if (tmp_buffer == NULL) {
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				memcpy(tmp_buffer, read_buffer, MARK_LENGTH);													// Copy first MARK_LENGTH bytes
				memcpy(&(((byte*)tmp_buffer)[MARK_LENGTH]), read_buffer, *orig_bytes_to_write - MARK_LENGTH);	// Copy the rest of bytes after MARK_LENGTH position
				invokeCipher(protection->cipher, *aux_buffer, tmp_buffer, *orig_bytes_to_write, composed_key);	// Cipher all copying to *aux_buffer
				free(tmp_buffer);*/

				//                              read_buf  +  &(((byte*)*orig_buf)[MARK_LENGTH])
				// { orig_buf }  ---unmark-->  { unmarked_buf + rest_of buf }  ---cipher-->  { ciphered_buf }  ---mark?-->  { ciphered_and_marked_buf }

				break;
			case DECIPHER:
				if (marked) {
					// Decipher and copy first MARK_LENGTH bytes
					invokeDecipher(protection->cipher, *aux_buffer, read_buffer, MARK_LENGTH, composed_key);
					// Decipher and copy the rest of bytes after MARK_LENGTH position
					invokeDecipher(protection->cipher, &(((byte*)*aux_buffer)[MARK_LENGTH]), &(((byte*)*orig_buffer)[MARK_LENGTH]), *orig_bytes_to_write - MARK_LENGTH, composed_key);
				} // else --> NOTHING (*aux_buffer still pointing to *orig_buffer)
				break;
			default:
				break;
		}

		if (read_buffer != NULL) {
			free(read_buffer);
		}

		return 0;
	}

	// When writting to eof it is the same as the offset being the original file size
	if (write_to_eof) {
		*orig_offset = file_size;
		PRINT("Write to EOF translated to new orig offset: %lld\n", *orig_offset);
	}
	*aux_buffer = *orig_buffer;
	*aux_bytes_written = *orig_bytes_written;

	small_file = file_size < MARK_LENGTH;

	// Check file size and buffer position/size and fix buffer limits in consequence
	if (small_file) {
		// Buffer starts and ends on same place
		*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
		*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;		// Buffer ends in same place
		//*mark_at_the_end = FALSE;						// Already false by default
	} else {	// file_size >= MARK_LENGTH
		if (*orig_offset >= MARK_LENGTH) {
			// Buffer starts and ends on same place
			*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
			*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;		// Buffer ends in same place
		} else {	// orig_start < MARK_LENGTH
			// Buffer starts on 0 and ends on the same place or MARK_LENGTH (whichever is higher)
			*aux_offset = 0;																//aux_inicio = 0;							// Buffer starts on file beginning
			*aux_bytes_to_write = MAX(MARK_LENGTH, *orig_bytes_to_write + *orig_offset);	//aux_fin = MAX(MARK_LENGTH, orig_inicio);	// Buffer ends at MARK_LENGTH
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
	}

	// Read and Check mark if necessary
	if (!small_file) {
		// TO DO create new handle
		// TO DO adjust offset of handle to position 0

		DWORD error_code = ERROR_SUCCESS;

		PRINT("TO DO!!!!! create handle and adjust its offset to read correctly\n");
		CloseHandle(handle);
		read_handle = CreateFileW(
			file_path,				// Name of the file
			GENERIC_READ + GENERIC_WRITE,	// Open for read/write
			0,						// Do not share
			NULL,					// Default security
			OPEN_EXISTING,			// Open existing file only
			FILE_ATTRIBUTE_NORMAL,	// Normal file
			NULL);					// No attr. template

		if (!read_handle || read_handle == INVALID_HANDLE_VALUE) {
			printf("ERROR: opening read_handle.\n");
			error_code = ERROR_OPEN_FAILED;
			goto LABEL_CLOSE_HANDLE;
		}
		handle = read_handle;

		LARGE_INTEGER distanceToMove = { 0 };
		//distanceToMove.QuadPart = 0;
		if (!SetFilePointerEx(read_handle, distanceToMove, NULL, FILE_BEGIN)) {
			error_code = GetLastError();
			PRINT("ERROR handle seeking (error=%lu)\n", error_code);
			goto LABEL_CLOSE_HANDLE;
		}

		// Allocate read buffer
		read_buffer = malloc(MARK_LENGTH * sizeof(byte));
		if (read_buffer == NULL) {
			error_code = ERROR_NOT_ENOUGH_MEMORY;
			goto LABEL_CLOSE_HANDLE;
		}

		// Read header of file
		if (!ReadFile(
			read_handle,
			read_buffer,
			MARK_LENGTH,
			&bytes_read,
			NULL)
			) {
			printf("ERROR reading mark inside preWrite!!!\n");
			error_code = ERROR_READ_FAULT;
			goto LABEL_CLOSE_HANDLE;
		}
		if (bytes_read != MARK_LENGTH) {
			error_code = ERROR_READ_FAULT;
			goto LABEL_CLOSE_HANDLE;
		}

		if (!marked_in_table) {
			// Get if buffer is marked an unmark it
			marked = checkMark(read_buffer);
			if (marked) {
				marked = unmark(read_buffer);
			}
		}



		LABEL_CLOSE_HANDLE:
		/*if (read_handle && read_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(read_handle);
		}*/
		if (error_code != ERROR_SUCCESS) {
			free(read_buffer);
			return error_code;
		}
	}

	PRINT("The file %s marked\n", marked ? "IS" : "is NOT");

	// Nothing/Cipher/Decipher operations. Also in NOTHING case set to (re)mark if needed to leave as it was
	switch (op) {
		case NOTHING:
			if (!small_file && *orig_offset < MARK_LENGTH) {
				*mark_at_the_end = marked;	// Decide to mark or not to leave it as it was
				if (!marked && *orig_bytes_to_write + *orig_offset - 1 < MARK_LENGTH) {
					free(*aux_buffer);
					*aux_buffer = *orig_buffer;
					*aux_bytes_to_write = *orig_bytes_to_write;
					*aux_offset = *orig_offset;
				}
			}
			if (*orig_buffer != &(((byte*)*aux_buffer)[*orig_offset-*aux_offset])) {
				memcpy(&(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer, *orig_bytes_to_write);
			} // else --> no need to copy, it is the same buffer already
			break;
		case CIPHER:	// IF marked unmark THEN cipher (no marking) ELSE cipher (marking)
			if (!marked && *orig_offset < MARK_LENGTH) {
				*mark_at_the_end = TRUE;
			}
			// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (read_buffer != NULL){// && bytes_read>=512) {
				PRINT_HEX(read_buffer, bytes_read);
			}
			if (*aux_offset == 0) {
				memcpy(*aux_buffer, read_buffer, bytes_read);
			}
			if (*aux_buffer != NULL){// && *aux_bytes_to_write >= 1024) {
				PRINT_HEX(*aux_buffer, *aux_bytes_to_write);
			}
			if (*orig_buffer != NULL){// && *orig_bytes_to_write >= 1024) {
				PRINT_HEX(*orig_buffer, *orig_bytes_to_write);
			}
			// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				PRINT("ANTES: *orig_buffer = %.*s\n", *orig_bytes_to_write, *orig_buffer);
				PRINT_HEX(*orig_buffer, *orig_bytes_to_write);
				// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (*aux_offset == 0 || !(*orig_buffer == &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]))) {
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer, *orig_bytes_to_write, composed_key);
				} else {
					LPVOID orig_buffer_copy = malloc(*orig_bytes_to_write);
					if (orig_buffer_copy == NULL) {
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					memcpy(orig_buffer_copy, *orig_buffer, *orig_bytes_to_write);
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), orig_buffer_copy, *orig_bytes_to_write, composed_key);
					free(orig_buffer_copy);
				}
				/*if (*orig_buffer == &(((byte*)*aux_buffer)[*orig_offset])) {
					LPVOID orig_buffer_copy = malloc(*orig_bytes_to_write);
					if (orig_buffer_copy == NULL) {
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					memcpy(orig_buffer_copy, *orig_buffer, *orig_bytes_to_write);
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset]), orig_buffer_copy, *orig_bytes_to_write, composed_key);
					free(orig_buffer_copy);
				} else {
					invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset]), *orig_buffer, *orig_bytes_to_write, composed_key);
				}*/
				// TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				PRINT("DESPUES: *aux_buffer = %.*s\n", *aux_bytes_to_write, (char*)*aux_buffer);
				PRINT_HEX(*aux_buffer, *aux_bytes_to_write);
				// TESTING END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)\n", result);
				return 1;
			}
			break;
		case DECIPHER:	// IF marked unmark THEN decipher (omitting mark) ELSE (is cleartext) nothing
			if (*aux_offset != *orig_offset) {		// only needed for marked files
				memcpy(*aux_buffer, read_buffer, bytes_read);
			}

			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				// if (*aux_offset == 0)
				//		usar puntero magico
				//		if	(marked)
				//			decipher
				//		else
				//			memcpy
				// else
				//		usando puntero normal
				//		if	(marked)
				//			decipher
				//		else
				//			nada de nada
				if (!small_file && *aux_offset != *orig_offset) {
					if (marked) {
						invokeDecipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer, *orig_bytes_to_write, composed_key);
					} else {
						memcpy(&(((byte*)*aux_buffer)[*orig_offset-*aux_offset]), *orig_buffer, *orig_bytes_to_write);
					}
				} else {
					if (marked) {	// && !small_file   but this is always true for marked because marked is false by default
						LPVOID orig_buffer_copy = malloc(*orig_bytes_to_write);
						if (orig_buffer_copy == NULL) {
							return ERROR_NOT_ENOUGH_MEMORY;
						}
						memcpy(orig_buffer_copy, *orig_buffer, *orig_bytes_to_write);
						invokeDecipher(protection->cipher, *aux_buffer, orig_buffer_copy, *orig_bytes_to_write, composed_key);
						free(orig_buffer_copy);
					}
				}
			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
				return 1;
			}
			break;
		default:
			break;
	}

	free(read_buffer);

	// Mark if needed
	if (!small_file && *mark_at_the_end) {
		PRINT("Marking buffer in prewrite\n");
		mark(*aux_buffer);
	}

	return 0;
}

int postWriteLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof, BOOL *mark_at_the_end,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID*   aux_buffer, DWORD*  aux_bytes_to_write, LPDWORD*  aux_bytes_written, LONGLONG*  aux_offset
) {
	DWORD error_code = ERROR_SUCCESS;

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

	uint64_t aux_file_size = 0;
	LPVOID rw_buffer = NULL;
	LPVOID rw_buffer2 = NULL;
	LPVOID rw_buffer3 = NULL;
	struct KeyData* composed_key = protection->key;
	int result = 0;

	HANDLE rw_handle = INVALID_HANDLE_VALUE;
	DWORD bytes_rw = 0;
	LARGE_INTEGER distanceToMove = { 0 };
	BOOL marked = FALSE;


	error_code = getFileSize(&aux_file_size, handle, file_path);
	if (error_code != 0) {
		PRINT("ERROR getting file size\n");
		return error_code;
	}

	// Check files that become larger or smaller than MARK_LENGTH
	if (file_size < MARK_LENGTH && aux_file_size >= MARK_LENGTH) {
		// Check if the operation is decipher and, if the file was marked at origin (and it is now in this fs) then unmark and decipher all the written contents + add it to a list
		if (op == DECIPHER) {
			// Create new handle
			CloseHandle(handle);
			rw_handle = CreateFileW(
				file_path,				// Name of the file
				GENERIC_READ + GENERIC_WRITE,	// Open for read/write
				0,						// Do not share
				NULL,					// Default security
				OPEN_EXISTING,			// Open existing file only
				FILE_ATTRIBUTE_NORMAL,	// Normal file
				NULL);					// No attr. template

			if (!rw_handle || rw_handle == INVALID_HANDLE_VALUE) {
				printf("ERROR: opening read_handle.\n");
				error_code = ERROR_OPEN_FAILED;
				goto LABEL_CLOSE_HANDLE1;
			}
			handle = rw_handle;

			// Point the handle to position 0
			if (!SetFilePointerEx(rw_handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				PRINT("ERROR handle seeking (error=%lu)\n", error_code);
				goto LABEL_CLOSE_HANDLE1;
			}

			// Allocate read buffer
			rw_buffer = malloc(MARK_LENGTH * sizeof(byte));
			if (rw_buffer == NULL) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				goto LABEL_CLOSE_HANDLE1;
			}

			// Read firs part of the file (MARK_LENGTH bytes)
			if (!ReadFile(
				rw_handle,
				rw_buffer,
				MARK_LENGTH,
				&bytes_rw,
				NULL)
				) {
				printf("ERROR reading mark inside preWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE1;
			}
			if (bytes_rw != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE1;
			}

			// Check mark and unmark the content
			marked = checkMark(rw_buffer);
			if (marked) {
				marked = unmark(rw_buffer);
			}

			// If marked
			//		Read the rest of the file
			//		Unmark the content
			//		Decipher the content
			//		Write new content to file
			//		Add file to table
			if (marked) {
				// Allocate buffer for reading the rest of the file and another one for writting everything (in 1 operation instead of 2)
				rw_buffer2 = malloc((aux_file_size - MARK_LENGTH) * sizeof(byte));
				if (rw_buffer2 == NULL) {
					error_code = ERROR_NOT_ENOUGH_MEMORY;
					goto LABEL_CLOSE_HANDLE1;
				}
				rw_buffer3 = malloc(aux_file_size * sizeof(byte));
				if (rw_buffer3 == NULL) {
					error_code = ERROR_NOT_ENOUGH_MEMORY;
					goto LABEL_CLOSE_HANDLE1;
				}

				// Read the rest of file
				if (!ReadFile(
					rw_handle,
					rw_buffer2,
					(aux_file_size - MARK_LENGTH),
					&bytes_rw,
					NULL)
					) {
					printf("ERROR reading mark inside preWrite!!!\n");
					error_code = ERROR_READ_FAULT;
					goto LABEL_CLOSE_HANDLE1;
				}
				if (bytes_rw != (aux_file_size - MARK_LENGTH)) {
					error_code = ERROR_READ_FAULT;
					goto LABEL_CLOSE_HANDLE1;
				}

				// Make the composed key if necessary
				result = makeComposedKey(protection->challenge_groups, composed_key);
				if (result == 0) {
					// Decipher all the current content of the file
					invokeDecipher(protection->cipher, rw_buffer3, rw_buffer, MARK_LENGTH, composed_key);
					invokeDecipher(protection->cipher, &(((byte*)rw_buffer3)[MARK_LENGTH]), rw_buffer2, (aux_file_size - MARK_LENGTH), composed_key);
				} else {
					fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
					return 1;
				}

				// Point the handle to position 0
				if (!SetFilePointerEx(rw_handle, distanceToMove, NULL, FILE_BEGIN)) {
					error_code = GetLastError();
					PRINT("ERROR handle seeking (error=%lu)\n", error_code);
					goto LABEL_CLOSE_HANDLE1;
				}

				// Write new content to file
				if (!WriteFile(
					rw_handle,
					rw_buffer3,
					aux_file_size,
					&bytes_rw,
					NULL)
					) {
					printf("ERROR reading mark inside preWrite!!!\n");
					error_code = ERROR_READ_FAULT;
					goto LABEL_CLOSE_HANDLE1;
				}
				if (bytes_rw != aux_file_size) {
					error_code = ERROR_READ_FAULT;
					goto LABEL_CLOSE_HANDLE1;
				}

				// Add file to table
				addToTable(file_path);
			}
			LABEL_CLOSE_HANDLE1:
			if (rw_handle && rw_handle != INVALID_HANDLE_VALUE) {
				CloseHandle(rw_handle);
			}
			if (rw_buffer != NULL) {
				free(rw_buffer);
			}
			if (rw_buffer2 != NULL) {
				free(rw_buffer2);
			}
			if (rw_buffer3 != NULL) {
				free(rw_buffer3);
			}
			if (error_code != ERROR_SUCCESS) {
				return error_code;
			}
		}

		// Mark if needed
		if (*mark_at_the_end) {
			// Read MARK_LENGTH and re-write it marked
			PRINT("TO DO!!!!! create handle and adjust its offset to read correctly\n");
			CloseHandle(handle);
			rw_handle = CreateFileW(
				file_path,				// Name of the file
				GENERIC_READ + GENERIC_WRITE,	// Open for read/write
				0,						// Do not share
				NULL,					// Default security
				OPEN_EXISTING,			// Open existing file only
				FILE_ATTRIBUTE_NORMAL,	// Normal file
				NULL);					// No attr. template

			if (!rw_handle || rw_handle == INVALID_HANDLE_VALUE) {
				printf("ERROR: opening read_handle.\n");
				error_code = ERROR_OPEN_FAILED;
				goto LABEL_CLOSE_HANDLE2;
			}
			handle = rw_handle;

			// Point the handle
			if (!SetFilePointerEx(rw_handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				PRINT("ERROR handle seeking (error=%lu)\n", error_code);
				goto LABEL_CLOSE_HANDLE2;
			}

			// Allocate read buffer
			rw_buffer = malloc(MARK_LENGTH * sizeof(byte));
			if (rw_buffer == NULL) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				goto LABEL_CLOSE_HANDLE2;
			}

			// Read header of file
			if (!ReadFile(
				rw_handle,
				rw_buffer,
				MARK_LENGTH,
				&bytes_rw,
				NULL)
				) {
				printf("ERROR reading mark inside preWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE2;
			}
			if (bytes_rw != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE2;
			}

			// Get if buffer is marked an unmark it
			PRINT("TO DO Marking buffer in postwrite\n");
			mark(rw_buffer);
			PRINT("MARKED in postWrite\n");

			// Point the handle
			if (!SetFilePointerEx(rw_handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				PRINT("ERROR handle seeking (error=%lu)\n", error_code);
				goto LABEL_CLOSE_HANDLE2;
			}

			if (!WriteFile(
				rw_handle,
				rw_buffer,
				MARK_LENGTH,
				&bytes_rw,
				NULL)
				) {
				printf("ERROR reading mark inside preWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE2;
			}
			if (bytes_rw != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto LABEL_CLOSE_HANDLE2;
			}

			LABEL_CLOSE_HANDLE2:
			if (rw_handle && rw_handle != INVALID_HANDLE_VALUE) {
				CloseHandle(rw_handle);
			}
			if (rw_buffer != NULL) {
				free(rw_buffer);
			}
			if (error_code != ERROR_SUCCESS) {
				return error_code;
			}
		}
	}

	return 0;
}
