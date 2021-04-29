/////  FILE INCLUDES  /////

#include "sharing_app.h"




/////  FUNCTION IMPLEMENTATIONS  /////

void sharingMainMenu() {
	char line[500] = { 0 };
	int choice = 0;
	BOOL quit_menu = FALSE;

	printf("\n\n\n");
	printf("  _______________________  \n");
	printf(" |                       | \n");
	printf(" |     SHARING  MENU     | \n");
	printf(" |_______________________| \n");
	printf("\n");
	printf("This tool is intended to create shareable files for public organizations or third parties.\n");
	printf("Selecting the decipher menu (1) allows you to decipher files so the next cipher is neutralized. These files can be shared with anyone without any other requisite.\n");
	printf("Selecting the create .uva menu (2) allows you to create '.uva' files from '.pdf' files. These files can only be viewed with the use of the third party application.\n");
	printf("Note: using this tool leaves traces in a blockchain server to avoid inappropriate behaviour. Use only when strictly needed.\n");
	do {
		printf("\n");
		printf("Select an option:\n");
		printf("  1) Decipher mode (share with anyone)\n");
		printf("  2) Create .uva file (share with third party)\n");
		printf("  0) Exit (also closes mirrored disks)\n");
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
	} while (!quit_menu);
}


void decipherFileMenu() {
	char file_path[MAX_PATH] = { 0 };
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
