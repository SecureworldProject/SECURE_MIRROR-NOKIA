/////  FILE INCLUDES  /////

#include <Windows.h>
#include <fileapi.h>
#include <time.h>
#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")

#include "main.h"
#include "sharing_app.h"
#include "context.h"
#include "logic.h"
#include "keymaker.h"
#include "system_test.h"
//#include "qrcodegen.h"
#include "securemirror_rsa.h"




/////  DEFINITIONS  /////

#define MAX_INPUT_LENGTH 500
#define READ_BUF_SIZE (1024 * 1024)		// 1 MB
#define DECIPHERED_SUFFIX_WCS L"_deciphered"
#define DECIPHERED_SUFFIX_WCS_LEN wcslen(DECIPHERED_SUFFIX_WCS)
#define MAX_LINK_LENGTH 500
#define PRIV_KEY_PEM_SUFFIX L"_priv.pem"
#define PUB_KEY_PEM_SUFFIX L"_pub.pem"



/////  FUNCTION PROTOTYPES  /////
void printMenuHelp();
void decipherFileMenu();
void uvaFileMenu();
void newRSAKeypairMenu();
void printQr(const uint8_t qrcode[]);
//void showQRDeepLink();
int createDecipheredFileCopy(WCHAR* file_path);
int createUvaFileCopy(WCHAR* file_path, time_t allowed_visualization_period_begin, time_t allowed_visualization_period_end, struct ThirdParty* third_party);




/////  FUNCTION IMPLEMENTATIONS  /////

/**
* Shows a text-based interactive menu which allows sereral options like exitting, deciphering files, showing a QR code to link the challenge capturer app, etc.
*
* @return
**/
void sharingMainMenu() {
	WCHAR line[MAX_INPUT_LENGTH] = { 0 };
	int choice = 0;
	BOOL quit_menu = FALSE;

	printMenuHelp();
	do {
		printf("\n");
		printf("Select an option:\n");
		printf("  0) Exit (also closes mirrored disks)\n");
		printf("  1) Decipher mode (share with anyone)\n");
		printf("  2) Create .uva file (share with third party)\n");
		//printf("  3) Show QR code (link android device)\n");
		printf("  3) Genrate third party rsa key pair and save it somewhere\n");
		#ifdef SECUREMIRROR_DEBUG_MODE
		printf("  4) (Debug only) Print the File Mark Info Table\n");
		printf("  5) (Debug only) Test System\n");
		printf("  6) (Debug only) Unit Test\n");
		printf("  7) (Debug only) Print Unit Test Information\n");
		#endif
		printf("  9) Show help\n");
		printf("\n");

		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &choice)) {
				switch (choice) {
					case 0:
						printf("Exitting...\n");
						quit_menu = TRUE;
						break;
					case 1:
						decipherFileMenu();
						break;
					case 2:
						uvaFileMenu();
						break;
					//case 3:
					//	showQRDeepLink();
					//	break;
					case 3:
						newRSAKeypairMenu();
						break;
					case 4:
						#ifdef SECUREMIRROR_DEBUG_MODE
						printFMITable();
						break;
						#endif		// else goes through next options until default
					case 5:
						#ifdef SECUREMIRROR_DEBUG_MODE
						testEverything();
						break;
						#endif		// else goes through next options until default
					case 6:
						#ifdef SECUREMIRROR_DEBUG_MODE
						printf("Not implemented yet.\n");
						unitTestMenu();
						break;
						#endif		// else goes through next options until default
					case 7:
						#ifdef SECUREMIRROR_DEBUG_MODE
						printf("Not implemented yet.\n");
						printUnitTestDataMenu();
						break;
						#endif		// else goes through next options until default
					/*case 8:
						testing_mode_on = !testing_mode_on;
						printf("Cambiado testing mode on (%p) a: %s\n", &testing_mode_on, (testing_mode_on) ? "true" : "false");
						break;*/
					case 9:
						printMenuHelp();
						break;
					default:
						printf("Invalid option, try again.\n");
						break;
				}
			}
		}
	} while (!quit_menu);
	exit(0);
}

/**
* Shows the menu help for each of the options allowed like exitting, deciphering files, showing a QR code to link the challenge capturer app, etc.
*
* @return
**/
void printMenuHelp() {
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
}


