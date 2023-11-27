/*
SecureWorld file logic.c
Contains the pre and post logic functions for all operations.
Invokes functions like cypher(), decypher(), mark(), unmark(), etc

Nokia Febrero 2021
*/


/////  FILE INCLUDES  /////

#include "logic.h"
#include "keymaker.h"
#include <Lmcons.h>		// only to get UNLEN
#include "huffman.h"
#include <bcrypt.h>

#pragma comment(lib, "Bcrypt")






/////  DEFINITIONS  /////

#define FILE_MARK_INFO_TABLE_INITIAL_SIZE 32
#define FILE_MARK_INFO_TABLE_SIZE_INCREMENT 8
#define FMI_TABLE_ENTRY_EXPIRATION_TIME 5		// Defined in secconds
#define THREAD_PURGE_SLEEP (20*1000)			// Defined in milisecconds
#define MAX_NAME 256							// Username max length


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



struct FileMarkInfo** file_mark_info_table = NULL;
size_t file_mark_info_table_size = 0;




/////  FUNCTION PROTOTYPES  /////

struct FileMarkInfo* removeFMITableEntry(WCHAR* file_path, WCHAR* app_path);
struct FileMarkInfo* getFMITableEntry(WCHAR* file_path, WCHAR* app_path);
struct FileMarkInfo* addFMITableEntry(struct FileMarkInfo* file_mark_info);

struct FileMarkInfo* createFMI(WCHAR* file_path, WCHAR* app_path, int8_t buffer_mark_lvl, uint32_t buffer_frn, int8_t file_mark_lvl, uint32_t file_frn, time_t last_closed);
void destroyFMI(struct FileMarkInfo* fmi);
void printFMI(struct FileMarkInfo* fmi);
struct KeyData* createFileBufferKey(struct KeyData* composed_key, uint32_t frn);

int getUsernameByPID(const DWORD procId, char* strUser, char* strdomain);
BOOL getLogonFromTokenHandle(HANDLE hToken, char* strUser, char* strdomain);




/////  FUNCTION IMPLEMENTATIONS  /////

enum Operation getOpSyncFolder(enum IrpOperation irp_op, WCHAR file_path[]) {
	enum Operation op = NOTHING;
	WCHAR* tmp_str = NULL;
	PRINT("Checking if path (%ws) is in a syncfolder or not\n", file_path);

	for (size_t i = 0; i < _msize(ctx.sync_folders) / sizeof(WCHAR*); i++) {
		PRINT1("Checking sync folder (%ws) \n", ctx.sync_folders[i]);
		tmp_str = wcsstr(file_path, ctx.sync_folders[i]);
		if (tmp_str != NULL && tmp_str == file_path) {
			// Match found
			PRINT("Match found - Irp op (%s) in syncfolder (%ws)\n", (irp_op == IRP_OP_READ) ? "READ":"WRITE", file_path);
			if (irp_op == IRP_OP_READ) {
				op = DECIPHER;
			} else if (irp_op == IRP_OP_WRITE) {
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


/**
* Removes from the the file_mark_info_table the struct FileMarkInfo* associated to the file_path and app_path passed as parameters.
* Note: the returned pointer should be freed when its use has finished.
*
* @param WCHAR* file_path
*		The file path associated to the struct FileMarkInfo* wanted to be removed.
* @param WCHAR* app_path
*		The file path of the application associated to the struct FileMarkInfo* wanted to be removed.
*
* @return struct FileMarkInfo*
*		The (valid) pointer to the struct FileMarkInfo associated to the {file_path, app_path} passed as parameters or NULL if entry does not exist. Remember to free after use.
**/
struct FileMarkInfo* removeFMITableEntry(WCHAR* file_path, WCHAR* app_path) {
	struct FileMarkInfo* fmi = NULL;

	if (file_path==NULL || app_path==NULL) {
		return NULL;
	}

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL && 0 == wcscmp(file_path, file_mark_info_table[i]->file_path) && 0 == wcscmp(app_path, file_mark_info_table[i]->app_path)) {
			fmi = file_mark_info_table[i];
			file_mark_info_table[i] = NULL;
			PRINT("FMI removed from the table\n");
			printFMI(fmi);
			break;
		}
	}
	return fmi;
}

/**
* Finds the struct FileMarkInfo* associated to the file_path and app_path passed as parameters in the file_mark_info_table.
* Note: the returned pointer must not be freed in any case.
*
* @param WCHAR* file_path
*		The file path associated to the struct FileMarkInfo* wanted to be retrieved.
* @param WCHAR* app_path
*		The file path of the application associated to the struct FileMarkInfo* wanted to be removed.
*
* @return struct FileMarkInfo*
*		The pointer to the struct FileMarkInfo associated to the {file_path, app_path} passed as parameters or NULL if entry does not exist. Must not be freed on any circumstances.
**/
struct FileMarkInfo* getFMITableEntry(WCHAR* file_path, WCHAR* app_path) {
	struct FileMarkInfo* fmi = NULL;

	if (file_path == NULL || app_path == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL && 0 == wcscmp(file_path, file_mark_info_table[i]->file_path) && 0 == wcscmp(app_path, file_mark_info_table[i]->app_path)) {
			fmi = file_mark_info_table[i];
			PRINT("FMI found in the table\n");
			printFMI(fmi);
			break;
		}
	}
	return fmi;
}

/**
* Adds the struct FileMarkInfo* passed as parameter in the file_mark_info_table.
* If file_path and app_path values from parameter fields already exist in the table, the value is overwritten, if not, it is inserted on a free space.
* If there are not available slots for the struct to be added to the table, more space is allocated.
* In the case that no more space is allocable, NULL is returned.
* Note: the returned pointer must not be freed in any case.
* Note 2: you can still modify data from the pointer after adding it to the table.
* Note 3: this function cannot fail.
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
	struct FileMarkInfo** tmp_table = NULL;

	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] == NULL) {
			if (!found_empty) {
				empty_index = i;
				found_empty = TRUE;
			}
		} else if (0 == wcscmp(file_mark_info->file_path, file_mark_info_table[i]->file_path) && 0 == wcscmp(file_mark_info->app_path, file_mark_info_table[i]->app_path)) {
			if (file_mark_info_table[i] != file_mark_info) {
				destroyFMI(file_mark_info_table[i]);
				file_mark_info_table[i] = file_mark_info;
				PRINT("FMI overwritten in the table\n");
			} else {
				PRINT("Identical FMI in the table (no need to overwrite)\n");
			}
			printFMITable();
			return file_mark_info;
		}
	}

	// If an empty slot was not found, extend the table and save the index to the first empty slot
	if (!found_empty) {
		size_t fmi_table_size_increment = (file_mark_info_table_size == 0) ? FILE_MARK_INFO_TABLE_INITIAL_SIZE : FILE_MARK_INFO_TABLE_SIZE_INCREMENT;
		tmp_table = realloc(file_mark_info_table, (file_mark_info_table_size + fmi_table_size_increment) * sizeof(struct FileMarkInfo*));
		if (tmp_table == NULL) {
			printf("ERROR: allocating memory for file_mark_info_table.\n");
			printFMITable();
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

	printFMITable();
	return file_mark_info;
}

/**
* Removes old entries from the FMITable.
* To consider an entry old, it is checked if its last_closed member is FMI_TABLE_ENTRY_EXPIRATION_TIME secconds older than current time.
*
* @return int
*		The number of removed entries from the table. If an error occurs -1 is returned.
**/
int purgeFMITable() {
	if (testing_mode_on) {
		return;
	}
	int removed_entries = 0;
	time_t current_time = 0;

	// Get current time
	time(&current_time);

	// Iterate through all the table removing old entries
	for (size_t i = 0; i < file_mark_info_table_size; i++) {
		if (file_mark_info_table[i] != NULL) {
			if (file_mark_info_table[i]->last_closed != 0 && current_time > file_mark_info_table[i]->last_closed + FMI_TABLE_ENTRY_EXPIRATION_TIME) {
				PRINT("Purging fmi:");
				printFMI(file_mark_info_table[i]);
				destroyFMI(file_mark_info_table[i]);
				file_mark_info_table[i] = NULL;
				removed_entries++;
			}
		}
	}

	return removed_entries;
}

void threadPurge() {
	PRINT("FMI Table purging thread started...\n");
	while (1) {
		Sleep(THREAD_PURGE_SLEEP);
		//PRINT("Purging FMI Table...\n");
		purgeFMITable();
	}
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
	struct tm* time_info;

	if (fmi == NULL) {
		PRINT("FileMarkInfo --> NULL \n");
	} else {
		if (fmi->last_closed == TIME_CURRENTLY_OPEN) {
			time_info = NULL;
		} else {
			time_info = localtime(&(fmi->last_closed));
		}

		PRINT("FileMarkInfo:\n\t file_path: '%ws', \n\t app_path:  '%ws', \n\t buf_mark_lvl: %4d,   buf_frn: 0x%08X,   file_mark_lvl: %4d,   file_frn: 0x%08X,   last_closed: %s",
			fmi->file_path, fmi->app_path, fmi->buffer_mark_lvl, fmi->buffer_frn, fmi->file_mark_lvl, fmi->file_frn, (time_info == NULL) ? "currently open" : "");
		if (time_info != NULL)
			printDateNice(time_info);
		PRINT("\n");
	}
}

// TODO: improve implementation with better randomness
uint32_t createFRN() {
	if (testing_mode_on) {
		PRINT("testing mode on is active, forcing new FRN to 5\n");
		return 5;
	}
	//return 6;
	
	int frn = INVALID_FRN;
	static is_initialized = FALSE;
	uint8_t frn_buf[4];

	// Initialization, should only be called once
	if (!is_initialized) {
		srand(time(NULL));
	}

	while (frn == INVALID_FRN){
		//getRandom(frn_buf, 4);
		//frn = (uint32_t)(frn_buf[0] << 24 | frn_buf[1] << 16 | frn_buf[2] << 8 | frn_buf[3]);
		frn = rand();	// Obtains a pseudo-random integer between 0 and RAND_MAX
	}

	return frn;
}


void getRandom(uint8_t* buffer, int buf_size) {
	static BCRYPT_ALG_HANDLE alg_h = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	// While algorithm handle is not correctly initialized, continue trying
	while (NULL == alg_h || !BCRYPT_SUCCESS(status)) {
		status = BCryptOpenAlgorithmProvider(
			&alg_h,
			BCRYPT_RNG_ALGORITHM,
			NULL,
			0
		);
	}

	// Generate random number
	do {
		status = BCryptGenRandom(
			alg_h,
			buffer,
			buf_size,
			0
		);
	} while (!BCRYPT_SUCCESS(status));	// Continue generating random numbers if there is an error

	// BCryptCloseAlgorithmProvider(alg_h, 0)		// No need to close, due to alg_h is static

	return;
}

/**
* Creates the key that is really used for ciphering/deciphering using the FRN (specific to the file) and the composed key (mix of subkeys from the challenges).
* The returned pointer must be freed after use (note ptr->data must also be freed).
*
* @param struct KeyData* comp_key
*		The composed key to be mixed with the FRN. It is not modified.
* @param uint32_t frn
*		The FRN to be mixed with the composed key.
*
* @return struct KeyData*
*		A newly allocated key that has information of both the FRN and the composed key. It must be free after use (note ptr->data must also be freed).
**/
struct KeyData* createFileBufferKey(struct KeyData* comp_key, uint32_t frn) {
	struct KeyData *result = NULL;
	uint32_t* ptr_32 = NULL;

