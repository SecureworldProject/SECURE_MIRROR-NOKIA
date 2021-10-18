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

#define MAX_SIMULTANEOUS_DOWNLOADS 10		// OLD can be removed
#define FILE_MARK_INFO_TABLE_INITIAL_SIZE 32
#define FILE_MARK_INFO_TABLE_SIZE_INCREMENT 8


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

struct FileMarkInfo* createFMI(WCHAR* file_path, int8_t write_buffer_mark_lvl, int8_t file_mark_lvl);	// Allocates memory
void destroyFMI(struct FileMarkInfo* file_mark_info);													// Frees memory
void printFMI(struct FileMarkInfo* fmi);




/////  FUNCTION IMPLEMENTATIONS  /////

enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]) {
	enum operation op = NOTHING;
	WCHAR* tmp_str = NULL;
	PRINT("Checking if path (%ws) is in a syncfolder or not\n", file_path);

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

	if (file_path==NULL) {
		return NULL;
	}

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL && 0 == wcscmp(file_path, file_mark_info_table[i]->file_path)) {
			result = file_mark_info_table[i];
			file_mark_info_table[i] = NULL;
			PRINT("FMI removed from the table\n");
			printFMI(result);
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
			printFMI(result);
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
			if (file_mark_info_table[i] != file_mark_info) {
				destroyFMI(file_mark_info_table[i]);
				file_mark_info_table[i] = file_mark_info;
				PRINT("FMI overwritten in the table\n");
			} else {
				PRINT("Identical FMI in the table (no need to overwrite)\n");
			}
			return file_mark_info;
		}
	}

	// If an empty slot was not found, extend the table and save the index to the first empty slot
	if (!found_empty) {
		size_t fmi_table_size_increment = (file_mark_info_table_size == 0) ? FILE_MARK_INFO_TABLE_INITIAL_SIZE : FILE_MARK_INFO_TABLE_SIZE_INCREMENT;
		tmp_table = realloc(file_mark_info_table, (file_mark_info_table_size + fmi_table_size_increment) * sizeof(struct FileMarkInfo*));
		if (tmp_table == NULL) {
			printf("ERROR: allocating memory for file_mark_info_table.\n");
			return NULL;
		} else {
			// Save the first empty slot index
			empty_index = file_mark_info_table_size;

			// Update the table pointer and size, and set new slots to NULL
			file_mark_info_table = tmp_table;
			for (size_t i = 0; i < fmi_table_size_increment; i++) {
				file_mark_info_table[file_mark_info_table_size + i] = (struct FileMarkInfo*)NULL;
			}
			file_mark_info_table_size += fmi_table_size_increment;
			//PRINT_HEX(file_mark_info_table, file_mark_info_table_size * sizeof(struct FileMarkInfo*));
		}
	}// At this point an empty slot always exist in the position empty_index

	// Assign parameter to the empty slot
	file_mark_info_table[empty_index] = file_mark_info;
	PRINT("FMI written in the table\n");

	return file_mark_info;
}

/**
* Prints all the values inside the FileMarkInfo struct (write_bufer_mark_lvl, file_mark_lvl and file_path) or NULL if it is the case.
*
* @param struct FileMarkInfo* fmi
*		The struct FileMarkInfo* to be printed.
*
* @return
**/
void printFMI(struct FileMarkInfo* fmi) {
	if (fmi == NULL) {
		PRINT("FileMarkInfo --> NULL \n");
	} else {
		PRINT("FileMarkInfo --> write_buf_mark: %d \t file_mark: %d   \t path: %ws\n", fmi->write_buffer_mark_lvl, fmi->file_mark_lvl, fmi->file_path);
	}
}

/**
* Prints the table itself (pointers in memory as hexadecimal) and then all the values of the structs inside (write_bufer_mark_lvl, file_mark_lvl and file_path).
*
* @return
**/
void printFMITable() {
	struct FileMarkInfo* fmi = NULL;

	// Print the pointers as they are stored
	PRINT_HEX(file_mark_info_table, file_mark_info_table_size * sizeof(struct FileMarkInfo*));

	// Print if the pointers are NULL and struct internal data (write_bufer_mark_lvl, file_mark_lvl and file_path) if not
	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		fmi = file_mark_info_table[i];
		PRINT1("Row %llu: ", i);
		printFMI(fmi);
	}
}

