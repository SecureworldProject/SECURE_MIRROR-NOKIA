/*
SecureWorld file logic.c
Contains the pre and post logic functions for all operations.
Invokes functions like cypher(), decypher(), mark(), unmark(), etc

Nokia Febrero 2021
*/


/////  FILE INCLUDES  /////

#include "logic.h"
#include <Lmcons.h>	// to get UNLEN




/////  FUNCTION PROTOTYPES  /////

void cipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size);
void decipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size);
enum Operation operationAddition(enum Operation op1, enum Operation op2);




/////  FUNCTION IMPLEMENTATIONS  /////

enum Operation operationAddition(enum Operation op1, enum Operation op2) {
	switch (op1) {
		case NOTHING:
			switch (op1) {
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
			switch (op1) {
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
			switch (op1) {
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

void cipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size) {
	typedef int(__stdcall* cipher_func_type)(LPVOID, LPVOID, DWORD);

	cipher_func_type cipher_func;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");

	cipher_func(out_buf, in_buf, buf_size);
}

void decipher(struct Cipher* p_cipher, LPVOID in_buf, LPVOID out_buf, DWORD buf_size) {
	typedef int(__stdcall* decipher_func_type)(LPVOID, LPVOID, DWORD);

	decipher_func_type decipher_func;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");

	decipher_func(out_buf, in_buf, buf_size);
}

BOOL checkMark() {
	// TO DO
	return TRUE;
}
void mark() {
	// TO DO
}
void unmark() {
	// TO DO
}


int preCreateLogic(WCHAR file_path_param[]) {
	WCHAR* tmp_str;

	PRINT("preCreateLogic!!\n");
	//PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld \n", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);
	WCHAR* user = NULL;
	DWORD user_size = 0;
	LPDWORD p_user_size = &user_size;

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

	for (size_t i = 0; i < _msize(ctx.parentals) / sizeof(struct ParentalControl*); i++) {
		//PRINT("comparing parental: %ws \n", ctx.parentals[i]->folder);
		tmp_str = wcsstr(file_path, ctx.parentals[i]->folder);
		if (tmp_str && tmp_str == file_path) {
			// Initialize user name if not done yet
			if (user == NULL){
				user_size = UNLEN + 1;
				user = malloc(sizeof(WCHAR) * user_size);
				if (GetUserNameW(user, p_user_size) != 0) {
					PRINT("UserName is: %ws\n", user);
				} else {
					PRINT("ERROR getting user name. Blocking access by default...\n");
					return TRUE;
				}
			}
			//PRINT("returning TRUE for folder %ws\n", tmp_str);
			return TRUE;			// TO DO remove this

			// TO DO check all this stuff: the parental challenges must set a key like true/false or something like that
			// Check if user name is allowed for this folder and challenges are correct.
			/*for (size_t j = 0; j < _msize(ctx.parentals[i]->users) / sizeof(char*); j++) {			// TO DO fix users should use WCHAR* not char*
				if (wcscmp(ctx.parentals[i]->users, user) == 0) {		// If current user is in allowed users, test the challenges
					// Test the challenges, if they are correct, resturn FALSE (allow access) if not, return TRUE
					for (size_t k = 0; k < _msize(ctx.parentals[i]->challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*); k++) {
						if () {	// check parental challenges
							return FALSE;	// Allow access
						} else {
							return TRUE;	// Block access
						}
					}
				}
			}*/
		}
	}

	return FALSE;
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

int postReadLogic(enum Operation op, WCHAR file_path[], LPCVOID* in_buffer, DWORD* buffer_length, LPDWORD* bytes_done, LONGLONG* offset, struct Cipher *p_cipher, LPCVOID out_buffer) {
	PRINT("postReadLogic!!\n");
	PRINT(" - Operation: %d\n - File path: %ws\n - InBuffer: %p\n - Buffer length: %lu\n - Bytes done: %lu\n - Offset: %lld\n - Cipher: %s\n - OutBuffer: %p\n",
		op, file_path, *in_buffer, *buffer_length, **bytes_done, *offset, p_cipher->id, out_buffer);

	switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			cipher(p_cipher, *in_buffer, out_buffer, *buffer_length);
			break;
		case DECIPHER:	// Call decipher
			decipher(p_cipher, *in_buffer, out_buffer, *buffer_length);
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


int preWriteLogic(enum Operation op, WCHAR file_path[], LPCVOID* in_buffer, DWORD* bytes_to_write, LPDWORD* bytes_written, LONGLONG* offset, struct Cipher* p_cipher, LPCVOID out_buffer) {
	PRINT("preWriteLogic!!\n");
	PRINT(" - Operation: %d\n - File path: %ws\n - InBuffer: %p\n - Bytes to write: %lu\n - Bytes written: %lu\n - Offset: %lld\n - Cipher: %s\n - OutBuffer: %p\n",
		op, file_path, *in_buffer, *bytes_to_write, **bytes_written, *offset, p_cipher->id, out_buffer);

	switch (op) {
		case NOTHING:
			break;
		case CIPHER:	// Call cipher
			cipher(p_cipher, *in_buffer, out_buffer, *bytes_to_write);
			break;
		case DECIPHER:	// Call decipher
			decipher(p_cipher, *in_buffer, out_buffer, *bytes_to_write);
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
