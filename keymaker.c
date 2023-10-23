/*
* SecureWorld file keymaker.c
* Contains the key making and obtaining functions, which check validity of the key and invoke the challenges using dlls if necessary.

Nokia mayo 2021
*/

/////  FILE INCLUDES  /////

#include "keymaker.h"
#include "main.h"




/////  FUNCTION PROTOTYPES  /////

struct KeyData getSubkey(struct ChallengeEquivalenceGroup* challenge_group, BOOL* had_expired);




/////  FUNCTION DEFINITIONS  /////

/**
 * Updates the composed_key parameter with a time-valid full key depending on the given challenge groups.
 * The created KeyData is special:
 *	- It only has size and data fileds filled.
 *	- It must never be modified and therefore its critical_section field is empty (not needed).
 *	- As this function ensures a time-valid key that should not be stored, its expires field is empty (not needed).
**/
int makeComposedKey(struct ChallengeEquivalenceGroup** challenge_groups, struct KeyData* composed_key) {
	// TO DO (possible improvements)
	// Idea 0: remove memory leakage doing corresponding free() on errors.
	// Idea 1: may be better to limit the max number of keys and make it array.
	// Idea 2: remove keys local var and getSubkey() twice instead of saving the result. This may be worse, must be tested.
	// Idea 3: remove keys local var and assume subkey sizes are fixed and already initialized. Compute the size directly and then call getSubkey() once.
	// Idea 4: fill critical_section and expires fields and allow to save it in folders struct.

	struct KeyData* keys = NULL;
	int num_groups = 0;
	int index = 0;

	// Param checking
	if (composed_key == NULL)		return ERROR_INVALID_PARAMETER;	// One or more params are null
	if (challenge_groups == NULL)	return -2;						// There are no groups, cannot make a key

	num_groups = _msize(challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*);
	if (num_groups <= 0)			return -2;						// There are no groups, cannot make a key

	// Allocate local variable keys to hold copies of all subkeys
	keys = malloc(sizeof(struct KeyData) * num_groups);			// Can be changed to array limmiting the maximum number of challenge groups
	if (keys == NULL)				return ERROR_NOT_ENOUGH_MEMORY;	// Cannot allocate memory

	if (composed_key->data != NULL) {
		free(composed_key->data);
	}
	composed_key->size = 0;

	// Get composed key size and allocate corresponding memory in data member
	for (size_t i = 0; i < num_groups; i++) {
		keys[i] = getSubkey(challenge_groups[i], NULL);
		composed_key->size += keys[i].size;
	}
	composed_key->data = calloc(composed_key->size, sizeof(byte));
	if (composed_key->data == NULL)	return ERROR_NOT_ENOUGH_MEMORY;	// Cannot allocate memory

	// Compose the key
	for (size_t i = 0; i < num_groups; i++) {
		memcpy(&(composed_key->data[index]), keys[i].data, keys[i].size);
		index += keys[i].size;
	}

	// Free local variable keys
	free(keys);

	PRINT_HEX(composed_key->data, composed_key->size);

	return ERROR_SUCCESS;	// Success
}


/**
 Returns a time-valid subkey for the given challenge group. In case key expired, forces computation at the moment.
 */
struct KeyData getSubkey(struct ChallengeEquivalenceGroup* challenge_group, BOOL* had_expired) {
	struct KeyData keydata = { 0 };
	time_t current_time = 0;
	int group_length = 0;
	typedef int(__stdcall* init_func_type)(struct ChallengeEquivalenceGroup*, struct Challenge*);
	typedef int(__stdcall* exec_ch_func_type)();
	typedef void(__stdcall* set_per_exec_func_type)(BOOL);
	BOOL need_to_reinit = FALSE;

	int result = 0;
	init_func_type init_func;
	exec_ch_func_type exec_ch_func;
	set_per_exec_func_type set_per_exec_func;


	// In fact, it is irrelevant if it cannot be obtained. It just forces compute key in that case
	/*if (time(&current_time) == -1) {
		fprintf(stderr, "Error while getting current time in getSubkey().\n");
	}*/

	// Get current time
	time(&current_time);

