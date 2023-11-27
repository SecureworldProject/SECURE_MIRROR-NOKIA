/////  FILE INCLUDES  /////

#include "main.h"
#include "context.h"
#include "sharing_app.h"

#ifdef RUN_VOLUME_MOUNTER
	#include "volume_mounter.h"
#endif //RUN_VOLUME_MOUNTER

#include "logic.h"

#include "dokan/wrapper_dokan.h"
#ifdef ENABLE_WINFSP
	#include "winfsp/wrapper_winfsp.h"
#endif ENABLE_WINFSP




/////  GLOBAL VARS  /////

struct LetterDeviceMap* letter_device_table;
BOOL testing_mode_on = FALSE;
struct ExecuteChallengeData main_thread_ch_exec_data = { 0 };





/////  FUNCTION DEFINITIONS  /////

int main(int argc, char* argv[]) {
	struct ThreadData th_data[NUM_LETTERS] = { 0 };
	HANDLE threads[NUM_LETTERS] = { 0 };
	for (size_t i = 0; i < NUM_LETTERS; i++) threads[i] = INVALID_HANDLE_VALUE;
	HANDLE volume_mounter_thread = INVALID_HANDLE_VALUE;
	size_t number_of_folders = 0;

	system("cls");
	printf("\n");
	printf("\n");
	printf("          _____                       __          __        _     _ \n");
	printf("         / ____|                      \\ \\        / /       | |   | |\n");
	printf("        | (___   ___  ___ _   _ _ __ __\\ \\  /\\  / /__  _ __| | __| |\n");
	printf("         \\___ \\ / _ \\/ __| | | | '__/ _ \\ \\/  \\/ / _ \\| '__| |/ _` |\n");
	printf("         ____) |  __/ (__| |_| | | |  __/\\  /\\  / (_) | |  | | (_| |\n");
	printf("        |_____/ \\___|\\___|\\__,_|_|  \\___| \\/  \\/ \\___/|_|  |_|\\__,_|\n");
	printf("\n");
	/*
	It looks like this (but needs to duplicate backslashes to scape them)
	  _____                       __          __        _     _ 
	 / ____|                      \ \        / /       | |   | |
	| (___   ___  ___ _   _ _ __ __\ \  /\  / /__  _ __| | __| |
	 \___ \ / _ \/ __| | | | '__/ _ \ \/  \/ / _ \| '__| |/ _` |
	 ____) |  __/ (__| |_| | | |  __/\  /\  / (_) | |  | | (_| |
	|_____/ \___|\___|\__,_|_|  \___| \/  \/ \___/|_|  |_|\__,_|

	*/

	// Test the FMI table
	#ifdef RUN_FMI_TABLE_TEST
		testFMItable();
	#endif //RUN_FMI_TABLE_TEST

	// Fill the table of equivalences between harddiskvolumes and letters
	initLetterDeviceMapping();

	// Load context from config.json
	loadContext();

	// Write a file with the parental folders for the minifilter
	writeParentalFoldersFile();

	// Print the context
	#ifdef RUN_PRINT_CONTEXT
		printContext();
	#endif //RUN_PRINT_CONTEXT

	// Launch purge thread
	HANDLE purge_thread;
	purge_thread = CreateThread(NULL, 0, threadPurge, NULL, 0, NULL);


	// For each folder create and launch a thread and make it call threadDokan() or threadWinFSP()
	number_of_folders = _msize(ctx.folders) / sizeof(struct Folder*);
	for (int i = 0, th_idx = 0; i < number_of_folders; i++) {
		th_idx = DEVICE_LETTER_TO_INDEX(ctx.folders[i]->mount_point);
		PRINT("indice es %d\n", th_idx);
		th_data[th_idx].index = th_idx;
		th_data[th_idx].path = ctx.folders[i]->path;
		th_data[th_idx].letter = ctx.folders[i]->mount_point;
		th_data[th_idx].name = ctx.folders[i]->name;
		th_data[th_idx].protection = ctx.folders[i]->protection;

		// Not necessary because manual drives are subsets of "C:\"
		//GetVolumePathNameW(L"C:\\", th_data[th_idx].volume_name, MAX_PATH);
		//PRINT("thread for volume: %ws \n", th_data[th_idx].volume_name);

		switch (ctx.folders[i]->driver) {
			case DOKAN:
				threads[th_idx] = CreateThread(NULL, 0, threadDokan, &th_data[th_idx], 0, NULL);
				break;
			case WINFSP:
				threads[th_idx] = CreateThread(NULL, 0, threadWinFSP, &th_data[th_idx], 0, NULL);
				break;
			default:
				break;
		}
		Sleep(1000);
	}

	// Launch volume mounter thread
	#ifdef RUN_VOLUME_MOUNTER
		struct VolumeMounterThreadData vol_mount_th_data = { 0 };
		vol_mount_th_data.index = number_of_folders;		// 1st idx for pendrives is number_of_folders+1-1
		vol_mount_th_data.threads_p = &threads;
		vol_mount_th_data.th_data_p = &th_data;

		volume_mounter_thread = CreateThread(NULL, 0, volumeMounterThread, &vol_mount_th_data, 0, NULL);
	#endif //RUN_VOLUME_MOUNTER

	// Initialize the critical section associated to the challenge executions
	InitializeCriticalSection(&main_thread_ch_exec_data.critical_section);
	main_thread_ch_exec_data.ch_group = NULL;
	main_thread_ch_exec_data.ch = NULL;
	main_thread_ch_exec_data.request_running = FALSE;
	main_thread_ch_exec_data.result = 0;

	// Initialize the parameters for the challenges
	initChallenges();

	// Initialize the parameters for the ciphers
	initCiphers();

	// Forever loop checking for new pendrives
	//Sleep(2000);

	// Sharing menu
	CreateThread(NULL, 0, sharingMainMenu, NULL, 0, NULL);

	// Loop to execute challenges when the requests are made
	challengeExecutorLoop();
}


