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

#define MARK_LENGTH 512

// This is a byte array of length 'MARK_LENGTH'
byte FILLING_SEQUENCE[] = {
	// Each line contains 8*8 = 64 zeros
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros ( 64)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (128)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (192)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (256)

	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (320)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (384)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,	// +64 zeros (448)
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0	// +64 zeros (512)
};

union UNION_UINT16 {
	struct {
		uint8_t low;
		uint8_t high;
	} part;
	uint16_t full;
};




/////  FUNCTION PROTOTYPES  /////

void cipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size, struct KeyData* composed_key);
void decipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size, struct KeyData* composed_key);

BOOL checkMark(uint8_t* input);
BOOL mark(uint8_t* input);
BOOL unmark(uint8_t* input);




/////  FUNCTION IMPLEMENTATIONS  /////

enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]) {
	enum operation op = NOTHING;
	WCHAR* tmp_str = NULL;
	PRINT("Checking if path (%ws) is in any syncfolder\n", file_path);

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
					return NOTHING;		// CIPHER 2 times
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
					return NOTHING;		// DECIPHER 2 times
				default:
					break;
			}
			break;
		default:
			break;
	}
	return NOTHING;
}

void fixBuffer() {
	// TO DO
}
void fixBufferLimitsPre() {
	// TO DO
}

void cipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size, struct KeyData* composed_key) {
	PRINT("Calling cipher dll......\n");

	typedef int(__stdcall* cipher_func_type)(LPVOID, LPVOID, DWORD, struct KeyData*);

	cipher_func_type cipher_func;
	int result;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");
	if (cipher_func != NULL) {
		result = cipher_func(out_buf, in_buf, buf_size, composed_key);
		if (result != 0) {
			PRINT("WARNING: error \n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}

}

void decipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size, struct KeyData* composed_key) {
	PRINT("Calling decipher dll......\n");

	typedef int(__stdcall* decipher_func_type)(LPVOID, LPVOID, DWORD, struct KeyData*);

	decipher_func_type decipher_func;
	int result;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");
	if (decipher_func != NULL) {
		result = decipher_func(out_buf, in_buf, buf_size, composed_key);
		if (result != 0) {
			PRINT("WARNING: error \n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}

}

/*
int getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path) {
	// check GetFileSizeEx()...
	BOOL opened = FALSE;


	// reopen the file
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		PRINT("invalid handle, cleanuped?\n");
		handle = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			DWORD error = GetLastError();
			PRINT("\tCreateFile error : %d\n\n", error);
			return -1;
		}
		opened = TRUE;
	}

	*file_size = 0;
	DWORD fileSizeLow = 0;
	DWORD fileSizeHigh = 0;
	fileSizeLow = GetFileSize(handle, &fileSizeHigh);
	if (fileSizeLow == INVALID_FILE_SIZE) {
		DWORD error = GetLastError();
		PRINT("\tcan not get a file size error = %d\n", error);
		if (opened)
			CloseHandle(handle);
		return -1;
	}

	*file_size = ((uint64_t)fileSizeHigh << 32) | fileSizeLow;

	return 0;
}

BOOL checkMarkOLD(HANDLE handle, WCHAR* file_path, uint8_t* input) {
	// TO DO


	// Read first MARK_LENGTH bytes
	DWORD bytes_to_read = MARK_LENGTH;
	DWORD bytes_read;

	if (!ReadFile(handle, (LPVOID) input, bytes_to_read, &bytes_read, NULL)) {
		return FALSE;
	}

	// Get first 2 bytes and interpret it as a number C (size of the compressed stream).
	uint16_t compressed_stream_size = 0;
	compressed_stream_size = ((uint16_t)input[0])<<8 + ((uint16_t)input[1]);

	// If from position C+2 until position M (end of stream) the apearing sequence does not match the filling sequece, then it is not marked.
	if (compressed_stream_size > MARK_LENGTH) {
		return FALSE;
	}
	if (memcmp(&(input[2 + compressed_stream_size - 1]), FILLING_SEQUENCE, MARK_LENGTH - (2 + compressed_stream_size - 1)) != 0) {
		return FALSE;
	}
									//  6 is  HEADER_BASE_SIZE
	if ((uint16_t)(*(uint16_t*)&input[4] + (6 << 3)) != (uint16_t)MARK_LENGTH) {
		return FALSE;
	}

	return TRUE;
}

void markOLD() {
	// TO DO

	// Check if the file is smaller than MARK_LENGTH
	HANDLE handle = (HANDLE)dokan_file_info->Context;
	uint64_t file_size;

	getFileSize(&file_size, handle, file_path);
	if (file_size < MARK_LENGTH) {
		return FALSE;
	}

	// Get first M bytes (stream) from file and compress them (resulting a compressed stream of C bytes).
	// Check if (C > M - 2), in that case, mark is omitted. Else:
	// Substitute first 2 bytes of the file with a codification of the C number. Then, substitute next C bytes with the compressed stream. Finally, fill the rest of the bytes until completing the M bytes with the filling sequence.


}

BOOL unmarkOLD(HANDLE handle, WCHAR* file_path) {	//input output (app buffers)
	// Check if the file is smaller than MARK_LENGTH
	uint64_t file_size;
	getFileSize(&file_size, handle, file_path);
	if (file_size < MARK_LENGTH) {
		return FALSE;
	}


	uint8_t* input;
	uint8_t* output;
	input = malloc(MARK_LENGTH * sizeof(uint8_t));

	if (!checkMark(handle, file_path, input)) {
		return FALSE;
	}

	// Read C bytes starting on position 3 and uncompress them. If the size of uncompressed is excactly M, we suppose file is marked.

	if (huffman_decode(input, &output) != EXIT_SUCCESS) {
		return FALSE;
	}
	// copy output-buffer into output-app-buffer


	free(input);
	free(output);
	// TO DO
}
*/

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
	// Get first 2 bytes and interpret it as a number C (size of the compressed stream).
	union UNION_UINT16 compressed_stream_size;
	compressed_stream_size.part.high = input[0];
	compressed_stream_size.part.low = input[1];

	// If from position C+2 until position M (end of stream) the apearing sequence does not match the filling sequece, then it is nor marked.
	if (compressed_stream_size.full > MARK_LENGTH) {
		return FALSE;
	}
	if (memcmp(&(input[2 + compressed_stream_size.full - 1]), FILLING_SEQUENCE, MARK_LENGTH - (2 + compressed_stream_size.full - 1)) != 0) {
		return FALSE;
	}
	if ((uint16_t)(*(uint16_t*)&input[4] + (6 /*HEADER_BASE_SIZE*/ << 3)) != (uint16_t)MARK_LENGTH) {
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
	uint32_t compressed_length;

	// Get first M bytes (stream) from file and compress them (resulting a compressed stream of C bytes).
	// Check if (C > M - 2), in that case, mark is omitted. Else:
	if (huffman_encode(input, &output, (uint32_t)MARK_LENGTH, &compressed_length) != 0 || compressed_length >= MARK_LENGTH-2) {
		return FALSE;
	}

	// Substitute first 2 bytes with a codification of the C number
	union UNION_UINT16 dummy;
	dummy.full = compressed_length;
	input[0] = dummy.part.high;
	input[1] = dummy.part.low;

	// Then, substitute next C bytes with the compressed stream
	memcpy(&(input[2]), output, compressed_length);

	// Fill the rest of the bytes until completing the M bytes with the filling sequence
	memcpy(&(input[2 + compressed_length - 1]), FILLING_SEQUENCE, MARK_LENGTH - (2 + compressed_length - 1));

	return TRUE;
}

/**
* Unmarks the buffer if it is possible. Returns if the resulting input buffer was unmarked.
* Assumes that the input buffer is long enough and it is marked.
*
* @param uint8_t* input
*		The buffer to be unmarked. In case it cannot be unmarked, it is not modified.
* @return BOOL
*		TRUE if the buffer was unmarked. FALSE if it could not.
**/
BOOL unmark(uint8_t* input) {
	uint8_t* output;		// Allocated inside huffman_decode

	// Uncompress bytes from position 3 of the input
	if (huffman_decode(&(input[2]), &output) != 0) {
		return FALSE;
	}

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
int preReadLogic(enum Operation op, WCHAR file_path[], LPCVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset) {
	// Change offset and bytes_to_do
	/*switch (op) {
		case NOTHING:
		case MARK:
		case UNMARK:
			break;

		case CIPHER:
			fixBufferLimitsPre();	// Change offset and bytes to do
			break;
		case DECIPHER:
			fixBufferLimitsPre();	// Change offset and bytes to do
			break;
		case IF_MARK_UNMARK_ELSE_CIPHER:
			if (checkMark()) {
				unmark();
			} else {
				fixBufferLimitsPre();	// Change offset and bytes to do
				cipher();
			}
			break;
		case IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING:
			if (checkMark()) {
				unmark();
				fixBufferLimitsPre();	// Change offset and bytes to do
				decipher();
			} else {
				fixBufferLimitsPre();	// Change offset and bytes to do
				cipher();
			}
			break;
		default:
			break;
	}*/
	return 0;
}

int postReadLogic(enum Operation op, WCHAR file_path[], LPCVOID* in_buffer, DWORD* buffer_length, LPDWORD* bytes_done, LONGLONG* offset, struct Protection *protection, LPCVOID out_buffer) {
	struct KeyData* composed_key = NULL;
	int result = 0;

	PRINT("postReadLogic!!\n");
	PRINT(" - Operation: %d\n - File path: %ws\n - InBuffer: %p\n - Buffer length: %lu\n - Bytes done: %lu\n - Offset: %lld\n - Cipher: %s\n - OutBuffer: %p\n",
		op, file_path, *in_buffer, *buffer_length, **bytes_done, *offset, protection->cipher->id, out_buffer);

	composed_key = protection->key;


	// Execute real logic associated
	switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				cipher(protection->cipher, *in_buffer, out_buffer, **bytes_done, composed_key);
			} else {
				fprintf(stderr, "ERROR in postReadLogic (%d)", result);
				return 1;
			}
			break;
		case DECIPHER:	// Call decipher
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				decipher(protection->cipher, *in_buffer, out_buffer, **bytes_done, composed_key);
			} else {
				fprintf(stderr, "ERROR in postReadLogic (%d)", result);
				return 1;
			}
			break;
		default:
			break;
	}


	// If write and cipher is by blocks, read necessary partial block (done before each cipher/decipher)
	// If write, execute operation
	/*switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			fixBuffer();	// In case of block cipher
			cipher();
			break;
		case DECIPHER:	// Call decipher
			fixBuffer();	// In case of block cipher
			decipher();
			break;
		case MARK:		// Call mark
			mark();
			break;
		case UNMARK:	// Call unmark
			unmark();
			break;
		case IF_MARK_UNMARK_ELSE_CIPHER:	// Call check mark, if it is true, then unmark, else cipher
			if (checkMark()) {
				unmark();
			} else {
				fixBuffer();	// In case of block cipher
				cipher();
			}
			break;
		case IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING:		// Call check mark, if it is true, then unmark and decipher, else cipher
			if (checkMark()) {
				unmark();
				fixBuffer();	// In case of block cipher
				decipher();
			} else {
				fixBuffer();	// In case of block cipher
				cipher();
			}
			break;
		default:
			break;
	}*/

	return 0;
}

int preWriteLogic(enum Operation op, WCHAR file_path[], LPCVOID* out_buffer, DWORD* bytes_to_write, LPDWORD* bytes_written, LONGLONG* offset, struct Protection* protection, LPCVOID in_buffer) {
	struct KeyData* composed_key = NULL;
	int result = 0;

	PRINT("preWriteLogic!!\n");
	PRINT(" - Operation: %d\n - File path: %ws\n - InBuffer: %p\n - Bytes to write: %lu\n - Bytes written: %lu\n - Offset: %lld\n - Cipher: %s\n - OutBuffer: %p\n",
		op, file_path, in_buffer, *bytes_to_write, **bytes_written, *offset, protection->cipher->id, *out_buffer);

	composed_key = protection->key;

	// Execute real logic associated
	switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				cipher(protection->cipher, in_buffer, *out_buffer, *bytes_to_write, composed_key);
			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
				return 1;
			}
			break;
		case DECIPHER:	// Call decipher
			result = makeComposedKey(protection->challenge_groups, composed_key);
			if (result == 0) {
				decipher(protection->cipher, in_buffer, *out_buffer, *bytes_to_write, composed_key);
			} else {
				fprintf(stderr, "ERROR in preWriteLogic (%d)", result);
				return 1;
			}
			break;
		default:
			break;
	}



	// If write and cipher is by blocks, read necessary partial block (done before each cipher/decipher)
	// If write, execute operation
	/*switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			fixBuffer();	// In case of block cipher
			cipher();
			break;
		case DECIPHER:	// Call decipher
			fixBuffer();	// In case of block cipher
			decipher();
			break;
		case MARK:		// Call mark
			mark();
			break;
		case UNMARK:	// Call unmark
			unmark();
			break;
		case IF_MARK_UNMARK_ELSE_CIPHER:	// Call check mark, if it is true, then unmark, else cipher
			if (checkMark()) {
				unmark();
			} else {
				fixBuffer();	// In case of block cipher
				cipher();
			}
			break;
		case IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING:		// Call check mark, if it is true, then unmark and decipher, else cipher
			if (checkMark()) {
				unmark();
				fixBuffer();	// In case of block cipher
				decipher();
			} else {
				fixBuffer();	// In case of block cipher
				cipher();
			}
			break;
		default:
			break;
	}*/

	return 0;
}