/**
* Starts a text-based dialog with the user to gather all the necessary information to make a deciphered copy of a file.
*
* @return
**/
void decipherFileMenu() {
	//WCHAR input_file_path[MAX_PATH] = { 0 };
	WCHAR* input_file_path = NULL;
	int result = 0;

	input_file_path = malloc(MAX_PATH * sizeof(WCHAR));
	if (input_file_path == NULL) {
		printf("\tError: cannot allocate memory.\n");
		return;
	}

	printf("\n\tYou have entered the decipher option.\n");

	// Get the input file path
	printf("\n\tEnter the full path of the file from which you want to create a deciphered copy below.\n");
	printf("\t--> ");
	if (fgetws(input_file_path, MAX_PATH, stdin)) {		// fgets() ensures that string ends with '\0'
		input_file_path[wcscspn(input_file_path, L"\n")] = L'\0';		// Remove trailing '\n'

		if (!PathFileExistsW(input_file_path)) {
			printf("\tThe specified path does not exist.\n");
			return;
		}
		if (PathIsDirectoryW(input_file_path)) {
			printf("\tThe specified path matches a directory not a file.\n");
			return;
		}
	}

	// TO DO: allow user to write output file path??? For the moment use only the first path adding DECIPHERED_SUFFIX_WCS at the end

	// Create deciphered copy
	printf("\tThe deciphered file copy is being created...\n");
	result = createDecipheredFileCopy(input_file_path);

	if (result != 0) {
		printf("\tThere was an error while trying to create the deciphered copy. (errcode: %d)\n", result);
	} else {
		printf("\tThe deciphered copy was successfully created.\n");
		printf("\tGENERATING TRACES FOR POTENTIALLY DANGEROUS ACTIVITIES.\n");
	}

	return;
}

/**
* Starts a text-based dialog with the user to gather all the necessary information to make a .uva copy of a .pdf file.
*
* @return
**/
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

