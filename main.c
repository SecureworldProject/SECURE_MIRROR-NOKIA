
/////  FILE INCLUDES  /////

#include "main.h"
#include "context.h"
#include "context2.h"




/////  FUNCTION DEFINITIONS  /////

int main(int argc, char* argv[]) {

	int i = 0;
	struct ThreadData th_data[NUM_LETTERS] = { 0 };
	HANDLE threads[NUM_LETTERS] = { 0 };
	char line[500] = { 0 };
	int choice = 0;
	BOOL quit_menu = FALSE;

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

	// Fill the table of equivalences between harddiskvolumes and letters
	initLetterDeviceMapping();

	// Load context from config.json
	loadContext();

	// Print the context
	printContext();

	// For each folder create and launch a thread and make it call threadDokan() or threadWinFSP()
	for (i = 0; i < _msize(ctx.folders) / sizeof(struct Folder*); i++) {
	//for (i = 0; i < 1; i++) {
		th_data[i].index = i;
		th_data[i].letter = ctx.folders[i]->mount_point;
		th_data[i].path = ctx.folders[i]->path;

		switch (ctx.folders[i]->driver) {
			case DOKAN:
				threads[i] = CreateThread(NULL, 0, threadDokan, &th_data[i], 0, NULL);
				break;
			case WINFSP:
				threads[i] = CreateThread(NULL, 0, threadWinFSP, &th_data[i], 0, NULL);
				break;
			default:
				break;
		}
		Sleep(1000);
	}

	// Initialize the parameters for the challenges
	initChallenges();
	
	// Forever loop checking for new pendrives
	Sleep(5000);

	printf("\n");
	printf("  _______________________  \n");
	printf(" |                       | \n");
	printf(" |     DECIPHER MENU     | \n");
	printf(" |_______________________| \n");
	printf("\n");
	printf("This tool is intended to share clear files with public organizations or third parties.\n");
	printf("Selecting the decipher menu (1) allows you to decipher files so the next cipher is neutralized. These files can be shared with anyone without any other requisite.\n");
	printf("Selecting the create .uva menu (2) allows you to create '.uva' files from '.pdf' files. These files can only be viewed with the use of the third party application.\n");
	printf("Note: using this tool leaves traces in a blockchain server to avoid inappropriate behaviour. Use only when strictly needed.\n");
	do {
		printf(" 1) Decipher mode (share with anyone)\n");
		printf(" 2) Create .uva file (share with third party)\n");
		printf(" 0) Exit (also closes mirrored disks)\n");
		if (fgets(line, sizeof(line), stdin)) {
			if (1 == sscanf(line, "%d", &choice)) {
				switch (choice) {
					case 1:
						decipherFileMenu();
						break;
					case 2:
						uvaFileMenu();
						break;
					case 0:
						printf("Exitting...\n");
						quit_menu = TRUE;
						break;
					default:
						printf("Invalid option, try again.\n");
						break;
				}
			}
		}
		printf("\n");
	} while (!quit_menu);
}


int threadDokan(struct ThreadData *th_data) {
	dokanMapAndLaunch(th_data->path, th_data->letter, th_data->index);		////////////////// TO DO UNCOMMENT
	/*while (TRUE) {
		printf("Hello, Dokan thread with id=%d reporting alive.\n", th_data->index);
		Sleep(8000);
	}*/

	return 0;
}

int threadWinFSP(struct ThreadData *th_data) {
	//winFSPMapAndLaunch(th_data->path, th_data->letter, th_data->index);		////////////////// TO DO UNCOMMENT
	while (TRUE) {
		printf("Hello, WinFSP thread with id=%d reporting alive.\n", th_data->index);
		Sleep(8000);
	}

	return 0;
}

void decipherFileMenu() {
	char file_path[MAX_PATH] = { 0 };
	char line[500] = { 0 };
	int result = 0;

	printf("   You have entered Decipher menu.\n");
	printf("   Enter the full path of the file from which you want to create a deciphered copy below.\n");
	printf("   --> ");
	if (fgets(file_path, sizeof(file_path), stdin)) {
		file_path[strlen(file_path) - 1] = '\0';
		if (PathFileExistsA(file_path) && !PathIsDirectoryA(file_path)) {
			printf("   The deciphered file being created...\n");

			// Call createDecipheredFileCopy(file_path) which will:
			// - Create a file in the same path adding "_deciphered" at the end (but before extension).
			// - Read the original file and call decipher() for all the content.
			// - Add blockchain traces
			// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns -1.
			#define createDecipheredFileCopy(A) 0		// TO DO create real function createDecipheredFileCopy(char* file_path)
			result = createDecipheredFileCopy(file_path);
			#undef createDecipheredFileCopy 			// TO DO create real function createDecipheredFileCopy(char* file_path)
			if (result != 0) {
				printf("   There was an error while trying to create the deciphered copy. (errcode: %d)\n", result);
			} else {
				printf("   The deciphered copy was successfully created.\n");
			}
		} else {
			printf("   The specified path does not exist.\n");
		}
	}
}

void uvaFileMenu() {
	char file_path[MAX_PATH] = { 0 };
	char line[500] = { 0 };
	int result = 0;
	time_t allowed_time_begin;
	time_t allowed_time_end;

	printf("   You have entered the .uva creation menu.\n");
	printf("   Enter the full path of the file from which you want to create a .uva file below.\n");
	printf("   --> ");
	// TO DO: ask also for allowed_time_begin, allowed_time_end and the enterprise to share with
	if (fgets(file_path, sizeof(file_path), stdin)) {
		file_path[strlen(file_path) - 1] = '\0';
		if (PathFileExistsA(file_path) && !PathIsDirectoryA(file_path)) {
			printf("   The .uva file is being created...\n");

			// Call createUvaFileCopy(file_path) which will:
			// - Check the file is a ".pdf" file.
			// - Create a file in the same path changing the extension to ".uva".
			// - Fill the .uva header with necessary metadata.
			// - Read the original file and call decipher() followed by cipherTP() for all the content while writting to the ".uva" file.
			// - Add blockchain traces
			// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns -1.
			#define createUvaFileCopy(A, B, C) 0		// TO DO create real function createUvaFileCopy(char* file_path)
			result = createUvaFileCopy(file_path, allowed_time_begin, allowed_time_end);
			#undef createUvaFileCopy					// TO DO create real function createUvaFileCopy(char* file_path)
			if (result != 0) {
				printf("   There was an error while trying to create the .uva file. (errcode: %d)\n", result);
				// Possible error: specify that only .pdf files can be transformed into .uva
			} else {
				printf("   The .uva file was successfully created.\n");
			}
		} else {
			printf("   The specified path does not exist.\n");
		}
	}
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
				letter_device_table[index].letter = (WCHAR)('A' + j);
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
	typedef int(__stdcall* init_func_type)();

	int result = 0;
	BOOL group_initialized;
	init_func_type dll_init_func;

	for (size_t i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
		for (size_t j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct Challenge*); j++) {
			// define function pointer corresponding with init() input and output types
			dll_init_func = (init_func_type)GetProcAddress(ctx.groups[i]->challenges[j]->lib_handle, "init");

			// Add parameters if necessary
			result = dll_init_func();
			if (result!=0) {
				PRINT("WARNING: error trying to initialize the\n");
			} else {
				break;		// Stop initializing more challenges in the group when one is already working
			}
		}
	}
}