	// Get a copy of the key data struct
	EnterCriticalSection(&(challenge_group->subkey->critical_section));
	keydata = *(challenge_group->subkey);
	LeaveCriticalSection(&(challenge_group->subkey->critical_section));

	printf("Current time: %lld\n", current_time);
	printf("Expiration time: %lld\n", keydata.expires);
	printf("Difftime(a, b): %f\n", difftime(current_time, keydata.expires));
	// Check if key expired and needs to be computed now
	if (difftime(current_time, keydata.expires) > 0) {		// Means: current_time > keydata.expires
		if (NULL != had_expired) {
			*had_expired = TRUE;
		}

		// TODO: start on the challenge that executed correctly on the init() instead of always starting on index 0

		// Iterate over challenges until one returns that it could be executed
		group_length = _msize(challenge_group->challenges) / sizeof(struct Challenge*);
		printf("group_length: %d\n", group_length);
		for (size_t ch_idx = 0; ch_idx < group_length; ch_idx++) {
			printf("ch_idx: %lld\n", ch_idx);
			// Check library was loaded
			if (INVALID_HANDLE_VALUE != challenge_group->challenges[ch_idx]->lib_handle) {

				// To ensure everything is working, run init() function
				init_func = (init_func_type)GetProcAddress(challenge_group->challenges[ch_idx]->lib_handle, "init");
				if (init_func != NULL) {
					printf("init_func is NOT null\n");
					result = init_func(challenge_group, challenge_group->challenges[ch_idx]);
					if (result == 0) {
						result = execChallengeFromMainThread(challenge_group, challenge_group->challenges[ch_idx]);
						if (result != 0) {
							PRINT("WARNING: error trying to execute the challenge '%ws'\n", challenge_group->challenges[ch_idx]->file_name);
						}
						else {
							break;		// Stop executing more challenges in the group when one is already working
						}
					} else if (result != 0) {
						PRINT("WARNING: error trying to init the challenge '%ws'\n", challenge_group->challenges[ch_idx]->file_name);
					} else {
						// Run executeChallenge() function
						printf("VALID handle value\n");
						exec_ch_func = (exec_ch_func_type)GetProcAddress(challenge_group->challenges[ch_idx]->lib_handle, "executeChallenge");
						printf("exec_ch_func holds now the addr of the func executeChallenge\n");
						if (exec_ch_func != NULL) {
							printf("exec_ch_func is NOT null\n");
							result = exec_ch_func();
							printf("result of exec_ch_func is %d\n", result);
							if (result != 0) {
								PRINT("WARNING: error trying to execute the challenge '%ws'\n", challenge_group->challenges[ch_idx]->file_name);
							} else {
								break;		// Stop executing more challenges in the group when one is already working
							}
						} else {
							PRINT("WARNING: error accessing the address to the executeChallenge() function of the challenge '%ws' (error: %d)\n", challenge_group->challenges[ch_idx]->file_name, GetLastError());
						}
					}
				} else {
					PRINT("WARNING: error accessing the address to the init() function of the challenge '%ws' (error: %d)\n", challenge_group->challenges[ch_idx]->file_name, GetLastError());
				}

				// To ensure challenge thread does not execute again, set periodic execution to false
				/*set_per_exec_func = (set_per_exec_func_type)GetProcAddress(challenge_group->challenges[ch_idx]->lib_handle, "setPeriodicExecution");
				if (set_per_exec_func != NULL) {
					printf("set_per_exec_func is NOT null\n");
					set_per_exec_func(FALSE);
				} else {
					PRINT("WARNING: error accessing the address to the setPeriodicExecution() function of the challenge '%ws' (error: %d)\n", challenge_group->challenges[ch_idx]->file_name, GetLastError());
				}*/
			} else {
				printf("INVALID HANDLE VALUE for the dll!!\n");
			}

			// Check if it is the last challenge in the group and reset ch_idx to restart the loop
			if (ch_idx == group_length - 1) {
				printf("update ch_idx\n");
				ch_idx = -1;        // ch_idx will become 0 With the update in the loop (ch_idx++)
				Sleep(1);           // sleep 1ms so process does not starve others
			}
		}
	}

	// Get a copy of the (updated) key data struct
	EnterCriticalSection(&(challenge_group->subkey->critical_section));
	keydata = *(challenge_group->subkey);
	LeaveCriticalSection(&(challenge_group->subkey->critical_section));