int threadDokan(struct ThreadData *th_data) {
	dokanMapAndLaunch(th_data->path, th_data->letter, th_data->name, th_data->protection);

	return 0;
}

int threadWinFSP(struct ThreadData *th_data) {
	#ifdef ENABLE_WINFSP
		winfspMapAndLaunch(th_data->path, th_data->letter, th_data->name, th_data->protection);
	#else
		PRINT("WinFSPMapAndLaunch parameters:   index=%2d     letter=%wc     path='%ws' \t\t\t (not implemented yet)\n", th_data->index, th_data->letter, th_data->path);
	#endif // ENABLE_WINFSP

	return 0;
}

void challengeExecutorLoop() {
	typedef int(__stdcall* exec_ch_func_type)();

	exec_ch_func_type exec_ch_func;

	while (TRUE) {
		if (main_thread_ch_exec_data.request_running) {
			exec_ch_func = (exec_ch_func_type)GetProcAddress(main_thread_ch_exec_data.ch->lib_handle, "executeChallenge");
			printf("Main Loop --> exec_ch_func holds now the addr of the func executeChallenge\n");
			if (exec_ch_func != NULL) {
				printf("Main Loop --> exec_ch_func is NOT null\n");
				main_thread_ch_exec_data.result = exec_ch_func();
				printf("Main Loop --> result of exec_ch_func is %d\n", main_thread_ch_exec_data.result);
				if (main_thread_ch_exec_data.result != 0) {
					PRINT("Main Loop --> WARNING: error trying to execute the challenge '%ws'\n", main_thread_ch_exec_data.ch->file_name);
				} else {
					// Stop executing more challenges in the group when one is already working
				}
			} else {
				PRINT("Main Loop --> WARNING: error accessing the address to the executeChallenge() function of the challenge '%ws' (error: %d)\n", main_thread_ch_exec_data.ch->file_name, GetLastError());
			}
			main_thread_ch_exec_data.request_running = FALSE;
		}
		Sleep(10);
	}
}