	result = malloc(1 * sizeof(struct KeyData));
	if (result != NULL) {
		result->expires = 0;												// Irrelevant, not used in the cipher dll
		result->size = comp_key->size + sizeof(frn);						// Size is addition of both comp_key size and FRN size
		result->data = malloc(result->size * sizeof(byte));					// Allocate memory for new key data
		memcpy_s(result->data, result->size, comp_key->data, comp_key->size);		// Copy the composed key data
		ptr_32 = (uint32_t*)&((result->data)[comp_key->size]);						// Get data ptr. Point to next available byte. Get mem addr. Convert into uint32_t*
		memcpy_s(ptr_32, (result->size) - comp_key->size, &frn, sizeof(frn));		// Append the FRN after the composed key data
	}

	return result;
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
		PRINT("Row %llu: ", i);
		printFMI(fmi);
	}
}

// Allocates memory
struct FileMarkInfo* createFMI(WCHAR* file_path, WCHAR* app_path, int8_t buffer_mark_lvl, uint32_t buffer_frn, int8_t file_mark_lvl, uint32_t file_frn, time_t last_closed) {
	if (file_path == NULL || app_path == NULL) {
		return NULL;
	}

	struct FileMarkInfo* fmi = NULL;
	fmi = malloc(1 * sizeof(struct FileMarkInfo));
	if (fmi != NULL) {
		wcscpy(fmi->file_path, file_path);
		wcscpy(fmi->app_path, app_path);
		fmi->buffer_mark_lvl = buffer_mark_lvl;
		fmi->file_mark_lvl = file_mark_lvl;
		fmi->buffer_frn = buffer_frn;
		fmi->file_frn = file_frn;
		fmi->last_closed = last_closed;
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
	WCHAR file_path[] = L"C:/test/00";
	WCHAR app_path[] = L"C:/appX/00";
	struct FileMarkInfo *fmi = NULL;
	int8_t file_mark_level = 0;
	int8_t buffer_mark_level = 0;

	srand(time(NULL));

	for (size_t times = 0; times < 2; times++) {
		if (times != 0) {
			PRINT("\n\nREPEATING EVERYTHING AGAIN!\n\n\n");
		}

		// Fill the table
		PRINT1("\nFilling the table...\n");
		for (size_t i = 0; i < 4; i++) {
			for (size_t j = 0; j < 4; j++) {
				file_path[9] = L'0' + i;
				app_path[9] = L'0' + j;
				addFMITableEntry(createFMI(file_path, app_path, UNKNOWN_MARK_LEVEL, INVALID_FRN, UNKNOWN_MARK_LEVEL, INVALID_FRN, 0));
			}
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();

		// Retrieve 5 random values
		PRINT1("\nRetrieving 5 random values from the table...\n");
		for (size_t i = 0; i < 5; i++) {
			file_path[9] = L'0' + rand() % 4;
			app_path[9] = L'0' + rand() % 4;
			fmi = getFMITableEntry(file_path, app_path);
			if (fmi != NULL) {
				PRINT2("Got entry: "); printFMI(fmi); PRINT("\n");
			}
		}

		// Add an already existing pointer to collide
		PRINT1("\nAdding an already existing pointer to collide in the table...\n");
		addFMITableEntry(fmi);

		// Add 5 random values that will collide. This time with different mark levels
		PRINT1("\nAdding 5 random values to the table...\n");
		for (size_t i = 0; i < 5; i++) {
			file_path[9] = L'0' + rand() % 4;
			app_path[9] = L'0' + rand() % 4;
			file_mark_level = (rand() % 3) - 1;		// Create random from -1, 0 or 1
			buffer_mark_level = (rand() % 3) - 1;	// Create random from -1, 0 or 1
			addFMITableEntry(createFMI(file_path, app_path, buffer_mark_level, createFRN(), file_mark_level, createFRN(), 0));
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();


		// Remove 5 random values
		PRINT1("\nRemoving 5 random values from the table...\n");
		for (size_t i = 0; i < 5; i++) {
			file_path[9] = L'0' + rand() % 4;
			app_path[9] = L'0' + rand() % 4;
			fmi = removeFMITableEntry(file_path, app_path);
			if (fmi != NULL) {
				PRINT2("Removed entry: "); printFMI(fmi); PRINT("\n");
				destroyFMI(fmi);
			}
		}

		// Print full table
		PRINT1("\nPrinting the table...\n");
		printFMITable();


		// Clear the table
		PRINT1("\nClearing the table...\n");
		for (size_t i = 0; i < 4; i++) {
			for (size_t j = 0; j < 4; j++) {
				file_path[9] = L'0' + i;
				app_path[9] = L'0' + j;
				fmi = removeFMITableEntry(file_path, app_path);
				destroyFMI(fmi);
			}
		}

	}
}


void invokeCipher(struct Cipher* p_cipher, LPVOID dst_buf, LPCVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key, uint32_t frn) {
	PRINT("Calling cipher dll......\n");

	PRINT("dst_buf = %p, src_buf = %p, buf_size = %d, frn = %lu\n", dst_buf, src_buf, buf_size, frn);

	if (frn == INVALID_FRN) {
		fprintf(stderr, "ERROR: cannot cipher with INVALID_FRN\n");
	}

	// FOR TESTING
	/*uint8_t last_frn_byte = (frn % 256);
	uint16_t current_value = 0;
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value + last_frn_byte) % 256);
	}*/

	struct KeyData* final_key = createFileBufferKey(composed_key, frn);
	if (final_key == NULL) {
		fprintf(stderr, "ERROR: could not allocate memory for the Key\n");
		return;
	}

	LPVOID src_buf_copy = NULL;
	if (dst_buf == src_buf) {
		src_buf_copy = malloc(buf_size);
		if (src_buf_copy == NULL) {
			printf("WARNING: could not allocate memory for the buffer to cipher\n");
			return;		// If copy of the buffer cannot be allocated, skip ciphering
		} else {
			memcpy(src_buf_copy, src_buf, buf_size);
		}
	}

	typedef int(__stdcall* cipher_func_type)(LPVOID, LPCVOID, DWORD, size_t, struct KeyData*);

	cipher_func_type cipher_func;
	int result;

	cipher_func = (cipher_func_type)GetProcAddress(p_cipher->lib_handle, "cipher");
	if (cipher_func != NULL) {
		if (src_buf_copy==NULL) {
			result = cipher_func(dst_buf, src_buf, buf_size, offset, final_key);	// this can be removed
		} else {
			result = cipher_func(dst_buf, src_buf_copy, buf_size, offset, final_key);
			free(src_buf_copy);
		}
		if (result != 0) {
			PRINT("WARNING: error ciphering\n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}

	PRINT("Done cipher dll......\n");
}

void invokeDecipher(struct Cipher* p_cipher, LPVOID dst_buf, LPCVOID src_buf, DWORD buf_size, size_t offset, struct KeyData* composed_key, uint32_t frn) {
	PRINT("Calling decipher dll......\n");

	PRINT("dst_buf = %p, src_buf = %p, buf_size = %d, frn = %lu\n", dst_buf, src_buf, buf_size, frn);

	if (frn == INVALID_FRN) {
		fprintf(stderr, "ERROR: cannot decipher with INVALID_FRN\n");
	}

	// FOR TESTING
	/*uint8_t last_frn_byte = (frn % 256);
	uint16_t current_value = 0;
	PRINT("\tbuf_size = %lu \n", buf_size);
	for (size_t i = 0; i < buf_size; i++) {
		current_value = (uint16_t)(((uint8_t*)src_buf)[i]);
		((byte*)dst_buf)[i] = (uint8_t)((current_value - last_frn_byte) % 256);
	}*/

	struct KeyData* final_key = createFileBufferKey(composed_key, frn);
	if (final_key == NULL) {
		fprintf(stderr, "ERROR: could not allocate memory for the Key\n");
		return;
	}

	LPVOID src_buf_copy = NULL;
	if (dst_buf == src_buf) {
		src_buf_copy = malloc(buf_size);
		if (src_buf_copy == NULL) {
			printf("WARNING: could not allocate memory for the buffer to decipher\n");
			return;		// If copy of the buffer cannot be allocated, skip deciphering
		} else {
			memcpy(src_buf_copy, src_buf, buf_size);
		}
	}

	typedef int(__stdcall* decipher_func_type)(LPVOID, LPCVOID, DWORD, size_t, struct KeyData*);

	decipher_func_type decipher_func;
	int result;

	decipher_func = (decipher_func_type)GetProcAddress(p_cipher->lib_handle, "decipher");
	if (decipher_func != NULL) {
		if (src_buf_copy==NULL) {
			result = decipher_func(dst_buf, src_buf, buf_size, offset, final_key);	// this can be removed
		} else {
			result = decipher_func(dst_buf, src_buf_copy, buf_size, offset, final_key);
			free(src_buf_copy);
		}

		if (result != 0) {
			PRINT("WARNING: error deciphering\n");
		}
	} else {
		PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", p_cipher->file_name, GetLastError());
	}

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
* Marks the buffer if it is possible with the specified mark level.
* If the input buffer cannot be marked, it is not modified and function returns UNKNOWN_MARK_LEVEL.
* Assumes that the input buffer is long enough (at least MARK_LENGTH bytes) and it is not marked.
* Allows any mark level from -127 to 127, but not UNKNOWN_MARK_LEVEL.
* In the case that 'mark_level' is 0, buffer is left unmodified.
*
* @param uint8_t* input
*		The buffer to be marked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be marked, it is not modified.
* @param int8_t mark_level
*		The mark level to set in the mark. If it is 0, the buffers is left unmodified. Its value cannot be UNKNOWN_MARK_LEVEL.
* @param uint32_t frn
*		The frn to set in the mark. Its value should not be INVALID_FRN unless mark_level is 0.
*
* @return int8_t
*		'mark_level' if the buffer was correctly marked (or not needed in case 'mark_level'==0). UNKNOWN_MARK_LEVEL otherwise.
**/
int8_t mark(uint8_t* input, int8_t mark_level, uint32_t frn) {
	// Check valid mark level
	if (mark_level == UNKNOWN_MARK_LEVEL) {
		return UNKNOWN_MARK_LEVEL;
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
	if (huffman_encode(input, &output, (uint32_t)MARK_LENGTH, &total_compressed_length) != 0 || total_compressed_length >= MARK_LENGTH - 1 - 4) {
		free(output);
		PRINT("Could not be marked\n");
		return UNKNOWN_MARK_LEVEL;
	}
	PRINT("Marked\n");

	// Save the FRN in the mark
	*((uint32_t*)(&(input[MARK_LENGTH - 1 - 4]))) = frn;	// Get uint8_t in pos 507. Take its memory addr. Make it uint32_t*. Indirect to use its contents


	// Set the mark level specified in the last byte
	input[MARK_LENGTH - 1] = mark_level;

	// Copy the compressed stream to the input buffer
	memcpy(input, output, total_compressed_length);

	// Fill the rest of the bytes with the filling sequence
	memcpy(&(input[total_compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - 1 - 4 - total_compressed_length); // -1 -4 due to the mark level needs one byte and FRN needs 4 to be stored

	// Free output buffer
	free(output);

	return mark_level;
}

/**
* Unmarks the buffer if it is possible and returns the mark level if the input buffer was marked.
* Allows any mark level from -127 to 127, but not 0 or UNKNOWN_MARK_LEVEL=-128.
*
* @param uint8_t* input
*		The buffer to be unmarked. Requires a minimum length of MARK_LENGTH bytes. In case it cannot be unmarked, it is not modified.
* @param uint32_t* frn
*		The frn to be updated with the contents from the mark. In case that input cannot be unmarked, it is set to INVALID_FRN.
*
* @return int8_t
*		The level of the mark if the buffer is marked. 0 otherwise.
**/
int8_t unmark(uint8_t* input, uint32_t* frn) {
	uint16_t decompressed_length = ((uint16_t*)(input))[0];	//*(uint32_t*)&input[0];
	uint16_t compressed_length = ((uint16_t*)(input))[1];	//*(uint32_t*)&input[1];
	uint16_t header_bit_length = ((uint16_t*)(input))[2] + (6 /*HEADER_BASE_SIZE*/ << 3); //*(uint16_t*)&input[4] + (6 /*HEADER_BASE_SIZE*/ << 3);
	int8_t mark_level = (int8_t)input[MARK_LENGTH - 1];
	uint32_t tmp_frn = *((uint32_t*)(&(input[MARK_LENGTH - 1 - 4])));	// Get uint8_t in pos 507. Take its memory addr. Make it uint32_t*. Indirect to take its contents

	PRINT("Checking mark. decompressed_length=%u, compressed_length=%u, header_bit_length=%u (%u bytes)\n", decompressed_length, compressed_length, header_bit_length, header_bit_length / 8 + ((header_bit_length % 8) ? 0 : 1));

	if (decompressed_length != (uint16_t)MARK_LENGTH) {
		*frn = INVALID_FRN;
		return 0;
	}

	if (compressed_length > MARK_LENGTH - 1 - 4) {	// -1 due to the mark level requires 1 Byte and FRN requires 4
		*frn = INVALID_FRN;
		return 0;
	}

	if (memcmp(&(input[compressed_length]), FILLING_SEQUENCE, MARK_LENGTH - 1 - 4 - compressed_length) != 0) {	// -1 due to the mark level requires 1 Byte
		*frn = INVALID_FRN;
		return 0;
	}

	if (mark_level == UNKNOWN_MARK_LEVEL || mark_level == 0) {
		*frn = INVALID_FRN;
		return 0;
	}

	if (tmp_frn == INVALID_FRN) {
		*frn = INVALID_FRN;
		return 0;
	}

	uint8_t* output = NULL;		// Allocated inside huffman_decode
	PRINT("Trying to unmark...\n");

	// Uncompress input
	if (huffman_decode(input, &output) != 0) {
		PRINT("Could not be unmarked\n");
		*frn = INVALID_FRN;
		return 0;
	}
	PRINT("Unmarked\n");

	// Copy FRN from the mark
	*frn = tmp_frn;

	// Copy decoded buffer into input buffer
	memcpy(input, output, MARK_LENGTH);

	// Free output buffer
	free(output);

	return mark_level;	// Allows mark_level to be any number from -127 to 127 (without the values 0 and UNKNOWN_MARK_LEVEL=-128)
}


BOOL getLogonFromTokenHandle(HANDLE token_handle, char* str_user, char* str_domain) {
	DWORD size = MAX_NAME;
	BOOL success = FALSE;
	DWORD length = 0;

	PTOKEN_USER p_token_user = NULL;
	//Verify the parameter passed in is not NULL.
	if (NULL == token_handle)
		goto LABEL_CLEANUP_GET_LOGON_NAME;

	if (!GetTokenInformation(
		token_handle,			// Handle to the access token
		TokenUser,				// Get information about the token's groups
		(LPVOID)p_token_user,	// Pointer to PTOKEN_USER buffer
		0,						// Size of buffer
		&length					// Receives required buffer size
	)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto LABEL_CLEANUP_GET_LOGON_NAME;

		p_token_user = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, length);

		if (p_token_user == NULL)
			goto LABEL_CLEANUP_GET_LOGON_NAME;
	}

	if (!GetTokenInformation(
		token_handle,			// Handle to the access token
		TokenUser,				// Get information about the token's groups
		(LPVOID)p_token_user,	// Pointer to PTOKEN_USER buffer
		length,					// Size of buffer
		&length					// Receives required buffer size
	)) {
		goto LABEL_CLEANUP_GET_LOGON_NAME;
	}
	SID_NAME_USE sid_type;
	char p_name[MAX_NAME];
	char p_domain[MAX_NAME];

	if (!LookupAccountSidA(NULL, p_token_user->User.Sid, p_name, &size, p_domain, &size, &sid_type)) {
		DWORD dwResult = GetLastError();
		if (dwResult == ERROR_NONE_MAPPED)
			strcpy(p_name, "NONE_MAPPED");
		else {
			fprintf(stderr, "LookupAccountSid Error %u\n", GetLastError());
		}
	} else {
		//printf("Current user is  %s\\%s\n", p_domain, p_name);

		strcpy(str_user, p_name);
		strcpy(str_domain, p_domain);
		success = TRUE;
	}

	LABEL_CLEANUP_GET_LOGON_NAME:
	if (p_token_user != NULL)
		HeapFree(GetProcessHeap(), 0, (LPVOID)p_token_user);
	return success;
}

int getUsernameByPID(const DWORD pid, char* str_user, char* str_domain) {
	HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (h_process == NULL)
		return E_FAIL;
	HANDLE token_handle = NULL;

	if (!OpenProcessToken(h_process, TOKEN_QUERY, &token_handle)) {
		CloseHandle(h_process);
		return E_FAIL;
	}
	BOOL result = getLogonFromTokenHandle(token_handle, str_user, str_domain);

	CloseHandle(token_handle);
	CloseHandle(h_process);
	return result ? S_OK : E_FAIL;
}


BOOL preCreateLogic(WCHAR file_path_param[], WCHAR* full_app_path, ULONG pid) {
	WCHAR* tmp_str;

	//PRINT("preCreateLogic!!\n");
	//PRINT(" - Operation: %d\n - File path: %ws\n - Buffer: %p\n - Bytes to do: %lu\n - Bytes done: %lu\n - Offset: %lld \n", op, file_path, *buffer, *bytes_to_do, **bytes_done, *offset);
	WCHAR* user = NULL;
	DWORD user_size = 0;
	LPDWORD p_user_size = &user_size;
	struct App* app = NULL;
	int result = 0;
	BOOL block_access = FALSE;	// Allow by default

	char* p_usr = NULL;
	char* p_dom = NULL;

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
			// OLD CHECKING METHOD
			// Initialize user name if not done yet
			/*if (user == NULL) {
				user_size = UNLEN + 1;
				user = malloc(sizeof(WCHAR) * user_size);
				if (user == NULL || GetUserNameW(user, p_user_size) == 0) {		// If the function fails, the return value is zero.
					PRINT("ERROR getting user name. Blocking access by default...\n");
					free(user);
					return TRUE;	// Block access
				} else {
					PRINT("UserName is: %ws\n", user);
				}
			}*/

			// Retrieve the logged on user
			p_usr = malloc(MAX_NAME);
			if (p_usr == NULL) {
				return TRUE;		// Block due to not being able to allocate memory for the username
			}
			p_dom = malloc(MAX_NAME);
			if (p_usr == NULL) {
				free(p_usr);
				return TRUE;		// Block due to not being able to allocate memory for the domain
			}
			getUsernameByPID(pid, p_usr, p_dom);
			PRINT("Obtained username and domain --> %s  -  %s\n", p_usr, p_dom);
			WCHAR p_w_usr[MAX_NAME] = { 0 };
			mbstowcs(p_w_usr, p_usr, MAX_NAME);
			free(p_usr);
			free(p_dom);

			// Check if user name is allowed for this folder and challenges are correct.
			for (size_t j = 0; j < _msize(ctx.parentals[i]->users) / sizeof(WCHAR*); j++) {
				if (wcscmp(ctx.parentals[i]->users[j], p_w_usr) == 0) {
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
			//PRINT("Blocking folder '%ws' due to user is not allowed\n", tmp_str);
			return TRUE;	// Block access
		}
	}

	// The path is not a parental controlled folder, then allow.
	return FALSE;	// Allow access
}




int preReadLogic(
	uint64_t file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, BOOL use_overlapped,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_buffer_length, LPDWORD* aux_read_length, LONGLONG* aux_offset
) {

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_buffer_length == NULL || orig_read_length == NULL || *orig_read_length == NULL ||
		orig_offset == NULL || file_path == NULL || app_path == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("preReadLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - App path: %ws\n",
		file_size, op, file_path, app_path);
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
	uint64_t file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped,
	LPVOID* orig_buffer, DWORD* orig_buffer_length, LPDWORD* orig_read_length, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_buffer_length, LPDWORD* aux_read_length, LONGLONG* aux_offset
) {

	// Parameter checking
	if (orig_buffer == NULL || *orig_buffer == NULL || orig_buffer_length == NULL || orig_read_length == NULL || *orig_read_length == NULL ||
		orig_offset == NULL || protection == NULL || file_path == NULL || app_path == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("postReadLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - App path: %ws\n",
		file_size, op, file_path, app_path);
	PRINT(" - Protection: %p (cipher->id: %s, key: %p)\n - Handle: %p\n",
		protection, protection->cipher->id, protection->key, handle);
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
	//int8_t buffer_mark_lvl = UNKNOWN_MARK_LEVEL;
	//int8_t file_mark_lvl = UNKNOWN_MARK_LEVEL;
	//uint32_t buffer_frn = INVALID_FRN;
	//uint32_t file_frn = INVALID_FRN;

	OVERLAPPED overlapped = { 0 };


	// SMALL FILES
	if (small_file) {
		goto POST_READ_CLEANUP;
	}


	// BIG FILES (file_size >= MARK_LENGTH)
	// Check the table
	fmi = getFMITableEntry(file_path, app_path);
	if (fmi == NULL) {
		fmi = createFMI(file_path, app_path, UNKNOWN_MARK_LEVEL, INVALID_FRN, UNKNOWN_MARK_LEVEL, INVALID_FRN, 0);
		addFMITableEntry(fmi);
	}

	// Offset further than the mark
	if (*orig_offset >= MARK_LENGTH) {
		//if (fmi->file_mark_lvl == UNKNOWN_MARK_LEVEL || frn == INVALID_FRN) {			// Read MARK_LENGTH first bytes and fill fmi->file_mark_lvl for next operations
		if (fmi->file_mark_lvl != 0) {
			// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
			if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				fprintf(stderr, "ERROR handle seeking in postWrite (error=%lu)\n", error_code);
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
				&overlapped)
				) {
				fprintf(stderr, "ERROR reading mark in postWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto POST_READ_CLEANUP;
			}
			if (extra_bytes_read != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto POST_READ_CLEANUP;
			}

			// Check mark and unmark the content and update in the table
			fmi->file_mark_lvl = unmark(extra_read_buffer, &(fmi->file_frn));

		}
	} else {	// Case where *orig_offset < MARK_LENGTH
		// Get if buffer is marked an unmark it. And update in the table
		fmi->file_mark_lvl = unmark(*aux_buffer, &(fmi->file_frn));
	}

	// Here the mark_lvl and fmi->file_mark_lvl variables should be filled (and equal)
	PRINT("The file mark level is %d\n", fmi->file_mark_lvl);
	if (fmi->file_mark_lvl != 1 && fmi->file_mark_lvl != 0 && fmi->file_mark_lvl != -1) {
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
			// Update fmi
			fmi->buffer_mark_lvl = fmi->file_mark_lvl;
			fmi->buffer_frn = fmi->file_frn;
			// Offset further than the mark
			if (*orig_offset >= MARK_LENGTH) {
				NOOP;
			}
			// Offset within the mark  (inicio < MARK_LENGTH)
			else {
				// Set the mark as it was
				mark(*aux_buffer, fmi->buffer_mark_lvl, fmi->buffer_frn);
				//if (UNKNOWN_MARK_LEVEL == mark(*aux_buffer, fmi->buffer_mark_lvl, fmi->buffer_frn) && NOTHING != op) {
				//	fprintf(stderr, "ERROR: could not mark buffer");
				//	error_code = -5;	// Avoid BROWSER to upload cleartext unmarkable files
				//	goto POST_READ_CLEANUP;
				//}
			}
			break;
		case CIPHER:
			switch (fmi->file_mark_lvl) {
				case 1:		// This should never happen
					printf("WARNING in postReadLogic: this should never happen (operation = %d, file_mark_lvl = %d)\n", op, fmi->file_mark_lvl);
					fprintf(stderr, "ERROR: cannot cipher a ciphered file");
					error_code = -4;	// Avoid BROWSER to upload cleartext files with fake mark
					goto POST_READ_CLEANUP;
				case 0:		// Cipher (and mark buffer if before MARK_LENGTH)
					// Update fmi
					if (fmi->buffer_mark_lvl == UNKNOWN_MARK_LEVEL) {
						fmi->buffer_mark_lvl = 1;
					}
					if (fmi->buffer_frn == INVALID_FRN) {
						fmi->buffer_frn = createFRN();
					}

					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key, fmi->buffer_frn);
					// Offset further than the mark
					if (*orig_offset >= MARK_LENGTH) {
						NOOP;
					}
					// Offset within the mark  (inicio < MARK_LENGTH)
					else {
						// Set the mark to level 1
						mark(*aux_buffer, 1, fmi->buffer_frn);
						//if (UNKNOWN_MARK_LEVEL == mark(*aux_buffer, 1, fmi->buffer_frn) && NOTHING != op) {
						//	fprintf(stderr, "ERROR: could not mark buffer");
						//	error_code = -5;	// Avoid BROWSER to upload cleartext unmarkable files
						//	goto POST_READ_CLEANUP;
						//}
					}
					break;
				case -1:	// Cipher (and leave without mark)
					// Update fmi
					fmi->buffer_mark_lvl = 0;
					fmi->buffer_frn = INVALID_FRN;

					invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key, fmi->file_frn);
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
			switch (fmi->file_mark_lvl) {
				case 1:		// Decipher (and leave without mark)
					// Update fmi
					fmi->buffer_mark_lvl = 0;
					fmi->buffer_frn = INVALID_FRN;

					invokeDecipher(protection->cipher, *aux_buffer, *aux_buffer, *aux_buffer_length, *aux_offset, composed_key, fmi->file_frn);
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
					// Update fmi
					fmi->buffer_mark_lvl = 0;
					fmi->buffer_frn = INVALID_FRN;

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
					printf("WARNING in postReadLogic: this should never happen (operation = %d, file_mark_lvl = %d)\n", op, fmi->file_mark_lvl);
					error_code = -4;	// Avoid double deciphered files
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
		fprintf(stderr, "ERROR: SetFilePointerEx in postRead (error_code = %lu)\n", error_code);
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
	uint64_t* file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped, UCHAR write_to_eof,
	LPCVOID* orig_buffer, DWORD* orig_bytes_to_write, LPDWORD* orig_bytes_written, LONGLONG* orig_offset,
	LPVOID* aux_buffer, DWORD* aux_bytes_to_write, LPDWORD* aux_bytes_written, LONGLONG* aux_offset
) {
	#pragma region COLLAPSABLE_REGION
	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL || aux_buffer == NULL || aux_bytes_to_write == NULL || aux_bytes_written == NULL || aux_offset == NULL ||
		file_path == NULL || app_path ==  NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("preWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - App path: %ws\n",
		*file_size, op, file_path, app_path);
	PRINT(" - Protection: %p (cipher->id: %s, key: %p)\n - Handle: %p\n - write_to_eof: %u\n",
		protection, protection->cipher->id, protection->key, handle, write_to_eof);
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

	struct FileMarkInfo* fmi = NULL;
	//int8_t file_mark_lvl = UNKNOWN_MARK_LEVEL;
	//uint32_t frn = INVALID_FRN;

	OVERLAPPED overlapped = { 0 };

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
		fmi = removeFMITableEntry(file_path, app_path);
		if (fmi != NULL) {
			free(fmi);
			fmi = NULL;
		}
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
	fmi = getFMITableEntry(file_path, app_path);
	printf("fmi %s null\n", fmi==NULL ? "IS" : "is NOT");
	if (op == DECIPHER) {	// FMI must have been created in postWrite of small file that grew into a big file
		if (fmi == NULL) {
			printf("WARNING in preWriteLogic: this should never happen (operation = %d, fmi = NULL)\n", op);
			error_code = ERROR_WRITE_FAULT;
			goto PRE_WRITE_CLEANUP;
		} else {
			if (fmi->buffer_mark_lvl == UNKNOWN_MARK_LEVEL) {
				printf("WARNING in preWriteLogic: this should never happen (operation = %d, mark_lvl = %d)\n", op, fmi->buffer_mark_lvl);
				error_code = ERROR_WRITE_FAULT;
				goto PRE_WRITE_CLEANUP;
			}
			if (fmi->buffer_mark_lvl == 1) {
				if ((*orig_offset < MARK_LENGTH && *orig_offset > 0)					// Only seccond part of the mark
					|| (*orig_offset == 0 && *orig_bytes_to_write < MARK_LENGTH)) {		// Only first part of the mark
					error_code = ERROR_WRITE_FAULT;
					goto PRE_WRITE_CLEANUP;
				}
			}
		}
	}
	if (op == CIPHER) {		// FMI must have been created in postWrite of small file that grew into a big file
		if (fmi == NULL) {
			printf("WARNING in preWriteLogic: this should never happen (operation = %d, fmi = NULL)\n", op);
			error_code = ERROR_WRITE_FAULT;
			goto PRE_WRITE_CLEANUP;
		} else {
			if (fmi->buffer_mark_lvl == UNKNOWN_MARK_LEVEL) {
				printf("WARNING in preWriteLogic: this should never happen (operation = %d, mark_lvl = %d)\n", op, fmi->buffer_mark_lvl);
				error_code = ERROR_WRITE_FAULT;
				goto PRE_WRITE_CLEANUP;
			}
			if (fmi->buffer_mark_lvl == -1) {
				if ((*orig_offset < MARK_LENGTH && *orig_offset > 0)					// Only seccond part of the mark
					|| (*orig_offset == 0 && *orig_bytes_to_write < MARK_LENGTH)) {		// Only first part of the mark
					error_code = ERROR_WRITE_FAULT;
					goto PRE_WRITE_CLEANUP;
				}
			}
		}
	}
	if (fmi == NULL) {
		fmi = createFMI(file_path, app_path, UNKNOWN_MARK_LEVEL, INVALID_FRN, UNKNOWN_MARK_LEVEL, INVALID_FRN, 0);
		addFMITableEntry(fmi);
	}

	// Offset further than the mark
	if (*orig_offset >= MARK_LENGTH) {
		// Buffer starts and ends on same place

		// Check if fmi has unknown values to update them
		if (fmi->file_mark_lvl == UNKNOWN_MARK_LEVEL || (fmi->file_mark_lvl != 0 && fmi->file_frn == INVALID_FRN)) {
			// Read the beginning of the file to get the FRN

			// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
			if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
				error_code = GetLastError();
				fprintf(stderr, "ERROR handle seeking (error=%lu)\n", error_code);
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
				NULL)//&overlapped)
				) {
				fprintf(stderr, "ERROR reading mark inside preWrite!!!\n");
				error_code = ERROR_READ_FAULT;
				goto READ_INSIDE_WRITE_CLEANUP;
			}
			if (bytes_read != MARK_LENGTH) {
				error_code = ERROR_READ_FAULT;
				goto READ_INSIDE_WRITE_CLEANUP;
			}
			fmi->file_mark_lvl = unmark(read_buffer, &(fmi->file_frn));	// Gets frn
		}
	}
	// Offset within the mark  (orig_offset < MARK_LENGTH)
	else {
		// Modif buffers
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
			fprintf(stderr, "ERROR handle seeking (error=%lu)\n", error_code);
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
			NULL)//&overlapped)
			) {
			fprintf(stderr, "ERROR reading mark inside preWrite!!!\n");
			error_code = ERROR_READ_FAULT;
			goto READ_INSIDE_WRITE_CLEANUP;
		}
		if (bytes_read != MARK_LENGTH) {
			error_code = ERROR_READ_FAULT;
			goto READ_INSIDE_WRITE_CLEANUP;
		}

		// As we have already obtained the file mark level, we can check/update the level stored in the table. TODO: check if check is really needed
		uint32_t tmp_frn = INVALID_FRN;
		int8_t tmp_mark_lvl = UNKNOWN_MARK_LEVEL;
		tmp_mark_lvl = unmark(read_buffer, &tmp_frn);
		if (fmi->file_mark_lvl == UNKNOWN_MARK_LEVEL) {
			// Update in the table
			fmi->file_mark_lvl = tmp_mark_lvl;
			fmi->file_frn = tmp_frn;
		} else if (fmi->file_mark_lvl != tmp_mark_lvl) {
			fprintf(stderr, "ERROR in preWriteLogic: file mark level in the table (%d) is different from the one actually read (%d)\n", fmi->file_mark_lvl, tmp_mark_lvl);
			error_code = -2;
			goto READ_INSIDE_WRITE_CLEANUP;
		}

		// Here the file_mark_lvl and fmi->file_mark_lvl variables should be filled (and equal)
		PRINT("The file mark level is %d\n", fmi->file_mark_lvl);
		if (fmi->file_mark_lvl != 1 && fmi->file_mark_lvl != 0 && fmi->file_mark_lvl != -1) {
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
		#pragma warning(suppress: 6386)
		memcpy(*aux_buffer, read_buffer, MARK_LENGTH);										// Copy bytes from file to the buffer
		memcpy(&(((byte*)*aux_buffer)[*orig_offset]), *orig_buffer, *orig_bytes_to_write);	// Fill (and partially overwrite) with the original bytes form application

	}

	if (fmi->buffer_mark_lvl == UNKNOWN_MARK_LEVEL || (fmi->buffer_mark_lvl != 0 && fmi->buffer_frn == INVALID_FRN)) {
		fmi->buffer_mark_lvl = 0;
		fmi->buffer_frn = INVALID_FRN;
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
			if (fmi->buffer_mark_lvl != fmi->file_mark_lvl) {
				printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
				error_code = -3;
				goto PRE_WRITE_CLEANUP;
			}
			// Update fmi
			fmi->buffer_mark_lvl = fmi->file_mark_lvl;
			fmi->buffer_frn = fmi->file_frn;

			// Do not modify the whole buffer
			NOOP;

			// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
			if (*orig_offset < MARK_LENGTH) {
				// Set the mark level to 'fmi->buffer_mark_lvl' (that should be the same than it was in the file)
				mark(*aux_buffer, fmi->file_mark_lvl, fmi->file_frn);
				//if (UNKNOWN_MARK_LEVEL == mark(*aux_buffer, fmi->file_mark_lvl, fmi->file_frn) && NOTHING != op) {
				//	fprintf(stderr, "ERROR: could not mark buffer");
				//	error_code = -5;	// Avoid BROWSER to upload cleartext unmarkable files
				//	goto PRE_WRITE_CLEANUP;
				//}
			}

			break;
		case CIPHER:
			switch (fmi->buffer_mark_lvl) {
				case 1:		// Do nothing (leave mark as it was --> mark buffer if before MARK_LENGTH). Note this should never happen
					/*// Check that write_buffer_mark_level is the same than file_mark_level (1)
					if (fmi->file_mark_lvl != UNKNOWN_MARK_LEVEL && fmi->file_mark_lvl != 1) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					} else {
						printf("NOTE in preWriteLogic: this operation was probably not intended. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
					}

					// Do not modify the whole buffer
					NOOP;

					// Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
					if (*orig_offset < MARK_LENGTH) {
						// Set the mark level to 'fmi->buffer_mark_lvl' (that should be the same than it was in the file)
						mark(*aux_buffer, fmi->buffer_mark_lvl);
					}

					break;*/
					printf("WARNING in preWriteLogic: this should never happen (operation = %d, buffer_mark_lvl = %d)\n", op, fmi->buffer_mark_lvl);
					fprintf(stderr, "ERROR: cannot cipher a ciphered file");
					error_code = -1;		// Avoid writting cleartext files in pendrive/syncfolder with fake marks
					goto PRE_WRITE_CLEANUP;
				case 0:		// Cipher (and mark buffer if before MARK_LENGTH)
					// Check that write_buffer_mark_level is one less than file_mark_level
					if (/*fmi->file_mark_lvl != UNKNOWN_MARK_LEVEL && */fmi->file_mark_lvl != 1) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Cipher the whole buffer
					if (*orig_offset >= MARK_LENGTH) {
						invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *orig_bytes_to_write, *orig_offset, composed_key, fmi->file_frn);
					} else { // Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
						invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset]), &(((byte*)*aux_buffer)[*orig_offset]), *orig_bytes_to_write, *orig_offset, composed_key, fmi->file_frn);

						// Set the mark to level 1
						mark(*aux_buffer, 1, fmi->file_frn);
						//if (UNKNOWN_MARK_LEVEL == mark(*aux_buffer, 1, fmi->file_frn && NOTHING != op)) {
						//	fprintf(stderr, "ERROR: could not mark buffer");
						//	error_code = -5;	// Avoid BROWSER to upload cleartext unmarkable files
						//	goto PRE_WRITE_CLEANUP;
						//}
					}

					break;
				case -1:	// Cipher (and leave without mark)
					// Check that write_buffer_mark_level is one less than file_mark_level
					if (/*fmi->file_mark_lvl != UNKNOWN_MARK_LEVEL &&*/ fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Cipher the whole buffer
					if (*orig_offset >= MARK_LENGTH) {
						invokeCipher(protection->cipher, *aux_buffer, *aux_buffer, *orig_bytes_to_write, *orig_offset, composed_key, fmi->buffer_frn);
					} else { // Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
						invokeCipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset]), &(((byte*)*aux_buffer)[*orig_offset]), *orig_bytes_to_write, *orig_offset, composed_key, fmi->buffer_frn);

						NOOP;	// Set the mark to 0 (which is, don't mark)
					}

					break;
			}
			break;
		case DECIPHER:
			switch (fmi->buffer_mark_lvl) {
				case 1:		// Decipher (and leave without mark)
					// Check that write_buffer_mark_level is one more than file_mark_level
					if (/*fmi->file_mark_lvl != UNKNOWN_MARK_LEVEL && */fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
						error_code = -3;
						goto PRE_WRITE_CLEANUP;
					}

					// Decipher the whole buffer
					if (*orig_offset >= MARK_LENGTH) {
						invokeDecipher(protection->cipher, *aux_buffer, *aux_buffer, *orig_bytes_to_write, *orig_offset, composed_key, fmi->buffer_frn);
					} else { // Only if Offset is within the mark  (*orig_offset < MARK_LENGTH)
						invokeDecipher(protection->cipher, &(((byte*)*aux_buffer)[*orig_offset]), &(((byte*)*aux_buffer)[*orig_offset]), *orig_bytes_to_write, *orig_offset, composed_key, fmi->buffer_frn);

						NOOP;	// Set the mark to 0 (which is, don't mark)
					}

					break;
				case 0:		// Do nothing (and leave without mark)
					// Check that write_buffer_mark_level is the same as file_mark_level
					if (/*fmi->file_mark_lvl != UNKNOWN_MARK_LEVEL && */fmi->file_mark_lvl != 0) {
						printf("WARNING in preWriteLogic: inconsistent mark levels. This should never happen (operation=%d, fmi->buffer_mark_lvl=%d, fmi->file_mark_lvl=%d)\n", op, fmi->buffer_mark_lvl, fmi->file_mark_lvl);
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
					printf("WARNING in preWriteLogic: this should never happen (operation = %d, buffer_mark_lvl = %d)\n", op, fmi->buffer_mark_lvl);
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
	uint64_t* file_size, enum Operation op, WCHAR* file_path, WCHAR* app_path, struct Protection* protection, HANDLE handle, BOOL use_overlapped, UCHAR write_to_eof,
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

	struct FileMarkInfo* fmi = NULL;
	//int8_t mark_lvl = UNKNOWN_MARK_LEVEL;
	//uint32_t frn = INVALID_FRN;

	OVERLAPPED overlapped_first_part = { 0 };
	OVERLAPPED overlapped_second_part = { 0 };
	overlapped_second_part.Offset = (DWORD)MARK_LENGTH;
	overlapped_second_part.OffsetHigh = (DWORD)MARK_LENGTH >> 32;
	OVERLAPPED overlapped_write = { 0 };

	if (orig_buffer == NULL || *orig_buffer == NULL || orig_bytes_to_write == NULL || orig_bytes_written == NULL || *orig_bytes_written == NULL ||
		orig_offset == NULL || protection == NULL || file_path == NULL || app_path == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	PRINT("postWriteLogic!!\n");
	PRINT(" - File size: %llu\n - Operation: %d\n - File path: %ws\n - App path: %ws\n",
		*file_size, op, file_path, app_path);
	PRINT(" - Protection: %p (cipher->id: %s, key: %p)\n - Handle: %p\n - write_to_eof: %u\n",
		 protection, protection->cipher->id, protection->key, handle, write_to_eof);
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
		fprintf(stderr, "ERROR getting file size in postWrite (error_code = %d)\n", error_code);
		return error_code;
	}

	// Check files that became larger than MARK_LENGTH
	if (*file_size < MARK_LENGTH && new_file_size >= MARK_LENGTH) {
		// Add fmi to the table. At this point file mark/frn are equal to buffer mark/frn, but will be changed afterwards if needed
		fmi = createFMI(file_path, app_path, UNKNOWN_MARK_LEVEL, INVALID_FRN, UNKNOWN_MARK_LEVEL, INVALID_FRN, 0);
		addFMITableEntry(fmi);

		PRINT("File has grown enough to admit mark (prev file_size = %llu, new file_size = %llu)\n", *file_size, new_file_size);

		// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
		if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
			error_code = GetLastError();
			fprintf(stderr, "ERROR handle seeking in postWrite (error=%lu)\n", error_code);
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
			NULL)//&overlapped_first_part)
			) {
			fprintf(stderr, "ERROR reading mark in postWrite!!!\n");
			error_code = ERROR_READ_FAULT;
			goto POST_WRITE_CLEANUP;
		}
		if (bytes_done != MARK_LENGTH) {
			error_code = ERROR_READ_FAULT;
			goto POST_WRITE_CLEANUP;
		}

		// Check mark and unmark the content and update fmi
		fmi->buffer_mark_lvl = unmark(buffer1, &(fmi->buffer_frn));

		// CASE OF NOTHING
		if (op == NOTHING) {
			fmi->file_mark_lvl = fmi->buffer_mark_lvl;
			fmi->file_frn = fmi->buffer_frn;
		}

		// CASE OF CIPHER
		if (op == CIPHER) {
			if (fmi->buffer_mark_lvl != 1) {
				// Update fmi
				if (fmi->buffer_mark_lvl == -1) {
					fmi->file_mark_lvl = 0;
					fmi->file_frn = INVALID_FRN;
				}
				if (fmi->buffer_mark_lvl == 0) {
					fmi->file_mark_lvl = 1;
					fmi->file_frn = createFRN();
				}

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
					NULL)//&overlapped_second_part)
					) {
					fprintf(stderr, "ERROR reading mark in postWrite!!!\n");
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
					if (fmi->buffer_mark_lvl == -1) {
						invokeCipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key, fmi->buffer_frn);
						invokeCipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key, fmi->buffer_frn);
					}
					if (fmi->buffer_mark_lvl == 0) {
						invokeCipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key, fmi->file_frn);
						invokeCipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key, fmi->file_frn);
					}
				} else {
					fprintf(stderr, "ERROR composing key in postWriteLogic (%d)", result);
					error_code = result;
					goto POST_WRITE_CLEANUP;
				}

				// Mark in the case of buffer_mark_lvl==0. If buffer_mark_lvl==-1, mark has already been taken out and there is no need to add another one
				/*if (fmi->buffer_mark_lvl == -1) {
					NOOP;
				}*/
				if (fmi->buffer_mark_lvl == 0) {
					mark(buffer3, fmi->file_mark_lvl, fmi->file_frn);
					//if (UNKNOWN_MARK_LEVEL == mark(buffer3, fmi->file_mark_lvl, fmi->file_frn) && NOTHING != op) {
					//	fprintf(stderr, "ERROR: could not mark buffer");
					//	error_code = -5;	// Avoid BROWSER to upload cleartext unmarkable files
					//	goto POST_WRITE_CLEANUP;
					//}
				}

				// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
				if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
					error_code = GetLastError();
					fprintf(stderr, "ERROR handle seeking in postWrite (error=%lu)\n", error_code);
					goto POST_WRITE_CLEANUP;
				}

				// Write new content to file
				if (!WriteFile(
					handle,
					buffer3,
					new_file_size,
					&bytes_done,
					(use_overlapped ? &overlapped_write : NULL))
					) {
					fprintf(stderr, "ERROR writing mark in postWrite!!!\n");
					error_code = ERROR_WRITE_FAULT;
					goto POST_WRITE_CLEANUP;
				}
				if (bytes_done != new_file_size) {
					error_code = ERROR_WRITE_FAULT;
					goto POST_WRITE_CLEANUP;
				}
			} else {
				printf("WARNING in postWriteLogic: this should never happen (operation = %d, buffer_mark_lvl = %d)\n", op, fmi->buffer_mark_lvl);
				fprintf(stderr, "ERROR: cannot cipher a ciphered file");
				error_code = ERROR_WRITE_FAULT;
				goto POST_WRITE_CLEANUP;
			}
		}


		// CASE OF DECIPHER
		if (op == DECIPHER) {
			if (fmi->buffer_mark_lvl == 1) {
				// The file mark must be one level less
				fmi->file_mark_lvl = 0;
				fmi->file_frn = INVALID_FRN;

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
					NULL)//&overlapped_second_part)
					) {
					fprintf(stderr, "ERROR reading mark in postWrite!!!\n");
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
					invokeDecipher(protection->cipher, buffer3, buffer1, MARK_LENGTH, 0, composed_key, fmi->buffer_frn);
					invokeDecipher(protection->cipher, &(((byte*)buffer3)[MARK_LENGTH]), buffer2, (new_file_size - MARK_LENGTH), MARK_LENGTH, composed_key, fmi->buffer_frn);
				} else {
					fprintf(stderr, "ERROR composing key in postWriteLogic (%d)", result);
					error_code = result;
					goto POST_WRITE_CLEANUP;
				}

				// No need to mark
				NOOP;

				// Make handle point to the beginning of the file (distanceToMove = 0, FILE_BEGIN)		//distanceToMove.QuadPart = 0;
				if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
					error_code = GetLastError();
					fprintf(stderr, "ERROR handle seeking in postWrite (error=%lu)\n", error_code);
					goto POST_WRITE_CLEANUP;
				}

				// Write new content to file
				if (!WriteFile(
					handle,
					buffer3,
					new_file_size,
					&bytes_done,
					(use_overlapped ? &overlapped_write : NULL))
					) {
					fprintf(stderr, "ERROR writing mark in postWrite!!!\n");
					error_code = ERROR_WRITE_FAULT;
					goto POST_WRITE_CLEANUP;
				}
				if (bytes_done != new_file_size) {
					error_code = ERROR_WRITE_FAULT;
					goto POST_WRITE_CLEANUP;
				}
			} else {
				if (fmi->buffer_mark_lvl == 0) {
					// Update fmi
					fmi->file_mark_lvl = 0;
					fmi->file_frn = INVALID_FRN;
				}
				if (fmi->buffer_mark_lvl == -1) {
					fprintf(stderr, "ERROR: cannot decipher a deciphered file\n");
					error_code = ERROR_WRITE_FAULT;
					goto POST_WRITE_CLEANUP;
				}
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
		PRINT("File is already big or it is still small (prev file_size = %llu, new file_size = %llu)\n", *file_size, new_file_size);
	}

	// Make handle point to where it should based on the read the application asked to do
	distanceToMove.QuadPart = *orig_offset + **orig_bytes_written;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR: SetFilePointerEx in postWrite(error_code = %lu)\n", error_code);
	}

	PRINT("Postwrite returns with error_code = %d \n", error_code);

	return error_code;
}


int postCleanupLogic(WCHAR* file_path, WCHAR* app_path) {
	//PRINT("postCleanupLogic!!\n");
	//PRINT(" - File path: %ws\n - App path: %ws\n",
	//	file_path, app_path);

	struct FileMarkInfo* fmi = NULL;
	time_t current_time = 0;

	// Get current time
	time(&current_time);

	fmi = getFMITableEntry(file_path, app_path);
	//printFMI(fmi);
	if (fmi != NULL) {
		fmi->last_closed = current_time;
	}

	return 0;
}


/**
* Marks, checks and unmarks a predefined buffer and prints it in the screen to see if mark and internal huffman functions are working well.
**/
/*void testMarkOLD() {		// NOT WORKING due to OLD marking is used
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
}*/
