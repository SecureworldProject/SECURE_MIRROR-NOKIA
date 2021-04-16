/*
SecureWorld file logic.c
Contains the pre and post logic functions for all operations.
Invokes functions like cypher(), decypher(), mark(), unmark(), etc

Nokia Febrero 2021
*/


/////  FILE INCLUDES  /////

#include "dokan.h"
#include "context.h"
#include "winnt.h"
#include <psapi.h>
#include "context.h"
#include "wrapper_dokan.h"
//#include "wrapper_winfsp.h"




/////  FUNCTION HEADERS  /////

int preCreateLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int preReadLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int postReadLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);
int preWriteLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset);




/////  FUNCTION IMPLEMENTATIONS  /////

void fixBuffer() {
	// TO DO
}
void fixBufferLimitsPre() {
	// TO DO
}
void cipher() {
	// TO DO
}
void decipher() {
	// TO DO
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


int preCreateLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset) {
	PRINT("preCreateLogic!!  %d \n", num);
	PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);

	PRINT("TO DO \n");

	return 0;
}

// This function allocates buffers to a memory size adjusted to the blocksize and other parameters. If blocksize is 0, allocates buffers of the same size.
int preReadLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset) {
	// Change offset and bytes_to_do
	switch (op) {
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
	}
	return 0;
}

int postReadLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset) {
	PRINT("postReadLogic!!  %d \n", num);
	PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);

	// If write and cipher is by blocks, read necessary partial block (done before each cipher/decipher)
	// If write, execute operation
	switch (op) {
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
	}

	return 0;
}


int preWriteLogic(int num, enum Operation op, WCHAR file_path[], LPVOID* buffer, DWORD* bytes_to_do, LPDWORD* bytes_done, LONGLONG* offset) {
	PRINT("preWriteLogic!!  %d \n", num);
	PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);

	// If write and cipher is by blocks, read necessary partial block (done before each cipher/decipher)
	// If write, execute operation
	switch (op) {
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
	}

	return 0;
}
