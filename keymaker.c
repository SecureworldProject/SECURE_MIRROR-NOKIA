/*
* SecureWorld file keymaker.c
* Contains the key making and obtaining functions, which check validity of the key and invoke the challenges using dlls if necessary.

Nokia mayo 2021
*/

/////  FILE INCLUDES  /////

#include "keymaker.h"




/////  FUNCTION PROTOTYPES  /////

struct KeyData* getSubkey(struct ChallengeEquivalenceGroup* challenge_group);




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

	struct KeyData** keys = NULL;
	int num_groups = 0;
	int index = 0;

	// Param checking
	if (composed_key == NULL)		return ERROR_INVALID_PARAMETER;	// One or more params are null
	if (challenge_groups == NULL)	return -2;						// There are no groups, cannot make a key

	num_groups = _msize(challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*);
	if (num_groups <= 0)			return -2;						// There are no groups, cannot make a key

	// Allocate local variable keys to hold pointers to all subkeys
	keys = malloc(sizeof(struct KeyData*) * num_groups);			// Can be changed to array limmiting the maximum number of challenge groups
	if (keys == NULL)				return ERROR_NOT_ENOUGH_MEMORY;	// Cannot allocate memory

	if (composed_key->data != NULL) {
		free(composed_key->data);
	}
	composed_key->size = 0;

	// Get composed key size and allocate corresponding memory in data member
	for (size_t i = 0; i < num_groups; i++) {
		keys[i] = getSubkey(challenge_groups[i]);
		composed_key->size += keys[i]->size;
	}
	composed_key->data = calloc(composed_key->size, sizeof(byte));
	if (composed_key->data == NULL)	return ERROR_NOT_ENOUGH_MEMORY;	// Cannot allocate memory

	// Compose the key
	for (size_t i = 0; i < num_groups; i++) {
		memcpy(&(composed_key->data[index]), keys[i]->data, keys[i]->size);
		index += keys[i]->size;
	}

	// Free local variable keys
	free(keys);

	PRINT_HEX(composed_key->data, composed_key->size);

	return ERROR_SUCCESS;	// Success
}


/**
 Returns a time-valid subkey for the given challenge group. In case key expired, forces computation at the moment.
 */
struct KeyData* getSubkey(struct ChallengeEquivalenceGroup* challenge_group) {
	time_t current_time = 0;
	int group_length = 0;
	typedef int(__stdcall* exec_ch_func_type)();

	int result = 0;
	exec_ch_func_type exec_ch_func;


	// In fact, it is irrelevant if it cannot be obtained. It just forces compute key in that case
	/*if (time(&current_time) == -1) {
		fprintf(stderr, "Error while getting current time in getSubkey().\n");
	}*/

	// Get current time
	time(&current_time);

	EnterCriticalSection(&(challenge_group->subkey->critical_section));
	// Check if key expired and needs to be computed now
	if (difftime(current_time, challenge_group->subkey->expires) < 0) {

		// TODO: start on the challenge that executed correctly on the init() instead of always starting on index 0
		// Iterate over challenges until one returns that it could be executed
		group_length = _msize(challenge_group->challenges) / sizeof(struct Challenge*);
		for (size_t j = 0; j < group_length; j++) {
			// Check library was loaded
			if (INVALID_HANDLE_VALUE != challenge_group->challenges[j]->lib_handle) {
				// Define function pointer corresponding with executeChallenge() input and output types
				exec_ch_func = (exec_ch_func_type)GetProcAddress(challenge_group->challenges[j]->lib_handle, "executeChallenge");

				// Add parameters if necessary
				if (exec_ch_func != NULL) {
					result = exec_ch_func();
					if (result != 0) {
						PRINT("WARNING: error trying to execute the challenge '%ws'\n", challenge_group->challenges[j]->file_name);
					} else {
						break;		// Stop executing more challenges in the group when one is already working
					}
				} else {
					PRINT("WARNING: error accessing the address to the executeChallenge() function of the challenge '%ws' (error: %d)\n", challenge_group->challenges[j]->file_name, GetLastError());
				}

				// At this point, current challenge did not work. Check if it is the last challenge in the group and reset j to restart the loop
				if (j == group_length - 1) {
					j = -1;		// J will become 0 With the update in the loop (j++)
					Sleep(1);	// Sleep 1ms so process does not starve others
				}
			}
		}
	}
	LeaveCriticalSection(&(challenge_group->subkey->critical_section));

	return challenge_group->subkey;
}


/**
  Updates the block_access variable depending on the given challenge groups.
 */
int makeParentalKey(struct ChallengeEquivalenceGroup** challenge_groups, BOOL *block_access) {
	int num_groups = 0;
	int index = 0;
	*block_access = FALSE;

	if (block_access == NULL)		return ERROR_INVALID_PARAMETER;	// block_access is NULL

	if (challenge_groups == NULL)	return 0;	// No challenge groups (same as if all challenges are passed)

	num_groups = _msize(challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*);
	if (num_groups <= 0)			return 0;	// No challenge groups (same as if all challenges are passed)

	// Make the key
	for (size_t i = 0; i < num_groups; i++) {
		*block_access |= getSubkey(challenge_groups[i])->data[0];
	}

	return 0;	// Success
}