int execChallengeFromMainThread(struct ChallengeEquivalenceGroup* ch_group, struct Challenge* ch) {

	printf("execChallengeFromMainThread --> Entering critical section\n");
	EnterCriticalSection(&main_thread_ch_exec_data.critical_section);
	printf("execChallengeFromMainThread --> Entered critical section\n");
	main_thread_ch_exec_data.ch_group = ch_group;
	main_thread_ch_exec_data.ch = ch;
	main_thread_ch_exec_data.request_running = TRUE;
	printf("execChallengeFromMainThread --> Challenge execution requested\n");
	while (main_thread_ch_exec_data.request_running) {
		printf("execChallengeFromMainThread --> Still waiting...\n");
		Sleep(100);
	}
	printf("execChallengeFromMainThread --> Leaving critical section\n");
	LeaveCriticalSection(&main_thread_ch_exec_data.critical_section);

	printf("execChallengeFromMainThread --> Challenge execution result: %d\n", main_thread_ch_exec_data.result);

	return main_thread_ch_exec_data.result;
}


int configureExecChFromMain(struct Challenge ch) {

	typedef int(__stdcall* ExecChFromMain_func_type)(struct ChallengeEquivalenceGroup*, struct Challenge*);
	typedef void(__stdcall* setExecChFromMain_func_type)(ExecChFromMain_func_type);
	setExecChFromMain_func_type setExecChFromMain = NULL;

	setExecChFromMain = (ExecChFromMain_func_type)GetProcAddress(ch.lib_handle, "setExecChFromMain");
	if (NULL == setExecChFromMain) {
		fprintf(stderr, "ERROR: could not access 'setExecChFromMain()' function inside challenge '%ws'\n", ch.file_name);
		return -1;
	}
	PRINT("Setting pointer to execChallengeFromMainThread inside challenge '%ws'\n", ch.file_name);
	setExecChFromMain(execChallengeFromMainThread);
	return 0;
}


/**
* Fills the letter_device_table global variable.
*/
void initLetterDeviceMapping() {
	DWORD logical_drives_mask = 0;
	int count = 0;
	WCHAR tmp_str[3] = L" :";
	int index = 0;

	logical_drives_mask = GetLogicalDrives();

	//printf("logical_drives_mask (in hex): %X\n", logical_drives_mask);
	for (size_t i = 0; i < NUM_LETTERS; i++) {
		if (logical_drives_mask & (1 << i)) {
			count++;
		}
	}

	letter_device_table = malloc(count * sizeof(struct LetterDeviceMap));
	if (letter_device_table) {
		index = 0;
		for (size_t j = 0; j < NUM_LETTERS; j++) {
			if (logical_drives_mask & (1 << j)) {
				#pragma warning(suppress: 6386)
				letter_device_table[index].letter = (WCHAR)('A' + j);
				#pragma warning(suppress: 6385)
				tmp_str[0] = letter_device_table[index].letter;
				if (QueryDosDeviceW(tmp_str, letter_device_table[index].device, MAX_PATH) == 0) {
					fprintf(stderr, "ERROR: device path translation of letter %wc: is longer than %d.\n", letter_device_table[index].letter, MAX_PATH);
				}
				index++;
			}
		}
	} else {
		fprintf(stderr, "ERROR: failed to allocate necessary memory.\n");
		exit(1);
	}

	// print table
	PRINT("\nletter_device_table:\n");
	for (size_t i = 0; i < count; i++) {
		PRINT("%wc: --> %ws\n", letter_device_table[i].letter, letter_device_table[i].device);
	}
}

void initChallenges() {
	typedef int(__stdcall* init_func_type)(struct ChallengeEquivalenceGroup*, struct Challenge*);

	int result = 0;
	init_func_type init_func;


	typedef int(__stdcall* exec_ch_func_type)();
	exec_ch_func_type exec_ch_func;



	for (size_t i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
		for (size_t j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct Challenge*); j++) {
			// Define function pointer corresponding with init() input and output types
			init_func = (init_func_type)GetProcAddress(ctx.groups[i]->challenges[j]->lib_handle, "init");

			// Add parameters if necessary
			if (init_func != NULL) {
				result = init_func(ctx.groups[i], ctx.groups[i]->challenges[j]);
				if (result != 0) {
					PRINT("WARNING: error trying to initialize the challenge '%ws'\n", ctx.groups[i]->challenges[j]->file_name);
				} else {
					break;		// Stop initializing more challenges in the group when one is already working
				}
			} else {
				PRINT("WARNING: error accessing the address to the init() function of the challenge '%ws' (error: %d)\n", ctx.groups[i]->challenges[j]->file_name, GetLastError());
			}
		}
	}
}