/**
* Starts a text-based dialog with the user to gather all the necessary information to generate and save a new RSA keypair.
*
* @return
**/
void newRSAKeypairMenu() {
	WCHAR* keypair_directory = NULL;
	WCHAR* keypair_filename = NULL;
	size_t keypair_directory_length = 0;
	size_t keypair_filename_length = 0;

	size_t priv_key_pem_suffix_length = wcslen(PRIV_KEY_PEM_SUFFIX);
	size_t pub_key_pem_suffix_length = wcslen(PUB_KEY_PEM_SUFFIX);

	//RSA* rsa_keypair = NULL;
	WCHAR* priv_key_filepath = NULL;
	WCHAR* pub_key_filepath = NULL;

	keypair_directory = malloc(MAX_PATH * sizeof(WCHAR));
	if (keypair_directory == NULL) {
		printf("\tError: cannot allocate memory.\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}

	printf("\n\tYou have entered the RSA Keypair generation option.\n");
	printf("\tA private and a public file will be created in the given folder appending '%ws' and '%ws' respectively to the filename given.\n", PRIV_KEY_PEM_SUFFIX, PUB_KEY_PEM_SUFFIX);

	// Get the directory
	printf("\n\tEnter below the folder in which you want the keypair to be created.\n");
	printf("\t--> ");
	if (fgetws(keypair_directory, MAX_PATH, stdin)) {		// fgets() ensures that string ends with '\0'
		// Ensure of trailing '/' for the directory and remove trailing /n due to console input
		keypair_directory_length = wcscspn(keypair_directory, L"\n");	// Returns the position of first '\n' or the length of the string if it is not contained
		if (MAX_PATH > keypair_directory_length) {
			if (keypair_directory[keypair_directory_length - 1] != L'/' || keypair_directory[keypair_directory_length - 1] != L'\\') {
				keypair_directory[keypair_directory_length] = L'/';
				keypair_directory_length++;
			}
		}
		keypair_directory[keypair_directory_length] = L'\0';

		// Check directory length and existence
		if (MAX_PATH <= keypair_directory_length + 1 + priv_key_pem_suffix_length || MAX_PATH <= keypair_directory_length + 1 + pub_key_pem_suffix_length) {
			printf("\tThe specified directory is too long.\n");
			goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
		}
		if (!PathFileExistsW(keypair_directory)) {
			printf("\tThe specified path does not exist.\n");
			goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
		}
		if (!PathIsDirectoryW(keypair_directory)) {
			printf("\tThe specified path matches a file not a directory.\n");
			goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
		}

		// Get the filename
		keypair_filename = malloc((MAX_PATH - keypair_directory_length) * sizeof(WCHAR));
		if (keypair_filename == NULL) {
			printf("\tError: cannot allocate memory.\n");
			goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
		}

		printf("\n\tEnter below the filename (no extension) for the keypair files. Remember two files with distinguishable suffixes will be created.\n");
		printf("\t--> ");
		if (fgetws(keypair_filename, (MAX_PATH - keypair_directory_length), stdin)) {		// fgets() ensures that string ends with '\0'
			keypair_filename_length = wcscspn(keypair_filename, L"\n");	// Returns the position of first '\n' or the length of the string if it is not contained
			keypair_filename[wcscspn(keypair_filename, L"\n")] = L'\0';		// Remove trailing '\n' if exists

			if (MAX_PATH <= keypair_directory_length + keypair_filename_length + priv_key_pem_suffix_length) {
				printf("\tThe specified path is too long to append %ws.\n", PRIV_KEY_PEM_SUFFIX);
				goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
			}
			if (MAX_PATH <= keypair_directory_length + keypair_filename_length + pub_key_pem_suffix_length) {
				printf("\tThe specified path is too long to append %ws.\n", PUB_KEY_PEM_SUFFIX);
				goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
			}
		}
	}

	priv_key_filepath = malloc((keypair_directory_length + keypair_filename_length + priv_key_pem_suffix_length + 1) * sizeof(WCHAR));
	if (priv_key_filepath == NULL) {
		printf("\tError: cannot allocate memory.\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}
	pub_key_filepath = malloc((keypair_directory_length + keypair_filename_length + pub_key_pem_suffix_length + 1) * sizeof(WCHAR));
	if (pub_key_filepath == NULL) {
		printf("\tError: cannot allocate memory.\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}
	//rsa_keypair = (RSA*)malloc(1 * sizeof(RSA*));
	//if (rsa_keypair == NULL) {
	//	printf("\tError: cannot allocate memory.\n");
	//	goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	//}

	wcscpy_s(priv_key_filepath, keypair_directory_length, keypair_directory);
	wcscat_s(priv_key_filepath, keypair_filename_length, keypair_filename);
	wcscat_s(priv_key_filepath, priv_key_pem_suffix_length, PRIV_KEY_PEM_SUFFIX);

	wcscpy_s(pub_key_filepath, keypair_directory_length, keypair_directory);
	wcscat_s(pub_key_filepath, keypair_filename_length, keypair_filename);
	wcscat_s(pub_key_filepath, pub_key_pem_suffix_length, PUB_KEY_PEM_SUFFIX);


	if (ERROR_SUCCESS != generate_and_write_key(RSA_F4, KEY_BYTES_SIZE * 8, priv_key_filepath, pub_key_filepath, NULL)) { //&rsa_keypair)) {
		printf("ERROR generating the keys\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}

	printf("\tThe RSA keypair files have been sucessfully generated:\n\t%ws\n\t%ws\n", priv_key_filepath, pub_key_filepath);

NEW_RSA_KEY_PAIR_MENU_CLEANUP:
	if (NULL != keypair_directory) {
		free(keypair_directory);
	}
	if (NULL != keypair_filename) {
		free(keypair_filename);
	}
	if (NULL != priv_key_filepath) {
		free(priv_key_filepath);
	}
	if (NULL != pub_key_filepath) {
		free(pub_key_filepath);
	}
	//if (NULL != rsa_keypair) {
	//	free(rsa_keypair);
	//}

	return;
}


/**
* Prints the given QR Code to the console.
*
* @param const uint8_t qrcode[]
*		The QR code (in uint8_t array format) to be printed.
* @return
**/
void printQr(const uint8_t qrcode[]) {
	int size = qrcodegen_getSize(qrcode);
	int border = 4;
	char str_filled_block[2] = { 219, 219 };
	for (int y = -border; y < size + border; y++) {
		for (int x = -border; x < size + border; x++) {
			fputs((qrcodegen_getModule(qrcode, x, y) ? str_filled_block : "  "), stdout);
		}
		fputs("\n", stdout);
	}
	fputs("\n", stdout);
}

/**
* Creates and shows in the console the QR code generated from the necessary data (API key, client ID, app ID, etc.)
*
* @return
**/
//void showQRDeepLink() {
//	const char* text = "secureworld://test/esto_es_un_qr_de_la_cmd";	// TO DO: change this with the real code including all necessary data
//
//	printf("\n\tYou have entered the QR linking option.\n");
//
//	// Make and print the QR Code symbol
//	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX] = { 0 };
//	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX] = { 0 };
//	BOOL ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_LOW,
//		qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//	if (ok)
//		printQr(qrcode);
//}


/**
* Creates a deciphered copy of the file given as parameter as saves it in the same path adding "_deciphered" at the end on the name (but before extension).
*
* @param WCHAR* input_file_name
*		The full path of the original file (e.g.: "M:/secureworld_project/paper/Manuscript.pdf").
*
* @return int
*		0 if everything is ok, other value if not.
**/
int createDecipheredFileCopy(WCHAR* input_file_path) {
	// This function will:
	// - Create a file in the same path adding "_deciphered" at the end (but before extension).
	// - Read the original file and call decipher() for all the content.
	// - TO DO: Add blockchain traces
	// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns an error code.

	HANDLE read_file_handle = NULL;
	HANDLE write_file_handle = NULL;

	size_t file_size = 0;
	LPVOID read_buf = NULL;
	LPVOID write_buf = NULL;
	DWORD rw_buf_size = 0;
	DWORD bytes_read = 0;
	DWORD bytes_written = 0;

	int8_t mark_lvl = 0;
	uint32_t frn = 0;

	struct Protection* protection = NULL;
	struct KeyData* composed_key = NULL;

	WCHAR* file_path = NULL;
	WCHAR* file_path_write = NULL;
	WCHAR* file_path_ext = NULL;
	size_t file_path_len = 0;
	size_t file_path_write_len = 0;
	size_t file_path_ext_len = 0;
	size_t path_cpy_pos = 0;

	LARGE_INTEGER distance_to_move = { 0 };

	DWORD error_code = ERROR_SUCCESS;
	DWORD result = ERROR_SUCCESS;

	// Get the path to the real (non-mirrored and non-surveilled) drive
	file_path = getRealPathFromMirrored(input_file_path);
	if (file_path == NULL) {
		error_code = -1;
		fprintf(stderr, "ERROR in createDecipheredFileCopy: cannot allocate memory for the clean file path.\n");
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	formatPath(&file_path);

	// Fill path lengths variables and check if path is short enough to add DECIPHERED_SUFFIX_WCS and still fit in MAX_PATH characters
	file_path_len = wcslen(file_path);
	file_path_write_len = file_path_len + DECIPHERED_SUFFIX_WCS_LEN;
	if (file_path_write_len >= MAX_PATH) {
		error_code = -1;
		fprintf(stderr, "ERROR in createDecipheredFileCopy: file path is too long to append (%ws)\n", DECIPHERED_SUFFIX_WCS);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	file_path_ext = PathFindExtensionW(file_path);	// Pointer to "." before the extension or to the L'\0' if path does not have extension.
	file_path_ext_len = wcslen(file_path_ext);

	// Compose the write path
	file_path_write = malloc((file_path_write_len + 1) * sizeof(WCHAR));
	//file_path_write = malloc((MAX_PATH) * sizeof(WCHAR));
	if (file_path_write == NULL) {
		fprintf(stderr, "ERROR in createDecipheredFileCopy: cannot allocate memory for the writing file path.\n");
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	path_cpy_pos = 0;
	wcsncpy_s(&(file_path_write[path_cpy_pos]), file_path_write_len + 1 - path_cpy_pos, file_path, file_path_len - file_path_ext_len);
	path_cpy_pos = file_path_len - file_path_ext_len;
	wcsncpy_s(&(file_path_write[path_cpy_pos]), file_path_write_len + 1 - path_cpy_pos, DECIPHERED_SUFFIX_WCS, DECIPHERED_SUFFIX_WCS_LEN);
	path_cpy_pos = file_path_len - file_path_ext_len + DECIPHERED_SUFFIX_WCS_LEN;
	wcsncpy_s(&(file_path_write[path_cpy_pos]), file_path_write_len + 1 - path_cpy_pos, file_path_ext, file_path_ext_len);
	file_path_write[file_path_write_len] = L'\0';


	// Open original file
	read_file_handle = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!read_file_handle || read_file_handle == INVALID_HANDLE_VALUE) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR in createDecipheredFileCopy: creating handle (error = %lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Get file size
	result = getFileSize(&file_size, read_file_handle, file_path);
	if (result != 0) {
		error_code = result;
		fprintf(stderr, "ERROR in createDecipheredFileCopy: getting file size (error = %lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	if (file_size == 0) {
		PRINT("File size is 0\n");
		error_code = -2;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Make sure read handle points to the beginning of the file
	if (!SetFilePointerEx(read_file_handle, distance_to_move, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR in createDecipheredFileCopy: read handle seeking position FILE_BEGIN (error=%lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Allocate read and write buffers
	//read_buf = malloc(file_size * sizeof(byte));		// May be too much (e.g. >100MB is A LOT)
	//read_buf = malloc(READ_BUF_SIZE * sizeof(byte));
	rw_buf_size = (DWORD)MIN(READ_BUF_SIZE, file_size);
	read_buf = malloc(rw_buf_size * sizeof(byte));
	if (read_buf == NULL) {
		fprintf(stderr, "ERROR in createDecipheredFileCopy: cannot allocate memory for read buffer.\n");
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}
	write_buf = malloc(rw_buf_size * sizeof(byte));
	if (write_buf == NULL) {
		fprintf(stderr, "ERROR in createDecipheredFileCopy: cannot allocate memory for write buffer.\n");
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Open write file
	write_file_handle = CreateFileW(file_path_write, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (write_file_handle == NULL || write_file_handle == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "ERROR opening write file (%ws)\n", file_path_write);
		error_code = ERROR_OPEN_FAILED;
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}

	// Make sure write handle points to the beginning of the file
	if (!SetFilePointerEx(write_file_handle, distance_to_move, NULL, FILE_BEGIN)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR in createDecipheredFileCopy: write handle seeking position FILE_BEGIN (error=%lu)\n", error_code);
		goto DECIPHERED_FILE_COPY_CLEANUP;
	}


	// Iterate enough times to process all the file
	//	- Read original file
	//	- Unmark if needed
	//	- Decipher if needed (always unless file is already on level -1)
	//	- Mark if needed
	//	- Write to the destination file
	for (size_t i = 0; i < 1 + (size_t)(file_size / READ_BUF_SIZE); i++) {
		// Read original file
		if (!ReadFile(
			read_file_handle,
			read_buf,
			rw_buf_size,
			&bytes_read,
			NULL)
			) {
			fprintf(stderr, "ERROR in createDecipheredFileCopy: trying to read the original file\n");
			error_code = ERROR_READ_FAULT;
			goto DECIPHERED_FILE_COPY_CLEANUP;
		}

		// Only do operations if file is bigger than MARK_LENGTH
		if (file_size >= MARK_LENGTH) {

			// If it is the first iteration, take care of the possible mark
			if (i == 0) {
				// Unmark if needed
				mark_lvl = unmark(read_buf, &frn);
			}

			// Decipher if needed (always unless file is already on level -1)
			if (mark_lvl == 0 || mark_lvl == 1) {
				// The first time it is needed to initialize the protection and composed key
				if (i == 0) {
					// Get the protection to use based on the file_path
					protection = getProtectionFromFilePath(file_path);	// Uses input_file_path because it is easier to retrieve the protection
					if (protection == NULL) {
						fprintf(stderr, "ERROR in createDecipheredFileCopy: could not get protection associated to the path.\n");
						error_code = -3;
						goto DECIPHERED_FILE_COPY_CLEANUP;
					}

					// Compose the key
					composed_key = protection->key;
					result = makeComposedKey(protection->challenge_groups, composed_key);
					if (0 != result) {
						fprintf(stderr, "ERROR in createDecipheredFileCopy: trying to compose key (%d)\n", result);
						error_code = -4;
						goto DECIPHERED_FILE_COPY_CLEANUP;
					}

					// Create a new FRN if the original file was cleartext (and therefore frn is invalid)
					if (frn == INVALID_FRN) {
						frn = createFRN();
					}
				}

				// Decipher
				invokeDecipher(protection->cipher, write_buf, read_buf, rw_buf_size, i * READ_BUF_SIZE, composed_key, frn);

				// If it is the first iteration, mark with one level less (1 --> 0, 0 --> -1)
				if (i == 0) {
					// Mark if needed
					mark(write_buf, mark_lvl - 1, frn);
				}
			} else {
				memcpy_s(write_buf, rw_buf_size, read_buf, rw_buf_size);
			}
		} else {
			// If there is no need to modify data, just copy the buffer
			memcpy_s(write_buf, rw_buf_size, read_buf, rw_buf_size);
		}

		// Write new content to file
		if (!WriteFile(
			write_file_handle,
			write_buf,
			bytes_read,
			&bytes_written,
			NULL)
			) {
			fprintf(stderr, "ERROR in createDecipheredFileCopy: could not write read data\n");
			error_code = ERROR_WRITE_FAULT;
			goto DECIPHERED_FILE_COPY_CLEANUP;
		}

		// Check all the bytes read from this iteration have been written
		if (bytes_read != bytes_written) {
			error_code = -5;
			fprintf(stderr, "ERROR in createDecipheredFileCopy: could not write all the read bytes\n");
			goto DECIPHERED_FILE_COPY_CLEANUP;
		}
	}


	// Make sure of freeing everything before leaving the function
	DECIPHERED_FILE_COPY_CLEANUP:
	if (file_path != NULL) {
		free(file_path);
	}
	if (read_file_handle != NULL) {
		CloseHandle(read_file_handle);
	}
	if (write_file_handle != NULL) {
		if (write_file_handle != INVALID_HANDLE_VALUE) {
			DeleteFileW(file_path_write);
		}
		CloseHandle(write_file_handle);
	}
	if (read_buf != NULL) {
		free(read_buf);
	}
	if (write_buf != NULL) {
		free(write_buf);
	}

	return error_code;
}

/**
* If the input file is a .pdf, creates a .uva version of the file given as parameter as saves it in the same path but changing the extension to ".uva".
*
* @param WCHAR* file_path
*		The full path of the original file (e.g.: "M:/OpticFiber/Datasheet.pdf").
* @param time_t allowed_visualization_period_begin
*		Date and time of the beginning of the allowed visualization period.
* @param time_t allowed_visualization_period_end
*		Date and time of the end of the allowed visualization period.
* @param struct ThirdParty* third_party
*		The struct containing information (including the public key) of the third party with which the file will be shared.
*
* @return int
*		0 if everything is ok, other value if not.
**/
int createUvaFileCopy(WCHAR* file_path, time_t allowed_visualization_period_begin, time_t allowed_visualization_period_end, struct ThirdParty* third_party) {
	printf("\t TO DO\n");
	// This function will:
	// - Check the file is a ".pdf" file.
	// - Create a file in the same path changing the extension to ".uva".
	// - Fill the .uva header with necessary metadata.
	//     - Look for the public RSA key of the third party and read it
	//     - Create a random key of 8 Bytes (=64 bits) for pdf ciphering
	//     - Create a 0-filled buffer of 470 bytes. This will become 512 Bytes (= uva header size) when ciphered with RSA
	//     - Write a uva version number to the buffer
	//     - Write the file ciphering key to the buffer
	//     - Write the allowed visualization frame to the buffer
	//     - The rest of the buffer is left as is (with 0s)
	//     - Cipher the buffer with the public RSA key previously read
	//     - Ensure new ciphered buffer length is 512 Bytes (= uva header size)
	//     - Write the new ciphered buffer to the .uva file
	// - Read the original file and call decipher() followed by cipherTP() for all the content while writting to the ".uva" file.
	// - Add blockchain traces
	// - If everything goes well, returns 0. In case something goes wrong, removes newly created file and returns -1.
	return 0;
}