// Allocates memory
struct FileMarkInfo* createFMI(WCHAR* file_path, int8_t write_buffer_mark_lvl, int8_t file_mark_lvl) {
	if (file_path == NULL) {
		return NULL;
	}

	struct FileMarkInfo* fmi = NULL;
	fmi = malloc(1 * sizeof(struct FileMarkInfo));
	if (fmi != NULL) {
		wcscpy(fmi->file_path, file_path);
		fmi->write_buffer_mark_lvl = write_buffer_mark_lvl;
		fmi->file_mark_lvl = file_mark_lvl;
	}
	return fmi;
}

// Frees memory
void destroyFMI(struct FileMarkInfo* file_mark_info) {
	free(file_mark_info);
}

/**
* Tests the functionality of the FMI related functions and the file_mark_info_table.
*
* @return
**/
void testFMItable() {
	WCHAR wstring[] = L"C:/test/00";
	struct FileMarkInfo *fmi = NULL;

	srand(time(NULL));

	for (size_t times = 0; times < 2; times++) {
		if (times != 0) {
			PRINT("\n\nREPEATING EVERYTHING AGAIN!\n\n\n");
		}

		// Fill the table
		PRINT1("\nFilling the table...\n");
		for (size_t i = 0; i < 3; i++) {
			for (size_t j = 0; j < 10; j++) {
				wstring[8] = L'0' + i;
				wstring[9] = L'0' + j;
				addFMITableEntry(createFMI(wstring, 128, 128));
			}
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();

		// Retrieve 5 random values
		PRINT1("\nRetrieving 5 random values from the table...\n");
		for (size_t i = 0; i < 5; i++) {
			wstring[8] = L'0' + rand() % 3;
			wstring[9] = L'0' + rand() % 10;
			fmi = getFMITableEntry(wstring);
			if (fmi != NULL) {
				PRINT2("path: %ws \t write_buf_mark: %d \t file_mark: %d\n", fmi->file_path, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
			}
		}

		// Add an already existing pointer to collide
		PRINT1("\nAdding an already existing pointer to collide in the table...\n");
		addFMITableEntry(fmi);

		// Add 5 random values that will collide. This time with different mark levels
		PRINT1("\nAdding 5 random values to the table...\n");
		for (size_t i = 0; i < 5; i++) {
			wstring[8] = L'0' + rand() % 3;
			wstring[9] = L'0' + rand() % 10;
			addFMITableEntry(createFMI(wstring, (rand() % 3) - 1, (rand() % 3) - 1));
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();


		// Remove 5 random values
		PRINT1("\nRemoving 5 random values from the table...\n");
		for (size_t i = 0; i < 5; i++) {
			wstring[8] = L'0' + rand() % 3;
			wstring[9] = L'0' + rand() % 10;
			fmi = removeFMITableEntry(wstring);
			if (fmi != NULL) {
				PRINT2("path: %ws \t write_buf_mark: %d \t file_mark: %d\n", fmi->file_path, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
			}
			destroyFMI(fmi);
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();


		// Clear the table
		PRINT1("\nClearing the table...\n");
		for (size_t i = 0; i < 3; i++) {
			for (size_t j = 0; j < 10; j++) {
				wstring[8] = L'0' + i;
				wstring[9] = L'0' + j;
				fmi = removeFMITableEntry(wstring);
				destroyFMI(fmi);
			}
		}

	}
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

	/*LPVOID src_buf_copy = NULL;
	if (dst_buf == src_buf) {
		src_buf_copy = malloc(buf_size);
		if (src_buf_copy == NULL) {
			return;		// If copy of the buffer cannot be allocated, skip ciphering
			NOOP;		// If copy of the buffer cannot be allocated, pray for it to work
		} else {
			memcpy(src_buf_copy, src_buf, buf_size);
		}
	}

	typedef int(__stdcall* cipher_func_type)(LPVOID, LPVOID, DWORD, size_t, struct KeyData*);

	cipher_func_type cipher_func;
	int result;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");
	if (cipher_func != NULL) {
		if (src_buf_copy==NULL) {
			result = cipher_func(dst_buf, src_buf, buf_size, offset, composed_key);
		} else {
			result = cipher_func(dst_buf, src_buf_copy, buf_size, offset, composed_key);
			free(src_buf_copy);
		}
		if (result != 0) {
			PRINT("WARNING: error ciphering\n");
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

	/*LPVOID src_buf_copy = NULL;
	if (dst_buf == src_buf) {
		src_buf_copy = malloc(buf_size);
		if (src_buf_copy == NULL) {
			return;		// If copy of the buffer cannot be allocated, skip deciphering
			NOOP;		// If copy of the buffer cannot be allocated, pray for it to work
		} else {
			memcpy(src_buf_copy, src_buf, buf_size);
		}
	}

	typedef int(__stdcall* decipher_func_type)(LPVOID, LPVOID, DWORD, size_t, struct KeyData*);

	decipher_func_type decipher_func;
	int result;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");
	if (decipher_func != NULL) {
		if (src_buf_copy==NULL) {
			result = decipher_func(dst_buf, src_buf, buf_size, offset, composed_key);
		} else {
			result = decipher_func(dst_buf, src_buf_copy, buf_size, offset, composed_key);
			free(src_buf_copy);
		}

		if (result != 0) {
			PRINT("WARNING: error deciphering\n");
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
* In the case that 'mark_level' is 0, buffer is left unmodified.
*
* @param uint8_t* input
*		The buffer to be marked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be marked, it is not modified.
* @param int8_t mark_level
*		The mark level to set in the mark. If it is 0, the buffers is left unmodified. Its value cannot be INVALID_MARK_LEVEL.
*
* @return int8_t
*		'mark_level' if the buffer was correctly marked (or not needed in case 'mark_level'==0). INVALID_MARK_LEVEL otherwise.
**/
int8_t mark(uint8_t* input, int8_t mark_level) {
	// Check valid mark level
	if (mark_level == INVALID_MARK_LEVEL) {
		return INVALID_MARK_LEVEL;
	}

	// If asking to mark with level 0, then do nothing
	if (mark_level == 0) {
		return 0;
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
* Allows any mark level from -127 to 127, but not 0 or INVALID_MARK_LEVEL=-128.
*
* @param uint8_t* input
*		The buffer to be unmarked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be unmarked, it is not modified.
*
* @return int8_t
*		The level of the mark if the buffer is marked. 0 otherwise.
**/
int8_t unmark(uint8_t* input) {
	uint16_t decompressed_length = ((uint16_t*)(input))[0];	//*(uint32_t*)&input[0];
	uint16_t compressed_length = ((uint16_t*)(input))[1];	//*(uint32_t*)&input[1];
	uint16_t header_bit_length = ((uint16_t*)(input))[2] + (6 /*HEADER_BASE_SIZE*/ << 3); //*(uint16_t*)&input[4] + (6 /*HEADER_BASE_SIZE*/ << 3);
	int8_t mark_level = (int8_t)input[MARK_LENGTH - 1];

	PRINT("Checking mark. decompressed_length=%u, compressed_length=%u, header_bit_length=%u (%u bytes)\n", decompressed_length, compressed_length, header_bit_length, header_bit_length / 8 + ((header_bit_length % 8) ? 0 : 1));

	if (mark_level == INVALID_MARK_LEVEL || mark_level == 0) {
		return 0;
	}

	if (decompressed_length != (uint16_t)MARK_LENGTH) {
		return 0;
	}

	if (compressed_length > MARK_LENGTH - 1) {	// -1 due to the mark level requires 1 Byte
		return 0;
	}

	if (memcmp(&(input[compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - 1 - compressed_length) != 0) {	// -1 due to the mark level requires 1 Byte
		return 0;
	}

	uint8_t* output = NULL;		// Allocated inside huffman_decode
	PRINT("Trying to unmark...\n");

	// Uncompress input
	if (huffman_decode(input, &output) != 0) {
		PRINT("Could not be unmarked\n");
		return 0;
	}
	PRINT("Unmarked\n");

	// Copy decoded buffer into input buffer
	memcpy(input, output, MARK_LENGTH);

	// Free output buffer
	free(output);

	return mark_level;	// Allows mark_level to be any number from -127 to 127 (without the values 0 and INVALID_MARK_LEVEL=-128)
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




int preReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_buffer_length, LPDWORD* aux_read_length, LONGLONG* aux_offset
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
	//PRINT(" - Aux buffer: %p\n - Aux buffer length: %lu\n - Aux bytes done: %lu\n - Aux offset: %lld\n",
	//	*aux_buffer, *aux_buffer_length, **aux_read_length, *aux_offset);


	BOOL small_file = file_size < MARK_LENGTH;
	PRINT("The file %s small\n", small_file ? "IS" : "is NOT");

	// By default: buffer is the same and read starts and ends on the same place
	*aux_buffer = *orig_buffer;
	*aux_read_length = *orig_read_length;

	*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
	*aux_buffer_length = *orig_buffer_length;		//aux_fin = orig_fin;		// Buffer ends in same place



	// Check file size and buffer position/size and fix buffer limits in consequence
	// SMALL FILES
	if (small_file) {
		goto PRE_READ_END;
	}

	// BIG FILES (file_size >= MARK_LENGTH)
	// Offset further than the mark
	if (*orig_offset >= MARK_LENGTH) {
		// Buffer starts and ends on same place
		goto PRE_READ_END;
	}
	// Offset within the mark  (inicio < MARK_LENGTH)
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

	PRE_READ_END:
	PRINT("Ending preReadLogic\n");

	return 0;
}


int postReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_buffer_length, LPDWORD* aux_read_length, LONGLONG* aux_offset
) {

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
	BOOL small_file = file_size < MARK_LENGTH;

	LPVOID extra_read_buffer = NULL;
	DWORD extra_bytes_read = 0;
	DWORD error_code = 0;

	LARGE_INTEGER distanceToMove = { 0 };

	struct FileMarkInfo* fmi = NULL;
	int8_t mark_lvl = INVALID_MARK_LEVEL;

	// SMALL FILES
	if (small_file) {
		goto POST_READ_CLEANUP;
	}

	// BIG FILES (file_size >= MARK_LENGTH)
	// Check the table
	fmi = getFMITableEntry(file_path);
	if (fmi == NULL) {
		fmi = createFMI(file_path, INVALID_MARK_LEVEL, INVALID_MARK_LEVEL);
		addFMITableEntry(fmi);
	}

	// Offset further than the mark
	if (*orig_offset >= MARK_LENGTH) {
		if (fmi->file_mark_lvl == INVALID_MARK_LEVEL) {			// Read MARK_LENGTH first bytes and fill fmi->file_mark_lvl for next operations
			// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
			if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				PRINT("ERROR handle seeking in postWrite (error=%lu)\n", error_code);
				goto POST_READ_CLEANUP;
			}

			// Allocate read buffer
			extra_read_buffer = malloc(MARK_LENGTH * sizeof(byte));
			if (extra_read_buffer == NULL) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				goto POST_READ_CLEANUP;
			}

			// Read first part of the file (MARK_LENGTH bytes)
			if (!ReadFile(
				handle,
				extra_read_buffer,
				MARK_LENGTH,
				&extra_bytes_read,
				NULL)
				) {
				printf("ERROR reading mark in postWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto POST_READ_CLEANUP;
			}
			if (extra_bytes_read != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto POST_READ_CLEANUP;
			}

			// Check mark and unmark the content
			mark_lvl = unmark(extra_read_buffer);

			// Modify in the table
			fmi->file_mark_lvl = mark_lvl;

		} else {
			mark_lvl = fmi->file_mark_lvl;		// Just to use the variable
		}
	} else {	// Case where *orig_offset < MARK_LENGTH
		// Get if buffer is marked an unmark it
		mark_lvl = unmark(*aux_buffer);
		if (fmi->file_mark_lvl == INVALID_MARK_LEVEL) {
			// Modify in the table
			fmi->file_mark_lvl = mark_lvl;
		}
	}

	// Here the mark_lvl and fmi->file_mark_lvl variables should be filled (and equal)
	PRINT("The file mark level is %d\n", mark_lvl);
	if (mark_lvl != 1 && mark_lvl != 0 && mark_lvl != -1) {
		fprintf(stderr, "ERROR in postReadLogic: detected a file with undefined mark level\n");
		error_code = -2;
		goto POST_READ_CLEANUP;
	}

	// If the key may be needed, compute it. In case of error, fail the operation.
	if (op != NOTHING) {
		result = makeComposedKey(protection->challenge_groups, composed_key);
		if (0 != result) {
			fprintf(stderr, "ERROR composing key in postReadLogic (%d)\n", result);
			error_code = -1;
			goto POST_READ_CLEANUP;
		}
	}

	// For each of the possible cases: do the ciphering/deciphering/marking operations when needed
	switch (op) {
		case NOTHING:		// Do nothing (leave mark as it was)
			// Offset further than the mark
			if (*orig_offset >= MARK_LENGTH) {
				NOOP;
			}
			// Offset within the mark  (inicio < MARK_LENGTH)
			else {
				// Set the mark as it was
				mark(*aux_buffer, mark_lvl);
			}
			break;
		case CIPHER:
			switch (mark_lvl) {
				case 1:		// This should never happen
					printf("WARNING in postReadLogic: this should never happen (operation = %d, mark_lvl = %d)\n", op, mark_lvl);
					error_code = -4;
					goto POST_READ_CLEANUP;
				case 0:		// Cipher (and mark buffer if before MARK_LENGTH)
					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key);
					// Offset further than the mark
					if (*orig_offset >= MARK_LENGTH) {
						NOOP;
					}
					// Offset within the mark  (inicio < MARK_LENGTH)
					else {
						// Set the mark to level 1
						mark(*aux_buffer, 1);
					}
					break;
				case -1:	// Cipher (and leave without mark)
					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key);
					//// Offset further than the mark
					//if (*orig_offset >= MARK_LENGTH) {
					//	NOOP;
					//}
					//// Offset within the mark  (inicio < MARK_LENGTH)
					//else {
					//	NOOP;
					//}
					break;
			}
			break;
		case DECIPHER:
			switch (mark_lvl) {
				case 1:		// Decipher (and leave without mark)
					invokeDecipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key);
					// Offset further than the mark
					//if (*orig_offset >= MARK_LENGTH) {
					//	NOOP;
					//}
					//// Offset within the mark  (inicio < MARK_LENGTH)
					//else {
					//	NOOP;
					//}
					break;
				case 0:		// Do nothing (and leave without mark)
					NOOP;
					//// Offset further than the mark
					//if (*orig_offset >= MARK_LENGTH) {
					//	NOOP;
					//}
					//// Offset within the mark  (inicio < MARK_LENGTH)
					//else {
					//	NOOP;
					//}
					break;
				case -1:	// This should never happen
					printf("WARNING in postReadLogic: this should never happen (operation = %d, mark_lvl = %d)\n", op, mark_lvl);
					error_code = -4;
					goto POST_READ_CLEANUP;
			}
			break;
	}

	// Copy buffer aux to orig
	if (*orig_buffer != *aux_buffer) {
		memcpy(*orig_buffer, &(((byte*)*aux_buffer)[*orig_offset - *aux_offset]), *orig_buffer_length);
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

	PRINT("Ending postReadLogic\n");

	return error_code;	// 0 = Success
}




int preWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_bytes_to_write, LPDWORD* aux_bytes_written, LONGLONG* aux_offset
) {
	#pragma region COLLAPSABLE_REGION
	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL || aux_buffer == NULL || aux_bytes_to_write == NULL || aux_bytes_written == NULL || aux_offset == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("preWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - Protection: %p   (cipher->id: %s, key:%p)\n - Handle: %p\n - Write_to_eof: %u\n",
		*file_size, op, file_path, protection, protection->cipher->id, protection->key, handle, write_to_eof);
	PRINT(" - Orig buffer: %p\n - Orig bytes to write: %lu\n - Orig bytes written: %lu\n - Orig offset: %lld\n",
		*orig_buffer, *orig_bytes_to_write, **orig_bytes_written, *orig_offset);
	//PRINT(" - Aux buffer: %p\n - Aux bytes to write: %lu\n - Aux bytes written: %lu\n - Aux offset: %lld\n",
		//aux_buffer, *aux_bytes_to_write, **aux_bytes_written, *aux_offset);

	// Create other temporal variables
	struct KeyData* composed_key = protection->key;
	int result = 0;

	LPVOID read_buffer = NULL;
	DWORD bytes_read = 0;

	BOOL marked = FALSE;

	LARGE_INTEGER distanceToMove = { 0 };
	DWORD error_code = ERROR_SUCCESS;

	int8_t file_mark_lvl = INVALID_MARK_LEVEL;
	struct FileMarkInfo* fmi = NULL;

	// Writting to EOF is the same as the offset being the original file size
	if (write_to_eof) {
		*orig_offset = *file_size;
		PRINT("Write to EOF translated to new orig offset: %lld\n", *orig_offset);
	}
	#pragma endregion


	// Check if the file is small
	BOOL small_file = *file_size < MARK_LENGTH;
	// Check if the file is big but we are rewriting the file completely
	if (!small_file && *orig_offset==0 && *orig_bytes_to_write==*file_size) {
		small_file = TRUE;
		fmi = removeFMITableEntry(file_path);
		free(fmi);
		fmi = NULL;
		*file_size = 0;
	}
	PRINT("The file %s small\n", small_file ? "IS" : "is NOT");

	// By default: buffer is the same and write starts and ends on the same place
	*aux_buffer = *orig_buffer;
	*aux_bytes_written = *orig_bytes_written;

	*aux_offset = *orig_offset;						//aux_inicio = orig_inicio;	// Buffer starts on same place
	*aux_bytes_to_write = *orig_bytes_to_write;		//aux_fin = orig_fin;		// Buffer ends in same place


	// Check file size and buffer position/size and fix buffer limits in consequence
	// SMALL FILES
	if (small_file) {
		return ERROR_SUCCESS;
	}

	// BIG FILES (file_size >= MARK_LENGTH)
	// Check the table
	fmi = getFMITableEntry(file_path);
	printf("fmi %s null\n", fmi==NULL ? "IS" : "is NOT");
	if (op == DECIPHER && (fmi==NULL || fmi->write_buffer_mark_lvl == INVALID_MARK_LEVEL)) {
		printf("WARNING in preWriteLogic: this should never happen (operation = %d, mark_lvl = %d)\n", op, fmi->write_buffer_mark_lvl);
	}
	if (fmi == NULL) {
		fmi = createFMI(file_path, 0, INVALID_MARK_LEVEL);
		addFMITableEntry(fmi);
	}

	// Offset further than the mark
	if (*orig_offset >= MARK_LENGTH) {
		// Buffer starts and ends on same place
		NOOP;
	}
	// Offset within the mark  (orig_offset < MARK_LENGTH)
	else {
		if (*orig_bytes_to_write + *orig_offset < MARK_LENGTH) {
			// Buffer starts on 0 and ends on MARK_LENGTH
			*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
			*aux_bytes_to_write = MARK_LENGTH;							//aux_fin = MARK_LENGTH;	// Buffer ends at MARK_LENGTH
		} else {	// fin >= MARK_LENGTH
			// Buffer starts on 0 and ends on same place
			*aux_offset = 0;											//aux_inicio = 0;			// Buffer starts on file beginning
			*aux_bytes_to_write = *orig_bytes_to_write + *orig_offset;	//aux_fin = orig_fin;		// Buffer ends in same place
		}

		// Allocate space for the aux buffer
		*aux_buffer = malloc(*aux_bytes_to_write * sizeof(byte));
		PRINT("Allocation for aux_buffer\n");
		if (*aux_buffer == NULL) {
			return ERROR_NOT_ENOUGH_MEMORY;
		}

		// Read the beginning of the file to merge it with the writing buffer, because mark will be fully rewriten.

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

		// As we have already obtained the file mark level, we can check/update the level stored in the table
		file_mark_lvl = unmark(read_buffer);
		if (fmi->file_mark_lvl == INVALID_MARK_LEVEL) {
			// Update in the table
			fmi->file_mark_lvl = file_mark_lvl;
		} else if (fmi->file_mark_lvl != file_mark_lvl) {
			fprintf(stderr, "ERROR in preWriteLogic: file mark level in the table (%d) is different from the one actually read (%d)\n", fmi->file_mark_lvl, file_mark_lvl);
			error_code = -2;
			goto READ_INSIDE_WRITE_CLEANUP;
		}

		// Here the file_mark_lvl and fmi->file_mark_lvl variables should be filled (and equal)
		PRINT("The file mark level is %d\n", file_mark_lvl);
		if (file_mark_lvl != 1 && file_mark_lvl != 0 && file_mark_lvl != -1) {
			fprintf(stderr, "ERROR in preWriteLogic: detected a file with mark level different from -1, 0 or 1\n");
			error_code = -2;
			goto READ_INSIDE_WRITE_CLEANUP;
		}

		READ_INSIDE_WRITE_CLEANUP:
		if (error_code != ERROR_SUCCESS) {
			free(read_buffer);
			return error_code;
		}

		// Compose the buffer of what will be actually written (data extended up to the mark)
		memcpy(*aux_buffer, read_buffer, MARK_LENGTH);				// Copy bytes from file to the buffer
		memcpy(*aux_buffer, *orig_buffer, orig_bytes_to_write);		// Overwrite (or fill) the buffer with the bytes to write from the application

	}

	// Diagram of posible states of data to write and read buffer
	//	0                           MARK_LENGTH                                                            file_size
	//	|--------------------------------|------------------------------------------------ ... ----------------|
	//	|                                |                                                                     |
	//	|--------------------------------|-------[data-data-data-data-data]--------------- ... ----------------|	(*orig_offset >= MARK_LENGTH) <==> read_buffer == NULL
	//	|                                |                                                                     |
	//	|--[data-data-data-data-data]----|------------------------------------------------ ... ----------------|	(*orig_offset < MARK_LENGTH) && (orig_end < MARK_LENGTH)
	//	[read_buffer-read_buffer-read_buf]------------------------------------------------ ... ----------------|
	//	|                                |                                                                     |
	//	|--[data-data-data-data-data-data|data-data-data-data]---------------------------- ... ----------------|	(*orig_offset < MARK_LENGTH) && (orig_end >= MARK_LENGTH)
	//	[read_buffer-read_buffer-read_buf]------------------------------------------------ ... ----------------|


	// If the key may be needed, compute it. In case of error, fail the operation.
	if (op != NOTHING) {
		result = makeComposedKey(protection->challenge_groups, composed_key);
		if (0 != result) {
			fprintf(stderr, "ERROR composing key in preWriteLogic (%d)\n", result);
			error_code = -1;
			goto PRE_WRITE_CLEANUP;
		}
	}

	// For each of the possible cases: do the ciphering/deciphering/marking operations when needed
	// NOTE: each case can be divided into 3 phases (to ease understanding):
	//	- Checking
	//	- Transformation of the whole buffer (do nothing, cipher the buffer or decipher the buffer)
	//	- Mark or not the first part (MARK_LENGTH) of the buffer (only in the case of orig_offset is within the mark)
	switch (op) {
		case NOTHING:		// Do nothing (leave mark as it was)
			// Check that write_buffer_mark_level is the same than file_mark_level
			if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->write_buffer_mark_lvl != fmi->file_mark_lvl) {
				printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
				error_code = -3;
				goto PRE_WRITE_CLEANUP;
			}

			// Do not modify the whole buffer
			NOOP;

			// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
			if (*orig_offset < MARK_LENGTH) {
				// Set the mark level to 'fmi->write_buffer_mark_lvl' (that should be the same than it was in the file)
				mark(*aux_buffer, fmi->write_buffer_mark_lvl);
			}

			break;
		case CIPHER:
			switch (fmi->write_buffer_mark_lvl) {
				case 1:		// Do nothing (leave mark as it was --> mark buffer if before MARK_LENGTH). Note this should never happen
					// Check that write_buffer_mark_level is the same than file_mark_level (1)
					if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->file_mark_lvl != 1) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					} else {
						printf("NOTE in preWriteLogic: this operation was probably not intended. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
					}

					// Do not modify the whole buffer
					NOOP;

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						// Set the mark level to 'fmi->write_buffer_mark_lvl' (that should be the same than it was in the file)
						mark(*aux_buffer, fmi->write_buffer_mark_lvl);
					}

					break;
				case 0:		// Cipher (and mark buffer if before MARK_LENGTH)
					// Check that write_buffer_mark_level is one less than file_mark_level
					if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->file_mark_lvl!=1) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Cipher the whole buffer
					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_bytes_to_write, *aux_offset, composed_key);

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						// Set the mark to level 1
						mark(*aux_buffer, 1);
					}

					break;
				case -1:	// Cipher (and leave without mark)
					// Check that write_buffer_mark_level is one less than file_mark_level
					if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Cipher the whole buffer
					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_bytes_to_write, *aux_offset, composed_key);

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						NOOP;	// Set the mark to 0 (which is, don't mark)
					}

					break;
			}
			break;
		case DECIPHER:
			switch (fmi->write_buffer_mark_lvl) {
				case 1:		// Decipher (and leave without mark)
					// Check that write_buffer_mark_level is one more than file_mark_level
					if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Decipher the whole buffer
					invokeDecipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_bytes_to_write, *aux_offset, composed_key);

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						NOOP;	// Set the mark to 0 (which is, don't mark)
					}

					break;
				case 0:		// Do nothing (and leave without mark)
					// Check that write_buffer_mark_level is the same as file_mark_level
					if (fmi->file_mark_lvl != INVALID_MARK_LEVEL && fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->write_buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->write_buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Do not modify the whole buffer
					NOOP;

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						NOOP;	// Set the mark to 0 (which is, don't mark)
					}

					break;
				case -1:	// This should never happen
					printf("WARNING in preWriteLogic: this should never happen (operation = %d, write_buffer_mark_lvl = %d)\n", op, fmi->write_buffer_mark_lvl);
					error_code = -4;
					goto PRE_WRITE_CLEANUP;
					break;
			}
			break;
	}


	PRE_WRITE_CLEANUP:
	if (read_buffer != NULL) {
		free(read_buffer);
	}

	return error_code;
}