void initCiphers() {
	typedef int(__stdcall* init_func_type)(struct Cipher*);

	int result = 0;
	init_func_type init_func;

	for (size_t i = 0; i < _msize(ctx.ciphers) / sizeof(struct Cipher*); i++) {
		// define function pointer corresponding with init() input and output types
		init_func = (init_func_type)GetProcAddress(ctx.ciphers[i]->lib_handle, "init");

		// Add parameters if necessary
		if (init_func != NULL) {
			result = init_func(ctx.ciphers[i]);
			if (result != 0) {
				PRINT("WARNING: error trying to initialize the cipher '%ws'\n", ctx.ciphers[i]->file_name);
			}
		} else {
			PRINT("WARNING: error accessing the address to the init() function of the cipher '%ws' (error: %d)\n", ctx.ciphers[i]->file_name, GetLastError());
		}
	}
}

DWORD writeParentalFoldersFile() {
	PRINT("\nWriting parental folders in a file for the minifilter...\n");
	if (ctx.parentals == NULL) {
		fprintf(stderr, "WARNING: there are no parental folders\n");
		return ERROR_SUCCESS;
	}

	HANDLE handle = INVALID_HANDLE_VALUE;
	size_t result = 0;
	DWORD error_code = ERROR_SUCCESS;
	LARGE_INTEGER distanceToMove = { 0 };
	DWORD bytes_written = 0;
	DWORD bytes_to_write = 0;
	LPCVOID buffer_to_write = NULL;

	size_t ch_groups_num = 0;
	char* ch_groups_names = NULL;
	size_t ch_groups_names_size = 0;
	//size_t longest_id_len = 0;

	size_t allowed_users_num = 0;
	char* allowed_users = NULL;
	size_t allowed_users_len = 0;
	size_t longest_allowed_user_len = 0;

	WCHAR* tmp_str = NULL;
	size_t tmp_str_size = 0;

	WCHAR* folder_path = NULL;
	WCHAR* file_path = NULL;
	size_t folder_path_len = 0;
	size_t file_path_len = 0;
	size_t filename_len = 0;
	size_t add_extra_char = 0;

	struct ParentalControl* parental_control = NULL;
	struct ParentalFolder* pf = NULL;
	WCHAR* pf_path_device_form = NULL;
	char* pf_path_device_form_str = NULL;
	size_t pf_path_device_form_len = 0;

	// Get the folder path from the environment variable
	folder_path = _wgetenv(ENVVAR_MINIFILTER_CONFIG_FOLDER);
	if (NULL == folder_path) {
		folder_path = L"";
	}

	// Get full filepath length
	folder_path_len = wcslen(folder_path);
	filename_len = wcslen(MINIFILTER_CONFIG_PARENTAL_PATHS_FILENAME);
	file_path_len = folder_path_len + filename_len;
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
		goto WRITE_PARENTAL_FOLDERS_FILE_CLEANUP;
	}

	// Compose the full filepath
	wcscpy(file_path, folder_path);
	if (add_extra_char) {
		wcscat(file_path, L"\\");
	}
	wcscat(file_path, MINIFILTER_CONFIG_PARENTAL_PATHS_FILENAME);


	//PRINT("writeTestFile() function call: buffer_to_write=%p, file_path=%ws, offset=%d, length=%d\n", buffer_to_write, file_path, offset, length);
	//PRINT_HEX(buffer_to_write, length);

	// Open the file with a wide character path/filename. Creates a new file always (replaces if exists)
	handle = CreateFileW(file_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		error_code = ERROR_OPEN_FAILED;
		fprintf(stderr, "ERROR: could not create (write) file (%ws) (error_code=%lu).\n", file_path, error_code);
		goto WRITE_PARENTAL_FOLDERS_FILE_CLEANUP;
	}
	PRINT("File %ws created\n", file_path);

	// Point to desired offset (0)
	distanceToMove.QuadPart = 0;
	if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR: could not point handle to the desired offset for writting file (%ws) (error_code=%lu).\n", file_path, error_code);
		goto WRITE_PARENTAL_FOLDERS_FILE_CLEANUP;
	}

	PRINT("Handle pointer moved\n");

	// Iterate over each parental control
	for (size_t i = 0; i < _msize(ctx.parentals) / sizeof(struct ParentalFolder*); i++) {
		parental_control = ctx.parentals[i];

		// PARENTAL FOLDER PATH
		// Get parental folder path translated to /Device/Harddisk/ syntax
		pf_path_device_form = getDevicePathFromFormattedDosPath(parental_control->folder);
		if (NULL == pf_path_device_form) {
			fprintf(stderr, "WARNING: could not transform dos path into device path\n");
			continue;
		}
		pf_path_device_form_len = wcslen(pf_path_device_form);
		pf_path_device_form_str = (char*)malloc((pf_path_device_form_len + 1) * sizeof(char));
		if (NULL == pf_path_device_form_str) {
			error_code = ERROR_NOT_ENOUGH_MEMORY;
			fprintf(stderr, "ERROR: could not allocate memory for pf_path_device_form_str.\n");
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		wcstombs(pf_path_device_form_str, pf_path_device_form, pf_path_device_form_len);

		// Write parental path to file
		bytes_written = 0;
		bytes_to_write = pf_path_device_form_len * sizeof(char);
		buffer_to_write = (LPCVOID)pf_path_device_form_str;
		PRINT("\t for-loop i=%llu: \tbuffer_to_write = %s\n", i, (char*)buffer_to_write);
		if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write parental folder into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		if (bytes_written != bytes_to_write) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write parental folder into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}


		// OUTER SEPARATOR
		// Write separator between parental folder path and required parental challenges
		bytes_written = 0;
		bytes_to_write = 1 * sizeof(char);
#ifdef MINIFILTER_USES_USERNAMES
		buffer_to_write = (LPCVOID)";";
#else
		if (NULL != parental_control->challenge_groups && 0 != _msize(parental_control->challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*)) { // Write only if there are parental groups
			buffer_to_write = (LPCVOID)":";
#endif
			if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
				error_code = ERROR_WRITE_FAULT;
				fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
				goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
			}
			if (bytes_written != bytes_to_write) {
				error_code = ERROR_WRITE_FAULT;
				fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
				goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
			}
#ifndef MINIFILTER_USES_USERNAMES
		}
