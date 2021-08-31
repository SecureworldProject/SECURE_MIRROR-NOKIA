/////  FILE INCLUDES  /////

#include "sharing_app.h"
#include <Windows.h>
#include <fileapi.h>
#include <time.h>
#include "context.h"
#include "logic.h"


#define MAX_INPUT_LENGTH 500
#define READ_BUF_SIZE 1024 * 1024	// 1 MB
#define DECIPHERED_SUFFIX_WCS L"_deciphered"

/////  FUNCTION PROTOTYPES  /////
void decipherFileMenu();
void uvaFileMenu();
int createDecipheredFileCopy(WCHAR* file_path);
int createUvaFileCopy(WCHAR* file_path, time_t allowed_visualization_period_begin, time_t allowed_visualization_period_end, struct ThirdParty* third_party);




/////  FUNCTION IMPLEMENTATIONS  /////

void sharingMainMenu() {
	WCHAR line[MAX_INPUT_LENGTH] = { 0 };
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
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &choice)) {
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
	WCHAR input_file_path[MAX_PATH] = { 0 };
	int result = 0;

	printf("\n\tYou have entered the decipher option.\n");

	// Get the input file path
	printf("\n\tEnter the full path of the file from which you want to create a deciphered copy below.\n");
	printf("\t--> ");
	if (fgetws(input_file_path, MAX_PATH, stdin)) {	// fgets() ensures that string ends with '\0'
		input_file_path[strcspn(input_file_path, L"\n")] = L'\0';		// Remove trailing '\n'

		if (!PathFileExistsW(input_file_path)) {
			printf("\tThe specified path does not exist.\n");
			return;
		}
		if (PathIsDirectoryW(input_file_path)) {
			printf("\tThe specified path matches a directory not a file.\n");
			return;
		}
	}

	// Get the output file path
	printf("\n\tEnter the full path of the file from which you want to create a deciphered copy below.\n");
	printf("\t--> ");
	if (fgetws(input_file_path, MAX_PATH, stdin)) {	// fgets() ensures that string ends with '\0'
		input_file_path[strcspn(input_file_path, L"\n")] = L'\0';		// Remove trailing '\n'

		if (!PathFileExistsW(input_file_path)) {
			printf("\tThe specified path does not exist.\n");
			return;
		}
		if (PathIsDirectoryW(input_file_path)) {
			printf("\tThe specified path matches a directory not a file.\n");
			return;
		}
	}

	printf("\tThe deciphered file copy is being created...\n");

	result = createDecipheredFileCopy(input_file_path);

	if (result != 0) {
		printf("\tThere was an error while trying to create the deciphered copy. (errcode: %d)\n", result);
	} else {
		printf("\tThe deciphered copy was successfully created.\n");
	}

	return;
}

void uvaFileMenu() {
	WCHAR line[500] = { 0 };
	time_t current_time;
	struct tm* time_info = NULL;
	int integer_user_input = 0;
	char formatted_time[22] = "";

	WCHAR file_path[MAX_PATH] = { 0 };
	time_t allowed_visualization_period_begin = 0;
	time_t allowed_visualization_period_end = 0;
	struct ThirdParty* third_party = NULL;
	int result = 0;

	printf("\n\tYou have entered the .uva creation option.\n");

	// Get the path
	printf("\n\tEnter the full path of the file from which you want to create a .uva file below.\n");
	printf("\t--> ");
	if (fgetws(file_path, MAX_PATH, stdin)) {
		file_path[wcslen(file_path) - 1] = '\0';		// End the buffer with null character for the case in which fgets() filled it completely
		if (!PathFileExistsW(file_path)) {
			printf("\tThe specified path does not exist.\n");
			return;
		}
		if (PathIsDirectoryW(file_path)) {
			printf("\tThe specified path matches a directory not a file.\n");
			return;
		}
	}

	// Get current time
	if (time(&current_time) == -1) {
		printf("\tError while getting current time.\n");
		return;
	}

	// Get the allowed visualization period
	for (size_t i = 0; i < 2; i++) {
		time_info = localtime(&current_time);

		strftime(formatted_time, 22, "%Y-%m-%d - %H:%M:%S", time_info);
		printf("\n\tEnter the date %s which the file will be accesible. Skipped values default to current date/time (%s).\n", (i==0)?"from":"until", formatted_time);

		// Get the year
		printf("\t Year \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_year = integer_user_input - 1900;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_year + 1900);
			}
		}
		// Get the month
		printf("\t Month \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_mon = integer_user_input - 1;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_mon + 1);
			}
		}
		// Get the day
		printf("\t Day \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_mday = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_mday);
			}
		}
		// Get the hours
		printf("\t Hours \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_hour = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_hour);
			}
		}
		// Get the minutes
		printf("\t Minutes \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_min = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_min);
			}
		}
		// Get the secconds
		printf("\t Secconds \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info->tm_sec = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info->tm_sec);
			}
		}

		// Fill weekday and day of the year and correct possible off-bound values in other fields (ie. tm_mon>11, tm_mday>31, etc.)
		if (i == 0) allowed_visualization_period_begin = mktime(time_info);
		if (i == 1) allowed_visualization_period_end = mktime(time_info);
	}

	// Check that allowed_visualization_period ending is later than beginning
	if (difftime(allowed_visualization_period_end, allowed_visualization_period_begin) <= 0) {
		printf("\tError: the ending of the allowed visualization period must be a later time than the beginning.\n");
		return;
	}

	// Get the third party to share with
	printf("\n\tSelect the third party you want to share the .uva file with:\n");
	for (size_t i = 0; i < _msize(ctx.third_parties)/sizeof(struct ThirdParty*); i++) {
		printf("\t  %llu) %s\n", i, ctx.third_parties[i]->id);
	}
	if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
		if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
			if (integer_user_input < 0 || integer_user_input > _msize(ctx.third_parties) / sizeof(struct ThirdParty*)) {
				printf("\tThere is no third party asigned to that number.\n");
				return;
			}
		}
	}

	printf("\tThe .uva file is being created...\n");

	result = createUvaFileCopy(file_path, allowed_visualization_period_begin, allowed_visualization_period_end, third_party);

	if (result != 0) {
		printf("\tThere was an error while trying to create the .uva file. (errcode: %d)\n", result);
		// Possible error: specify that only .pdf files can be transformed into .uva
	} else {
		printf("\tThe .uva file was successfully created.\n");
	}

	return;
}