	return keydata;
}


/**
  Updates the block_access variable depending on the given challenge groups.
 */
int makeParentalKey(struct ChallengeEquivalenceGroup** challenge_groups, BOOL *block_access) {
	BOOL had_expired = TRUE;
	int num_groups = 0;
	int index = 0;
	struct KeyData curr_key = { 0 };
	*block_access = FALSE;

	if (block_access == NULL)		return ERROR_INVALID_PARAMETER;	// block_access is NULL

	if (challenge_groups == NULL)	return ERROR_SUCCESS;	// No challenge groups (same as if all challenges are passed)

	num_groups = _msize(challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*);
	if (num_groups <= 0)			return ERROR_SUCCESS;	// No challenge groups (same as if all challenges are passed)

	// Make the key
	for (size_t i = 0; i < num_groups; i++) {
		had_expired = FALSE;
		PRINT("challenge_group->id = %s\n", challenge_groups[i]->id);
		curr_key = getSubkey(challenge_groups[i], &had_expired);
		if (had_expired) {
			updateParentalChSuccessFile(challenge_groups[i], curr_key.data[0]);
		}
		PRINT("curr_key = %d \n", curr_key.data[0]);
		*block_access |= curr_key.data[0];
	}

	return ERROR_SUCCESS;	// Success
}


void updateParentalChSuccessFile(struct ChallengeEquivalenceGroup* ch_group, byte ch_result) {
	WCHAR* file_path = NULL;
	WCHAR* folder_path = NULL;
	HANDLE handle = INVALID_HANDLE_VALUE;
	size_t folder_path_len = 0;
	size_t ch_group_id_len = 0;
	size_t file_path_len = 0;
	size_t add_extra_char = 0;

	// Get the folder path from the environment variable
	folder_path = _wgetenv(ENVVAR_MINIFILTER_CONFIG_FOLDER);
	if (NULL == folder_path) {
		folder_path = L"";
	}

	// Get full filepath length
	folder_path_len = wcslen(folder_path);
	ch_group_id_len = strlen(ch_group->id);
	file_path_len = folder_path_len + ch_group_id_len;
	if (0 != folder_path_len) {
		if (L'\\' != folder_path[folder_path_len - 1] && L'/' != folder_path[folder_path_len - 1]) {
			add_extra_char = 1;
			file_path_len++; // Adds 1 to the length for putting together folder and filename if needed
		}
	}

	// Allocate full filepath
	file_path = malloc((file_path_len + 1) * sizeof(WCHAR));
	if (NULL == file_path) {
		fprintf(stderr, "ERROR: could not allocate memory for the filepath.\n");
		goto UPDATE_PARENTAL_CH_SUCCESS_FILE_CLEANUP;
	}

	// Compose the full filepath
	wcscpy(file_path, folder_path);
	if (add_extra_char) {
		wcscat(file_path, L"\\");
	}
	mbstowcs(&(file_path[folder_path_len + add_extra_char]), ch_group->id, ch_group_id_len + 1);
	PRINT("Ch equivalent group file: %ws\n", file_path);


	// Create or remove the file accordingly to the result
	if (0 == ch_result) { // no block --> create file
		// Open the file with a wide character path/filename. Creates a new empty file always
		handle = CreateFileW(file_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == handle) {
			fprintf(stderr, "WARNING: could not create empty file (%ws).\n", file_path);
			goto UPDATE_PARENTAL_CH_SUCCESS_FILE_CLEANUP;
		}
		PRINT("File %ws created\n", file_path);
	} else { // block --> delete file
		if (!(DeleteFileW(file_path))) {
			fprintf(stderr, "WARNING: could not delete empty file (%ws).\n", file_path);
			goto UPDATE_PARENTAL_CH_SUCCESS_FILE_CLEANUP;
		}
	}

UPDATE_PARENTAL_CH_SUCCESS_FILE_CLEANUP:
	if (NULL != file_path) {
		free(file_path);
	}
	if (INVALID_HANDLE_VALUE != handle) {
		CloseHandle(handle);
	}

	return; // Just return with no retry/waiting. This will make minifilter have undesired behaviour, but it is a secondary service.
}