#endif

		// CHALLENGE GROUPS
		// Get the number of challenge groups
		if (NULL == parental_control->challenge_groups) {
			ch_groups_num = 0;
		} else {
			ch_groups_num = _msize(parental_control->challenge_groups) / sizeof(struct ChallengeEquivalenceGroup*);
		}

		// Get the combined string length of all challenge groups in the parental control
		if (0 == ch_groups_num) {
			ch_groups_names_size = 1; // 1 for '\0'
		} else {
			ch_groups_names_size = 0;
			for (size_t j = 0; j < ch_groups_num; j++) {
				tmp_str_size = strlen(parental_control->challenge_groups[j]->id) + 1; // +1 always due to adding separator between them (':') or adding null ('\0') at the end
				ch_groups_names_size += tmp_str_size;
			}
		}

		// Allocate memory for the combined string
		ch_groups_names = (char*)malloc(ch_groups_names_size * sizeof(char));
		if (NULL == ch_groups_names) {
			error_code = ERROR_NOT_ENOUGH_MEMORY;
			fprintf(stderr, "ERROR: could not allocate memory for ch_groups_names.\n");
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}

		// Get the combined string of all challenge groups in the parental control
		ch_groups_names[0] = '\0';
		for (size_t j = 0; j < ch_groups_num; j++) {
			strcat(ch_groups_names, parental_control->challenge_groups[j]->id);
			if (j + 1 < ch_groups_num) {
				// INNER SEPARATOR
				strcat(ch_groups_names, ":");
			}
		}
		ch_groups_names[ch_groups_names_size - 1] = '\0'; // Only needed if no challenges because strcat ensures ending in '\0'

		// Write the combined string
		bytes_written = 0;
		bytes_to_write = (ch_groups_names_size - 1) * sizeof(char);
		buffer_to_write = (LPCVOID)ch_groups_names;
		if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		if (bytes_written != bytes_to_write) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}