int createDecipheredFileCopy(WCHAR* file_path) {
	printf("\t TO DO\n");
	// This function will:
	// - Create a file in the same path adding "_deciphered" at the end (but before extension).
	// - Read the original file and call decipher() for all the content.
	// - Add blockchain traces
	// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns an error code.


	HANDLE read_file_handle = NULL;
	HANDLE write_file_handle = NULL;
	LPVOID* read_buf = NULL;
	size_t file_size = 0;
	DWORD bytes_read = 0;
	WCHAR* file_path_write = NULL;
	int file_path_write_len = 0;
	LARGE_INTEGER distance_to_move = { 0 };

	DWORD error_code = ERROR_SUCCESS;
	DWORD result = ERROR_SUCCESS;

	// Open original file
	read_file_handle = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!read_file_handle || read_file_handle == INVALID_HANDLE_VALUE) {
		error_code = GetLastError();
		PRINT1("ERROR in createDecipheredFileCopy: creating handle (error = %lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Get file size
	result = getFileSize(&file_size, read_file_handle, file_path);
	if (result != 0) {
		error_code = result;
		PRINT1("ERROR in createDecipheredFileCopy: getting file size (error = %lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	if (file_size == 0) {
		PRINT1("File size is 0\n");
		error_code = -1;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Make handle point to the beginning of the file
	if (!SetFilePointerEx(read_file_handle, distance_to_move, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		PRINT1("ERROR handle seeking in postWrite (error=%lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Allocate read buffer
	/*read_buf = calloc(file_size, sizeof(byte));
	if (read_buf == NULL) {
		PRINT1("ERROR allocate memory for reading.\n");
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Open write file
	file_path_write_len = wcslen(file_path) + wcslen(DECIPHERED_SUFFIX_WCS) +1;		// +1 for the '\0'
	file_path_write = malloc(file_path_write_len * sizeof(WCHAR));
	if (file_path_write == NULL) {
		PRINT1("ERROR allocate memory for write path (%ws)\n", file_path_write);
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	wcscpy_s(file_path_write, wcslen(file_path) + 1, file_path);	// +1 for the '\0' (maybe not needed because another string is copied afterwards)
	wcscpy_s(&(file_path_write[wcslen(file_path)]), wcslen(DECIPHERED_SUFFIX_WCS) + 1, DECIPHERED_SUFFIX_WCS);		// +1 for the '\0'
	write_file_handle = _wfopen(file_path_write, L"rb");
	if (write_file_handle == NULL) {
		PRINT1("ERROR opening write file (%ws)\n", file_path_write);
		error_code = ERROR_OPEN_FAILED;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}


	// Read original file
	while (!feof(read_file_handle)) {
		bytes_read = fread_s(read_buf, file_size, sizeof(byte), file_size, read_file_handle);
		if (ferror(read_file_handle)) {
			PRINT1("ERROR reading file.\n");
			error_code = ERROR_READ_FAULT;
			goto DECIPHERED_FILE_COPY_CLEANUP;
		}

		// TO DO ///////////////////////////////////

	}


	// Cycle until end of file reached:
	//while (!feof(stream)) {
	//	// Attempt to read in 100 bytes:
	//	count = fread(buffer, sizeof(char), 100, stream);
	//	if (ferror(stream)) {
	//		perror("Read error");
	//		break;
	//	}

	//	// Total up actual bytes read
	//	total += count;
	//}
	// TO DO ///////////////////////////////////
	*/



	// Make sure of freeing everything before leaving the function
	DECIPHERED_FILE_COPY_CLEANUP:
	if (read_file_handle != NULL) {
		fclose(read_file_handle);
	}
	if (write_file_handle != NULL) {
		fclose(write_file_handle);
	}
	if (read_buf != NULL) {
		free(read_buf);
	}

	return error_code;
}

int createUvaFileCopy(WCHAR* file_path, time_t allowed_visualization_period_begin, time_t allowed_visualization_period_end, struct ThirdParty* third_party) {
	printf("\t TO DO\n");
	// This function will:
	// - Check the file is a ".pdf" file.
	// - Create a file in the same path changing the extension to ".uva".
	// - Fill the .uva header with necessary metadata.
	// - Read the original file and call decipher() followed by cipherTP() for all the content while writting to the ".uva" file.
	// - Add blockchain traces
	// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns -1.
	return 0;
}