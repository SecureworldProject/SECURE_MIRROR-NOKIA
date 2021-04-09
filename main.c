
/////  FILE INCLUDES  /////

#include "main.h"




/////  FUNCTION DEFINITIONS  /////

int main(int argc, char* argv[]) {

	int i = 0;
	HANDLE threads[25] = { 0 };
	struct ThreadData th_data[25] = { 0 };
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
		th_data[i].thread_id = i;
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
	}
	
	// Forever loop checking for new pendrives
	printf("\n");
	printf("  _______________________  \n");
	printf(" |                       | \n");
	printf(" |     DECIPHER MENU     | \n");
	printf(" |_______________________| \n");
	printf("\n");
	printf("This tool is intended to share clear files with third parties.\n");
	printf("You can decipher files so the next cipher is neutralized and file can be shared by email/pendrive.\n");
	printf("Note: using this tool leaves traces in a blockchain server to avoid inappropriate behaviour. Use only when strictly needed.\n");
	do {
		printf(" 1) Decipher file\n");
		printf(" 2) Cipher file\n");
		printf(" 0) Exit (also closes mirrored disks)\n");
		if (fgets(line, sizeof(line), stdin)) {
			if (1 == sscanf(line, "%d", &choice)) {
				switch (choice) {
					case 1:
						printf("Selected DECIPHER mode.\n");
						decipherMenu();
						break;
					case 2:
						printf("Selected CIPHER mode.\n");
						// TO DO cipher menu
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
	//dokanMapAndLaunch(th_data.path, th_data.letter);		////////////////// TO DO UNCOMMENT
	while (TRUE) {
		printf("Hello, Dokan thread with id=%d reporting alive.\n", th_data->thread_id);
		Sleep(8000);
	}

	return 0;
}

int threadWinFSP(struct ThreadData *th_data) {
	//winFSPMapAndLaunch(th_data.path, th_data.letter);		////////////////// TO DO UNCOMMENT
	while (TRUE) {
		printf("Hello, WinFSP thread with id=%d reporting alive.\n", th_data->thread_id);
		Sleep(8000);
	}

	return 0;
}

void decipherMenu() {
	char file_path[MAX_PATH] = { 0 };
	char line[500] = { 0 };
	int result = 0;

	printf("   You have entered Decipher mode.\n");
	printf("   Enter the full path of the file you want to decipher below.\n");
	printf("   --> ");
	if (fgets(file_path, sizeof(file_path), stdin)) {
		file_path[strlen(file_path) - 1] = '\0';
		if (PathFileExistsA(file_path) && !PathIsDirectoryA(file_path)) {
			printf("   A deciphered copy of the file is being created...\n");

			// Call createDecipheredFileCopy(file_path) which will:
			// - Create a file in the same path adding "_deciphered" at the end (but before extension).
			// - Read the file_path file and call decipher() for all the content.
			// - Add blockchain traces
			// - If everything goes well, returns 0. In case something goes wrong, removes the newly created file and returns -1.
			#define createDecipheredFileCopy(A) 0		// TO DO create real function createDecipheredFileCopy(char* file_path)
			result = createDecipheredFileCopy(file_path);
			#undef createDecipheredFileCopy 			// TO DO create real function createDecipheredFileCopy(char* file_path)
			if (result == -1) {
				printf("   There was an error while trying to create the deciphered copy.\n");
			} else {
				printf("   The deciphered copy was successfully created.\n");
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