#ifdef MINIFILTER_USES_USERNAMES
		// OUTER SEPARATOR
		// Write separator between required parental challenges and allowed users
		bytes_written = 0;
		bytes_to_write = 1 * sizeof(char);
		buffer_to_write = (LPCVOID)";";
		if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		if (bytes_written != bytes_to_write) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}


		// ALLOWED USERS
		// Get the combined string of all allowed users in the parental control
		if (NULL == parental_control->users || 0 == (allowed_users_num = _msize(parental_control->users) / sizeof(WCHAR*))) {
			allowed_users_len = 1;
			allowed_users = (char*)malloc(allowed_users_len * sizeof(char));
			if (NULL == allowed_users) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				fprintf(stderr, "ERROR: could not allocate memory for allowed_users.\n");
				goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
			}
		} else {
			allowed_users_len = 0;
			longest_allowed_user_len = 0;
			for (size_t j = 0; j < allowed_users_num; j++) {
				tmp_str_size = wcslen(parental_control->users[j]) + 1; // +1 always due to adding separator between them (':') or adding null ('\0') at the end
				allowed_users_len += tmp_str_size;
				longest_allowed_user_len = MAX(longest_allowed_user_len, tmp_str_size);
			}
			allowed_users = (char*)malloc(allowed_users_len * sizeof(char));
			if (NULL == allowed_users) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				fprintf(stderr, "ERROR: could not allocate memory for allowed_users.\n");
				goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
			}
			tmp_str = (char*)malloc(longest_allowed_user_len * sizeof(char));
			if (NULL == tmp_str) {
				error_code = ERROR_NOT_ENOUGH_MEMORY;
				fprintf(stderr, "ERROR: could not allocate memory for tmp_str.\n");
				goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
			}

			for (size_t j = 0; j < allowed_users_num; j++) {
				wcstombs(tmp_str, parental_control->users[j], longest_allowed_user_len);
				strcat(allowed_users, tmp_str);
				if (j + 1 < allowed_users_num) {
					// INNER SEPARATOR
					strcat(allowed_users, ":");
				}
			}
		}
		allowed_users[allowed_users_len - 1] = '\0';

		// Write the combined string
		bytes_written = 0;
		bytes_to_write = (allowed_users_len - 1) * sizeof(char);
		buffer_to_write = (LPCVOID)allowed_users;
		if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		if (bytes_written != bytes_to_write) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
#endif

		// SEPARATOR --> "\n"
		// Write separator between parental controls
		bytes_written = 0;
		bytes_to_write = 1 * sizeof(char);
		buffer_to_write = (LPCVOID)"\n";
		if (!WriteFile(handle, buffer_to_write, bytes_to_write, &bytes_written, NULL)) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}
		if (bytes_written != bytes_to_write) {
			error_code = ERROR_WRITE_FAULT;
			fprintf(stderr, "ERROR: could not write into file (%ws).\n", file_path);
			goto LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP;
		}

		// CLEANUP
	LOOP_IN_WRITEPARENTALFOLDERSFILE_CLEANUP:
		if (ch_groups_names != NULL) {
			free(ch_groups_names);
			ch_groups_names = NULL;
		}
		if (tmp_str != NULL) {
			free(tmp_str);
			tmp_str = NULL;
		}
		if (ERROR_SUCCESS != error_code) {
			goto WRITE_PARENTAL_FOLDERS_FILE_CLEANUP;
		}
	}

	// Close file handle if necessary and return corresponding error_code
WRITE_PARENTAL_FOLDERS_FILE_CLEANUP:
	if (NULL != file_path) {
		free(file_path);
	}
	if (handle != INVALID_HANDLE_VALUE) {
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
	if (NULL != pf_path_device_form) {
		free(pf_path_device_form);
		pf_path_device_form = NULL;
	}

	PRINT("\nParental folders' file %s completed (error_code = %lu)\n", (error_code == ERROR_SUCCESS) ? "was" : "could not be", error_code);

	// Return corresponding error_code or success code
	return error_code;
}