int postWriteLogic(
	uint64_t* file_size, enum Operation op, WCHAR* file_path, struct Protection* protection, HANDLE handle, UCHAR write_to_eof,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_bytes_to_write, LPDWORD* aux_bytes_written, LONGLONG* aux_offset
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
	int8_t mark_lvl = INVALID_MARK_LEVEL;

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("postWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - Protection: %p   (cipher->id: %s, key:%p)\n - Handle: %p\n - write_to_eof: %u\n",
		*file_size, op, file_path, protection, protection->cipher->id, protection->key, handle, write_to_eof);
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
	if (*file_size < MARK_LENGTH && new_file_size >= MARK_LENGTH) {
		PRINT("File has grown enough to admit mark (prev file_size = %llu, new file_size = %llu)\n", *file_size, new_file_size);

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
		mark_lvl = unmark(buffer1);

		// Add to the table
		addFMITableEntry(createFMI(file_path, mark_lvl, mark_lvl));


		// CASE OF CIPHER
		if (op == CIPHER && mark_lvl != 1) {
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
				// Cipher all the current content of the file
				invokeCipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key);
				invokeCipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key);
			} else {
				fprintf(stderr, "ERROR composing key in postWriteLogic (%d)", result);
				error_code = result;
				goto POST_WRITE_CLEANUP;
			}

			// Mark in the case of mark_lvl==0. If mark_lvl==-1, mark has already been taken out and there is no need to add another one
			if (mark_lvl==0) {
				mark(buffer3, 1);
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
				error_code = ERROR_WRITE_FAULT;
				goto POST_WRITE_CLEANUP;
			}
			if (bytes_done != new_file_size) {
				error_code = ERROR_WRITE_FAULT;
				goto POST_WRITE_CLEANUP;
			}
		}


		// CASE OF DECIPHER
		if (op == DECIPHER && mark_lvl == 1) {
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
				// Cipher all the current content of the file
				invokeDecipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key);
				invokeDecipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key);
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
				error_code = ERROR_WRITE_FAULT;
				goto POST_WRITE_CLEANUP;
			}
			if (bytes_done != new_file_size) {
				error_code = ERROR_WRITE_FAULT;
				goto POST_WRITE_CLEANUP;
			}
		}


		// CLEAR ALL THE BUFFERS USED
		POST_WRITE_CLEANUP:
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
		PRINT("File as already big or it is still small (prev file_size = %llu, new file_size = %llu)\n", *file_size, new_file_size);
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
