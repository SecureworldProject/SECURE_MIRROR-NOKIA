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
#include "securemirror_rsa.h"




/////  DEFINITIONS  /////

#define MAX_INPUT_LENGTH 500
#define READ_BUF_SIZE (1024 * 1024)		// 1 MB
#define DECIPHERED_SUFFIX_WCS L"_deciphered"
#define DECIPHERED_SUFFIX_WCS_LEN wcslen(DECIPHERED_SUFFIX_WCS)

#define PRIV_KEY_PEM_SUFFIX L"_priv.pem"
#define PUB_KEY_PEM_SUFFIX L"_pub.pem"
#define UVA_FILE_VERSION 1
#define DATETIME_FORMAT "%Y-%m-%d - %H:%M:%S"		// "YYYY-MM-DD - hh:mm:ss"
#define DATETIME_FORMAT_SIZE 22						// 4+1+2+1+2 +3+ 2+1+2+1+2 +1 for the '\0'
#define DATE_FORMAT "%d/%m/%Y"						// YYYY/MM/DD
#define DATE_FORMAT_SIZE 11							// 2+1+2+1+4 +1 for the '\0'
#define ALLOW_MIRRORED_PATHS_ONLY FALSE				// TRUE ensures that the path uses a mirrored letter. FALSE allows also paths starting with "C:\..." and "\\?\C:\..."

#define BLOCKCHAIN_JSON_FIELD_TIMESTAMP		"timestamp"
#define BLOCKCHAIN_JSON_FIELD_USERNAME		"usuario"
#define BLOCKCHAIN_JSON_FIELD_FILE			"file_name"
#define BLOCKCHAIN_JSON_FIELD_OPTYPE		"operation_type"

#define ENVVAR_BLOCKCHAIN_TRACE_FOLDER "SECUREMIRROR_BLOCKCHAIN_TRACE_FOLDER"
#define BLOCKCHAIN_TRACE_BASE_FILENAME "blockchainTask"
#define BLOCKCHAIN_DATETIME_FORMAT "%Y-%m-%d_%H%M%S"	// "YYYY-MM-DD_hhmmss"
#define BLOCKCHAIN_DATETIME_FORMAT_SIZE 18				// 4+1+2+1+2 +1+ 2+2+2 +1 (for '\0') = 18




/////  FUNCTION PROTOTYPES  /////
void printMenuHelp();
void decipherFileMenu();
void uvaFileMenu();
void newRSAKeypairMenu();
int createDecipheredFileCopy(WCHAR* file_path);
int createUvaFileCopy(WCHAR* pdf_file_path, struct tm* access_period_start, struct tm* access_period_end, struct ThirdParty* third_party);
void setBlockchainTrace(const WCHAR* wcs_sharable_file_name, const char* operation_type);
char* getBlockchainTimestamp();
void thirdPartyPdfBufferCipher(LPVOID* dst_buf, LPCVOID* src_buf, size_t buf_size, uint64_t pdf_key);
void resetAllChallenges();



/////  FUNCTION IMPLEMENTATIONS  /////

/**
* Shows a text-based interactive menu which allows sereral options like exitting, deciphering files, creating an uva file, etc.
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
		printf("  3) Generate and save third party RSA keypair\n");
		#ifdef SECUREMIRROR_DEBUG_MODE
		printf("  4) (Debug only) Print the File Mark Info Table\n");
		printf("  5) (Debug only) Test System\n");
		printf("  6) (Debug only) Unit Test\n");
		printf("  7) (Debug only) Print Unit Test Information\n");
		#endif
		printf("  8) Reset all challenges\n");
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
					case 8:
						resetAllChallenges();
						break;
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
* Shows the menu help for each of the options allowed like exitting, deciphering files, creating uva files, etc.
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
	}

	return;
}

/**
* Starts a text-based dialog with the user to gather all the necessary information to make a .uva copy of a .pdf file.
* Note: the resulting .uva file will allways be in a ciphering lvl of -1. This is not operative-dependant.It is assumed that pulling the file out of the system will cipher it.
*
* @return
**/
void uvaFileMenu() {
	WCHAR line[500] = { 0 };
	time_t current_time = 0;
	struct tm* current_time_info = NULL;
	struct tm* time_info[2] = { NULL, NULL };
	int integer_user_input = 0;
	char formatted_time[DATETIME_FORMAT_SIZE] = "";

	WCHAR* file_path = NULL;
	WCHAR* tmp_file_path = NULL;
	size_t file_path_len = 0;
	time_t allowed_visualization_period_begin = 0;
	time_t allowed_visualization_period_end = 0;
	struct ThirdParty* third_party = NULL;
	int result = 0;

	printf("\n\tYou have entered the .uva creation option.\n");

	file_path = malloc(MAX_PATH * sizeof(WCHAR));
	if (NULL == file_path) {
		fprintf(stderr, "\tError: could not allocate space for the path.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}
	// Get the path
	printf("\n\tEnter the full path of the file from which you want to create a .uva file below.\n");
	printf("\t--> ");
	if (fgetws(file_path, MAX_PATH, stdin)) {		// fgets() ensures that string ends with '\0'
		//file_path[wcscspn(file_path, L"\n")] = L'\0';		// Remove trailing '\n'
		file_path_len = wcscspn(file_path, L"\n");	// Returns the position of first '\n' or the length of the string if it is not contained
		file_path[file_path_len] = L'\0';			// Remove trailing '\n' if exists
		if (!PathFileExistsW(file_path)) {
			fprintf(stderr, "\tError: the specified path does not exist.\n");
			goto UVA_FILE_MENU_CLEANUP;
		}
		if (PathIsDirectoryW(file_path)) {
			fprintf(stderr, "\tError: the specified path matches a directory not a file.\n");
			goto UVA_FILE_MENU_CLEANUP;
		}
		if (0 != wcscmp(&(file_path[file_path_len - 4]), L".pdf")) {
			fprintf(stderr, "\tError: the specified path does not match a .pdf file.\n");
			goto UVA_FILE_MENU_CLEANUP;
		}


		// Get the path to the real (non-mirrored and non-surveilled) drive
		tmp_file_path = getRealPathFromMirrored(file_path);
#if ALLOW_MIRRORED_PATHS_ONLY
		if (NULL == tmp_file_path) {
			fprintf(stderr, "\tError: could not get the real file path.\n");
			goto UVA_FILE_MENU_CLEANUP;
		} else {
#else
		if (NULL != tmp_file_path) {
#endif
			free(file_path);
			file_path = tmp_file_path;
			file_path_len = wcslen(file_path);
		}
		formatPath(&file_path);
	}

	// Get current time
	if (time(&current_time) == -1) {
		fprintf(stderr, "\tError: could not retrieve current time.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}
	current_time_info = localtime(&current_time);

	// Get the allowed visualization period
	for (size_t i = 0; i < 2; i++) {
		time_info[i] = (struct tm*)malloc(1 * sizeof(struct tm));
		if (NULL == time_info) {
			fprintf(stderr, "\tError: could not allocate memory for the %s datetime.\n", (0 == i) ? "beginning" : "ending");
			goto UVA_FILE_MENU_CLEANUP;
		}
		memcpy(time_info[i], current_time_info, sizeof(struct tm));
		strftime(formatted_time, DATETIME_FORMAT_SIZE, DATETIME_FORMAT, time_info[i]);
		printf("\n\tEnter the date %s which the file will be accesible. Skipped values default to current date/time (%s).\n", (0 == i) ? "from" : "until", formatted_time);

		// Get the year
		printf("\t Year \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_year = integer_user_input - 1900;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_year + 1900);
			}
		}
		// Get the month
		printf("\t Month \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_mon = integer_user_input - 1;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_mon + 1);
			}
		}
		// Get the day
		printf("\t Day \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_mday = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_mday);
			}
		}
		// Get the hours
		printf("\t Hours \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_hour = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_hour);
			}
		}
		// Get the minutes
		printf("\t Minutes \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_min = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_min);
			}
		}
		// Get the secconds
		printf("\t Secconds \t --> ");
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
				PRINT2("Detected the number %d.\n", integer_user_input);
				time_info[i]->tm_sec = integer_user_input;
			} else {
				PRINT2("Value skipped, using current value %d.\n", time_info[i]->tm_sec);
			}
		}
	}

	// Fill weekday and day of the year and correct possible off-bound values in other fields (ie. tm_mon>11, tm_mday>31, etc.)
	allowed_visualization_period_begin = mktime(time_info[0]);
	allowed_visualization_period_end = mktime(time_info[1]);

	// Check that values are valid
	if (-1 == allowed_visualization_period_begin) {
		fprintf(stderr, "\tError: the beginning of the allowed visualization period is not valid.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}
	if (-1 == allowed_visualization_period_end) {
		fprintf(stderr, "\tError: the ending of the allowed visualization period is not valid.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}

	// Check that allowed_visualization_period ending is later than beginning
	if (difftime(allowed_visualization_period_end, allowed_visualization_period_begin) <= 0) {
		fprintf(stderr, "\tError: the ending of the allowed visualization period must be a later time than the beginning.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}

	// Get the third party to share with
	printf("\n\tSelect the third party you want to share the .uva file with:\n");
	for (size_t i = 0; i < _msize(ctx.third_parties) / sizeof(struct ThirdParty*); i++) {
		printf("\t  %llu) %s\n", i, ctx.third_parties[i]->id);
	}
	if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {
		if (1 == swscanf_s(line, L"%d", &integer_user_input)) {
			if (integer_user_input < 0 || integer_user_input > _msize(ctx.third_parties) / sizeof(struct ThirdParty*)) {
				fprintf(stderr, "\tError: there is no third party asigned to that number.\n");
				goto UVA_FILE_MENU_CLEANUP;
			}
			third_party = ctx.third_parties[integer_user_input];
		}
	}
	if (NULL == third_party) {
		fprintf(stderr, "\tError: the third party is NULL.\n");
		goto UVA_FILE_MENU_CLEANUP;
	}

	PRINT1("The .uva file is being created with the following information:\n");
	PRINT2("- Version:             %d\n", UVA_FILE_VERSION);
	strftime(formatted_time, DATETIME_FORMAT_SIZE, DATETIME_FORMAT, time_info[0]);
	PRINT2("- Access period start: %s\n", formatted_time);
	strftime(formatted_time, DATETIME_FORMAT_SIZE, DATETIME_FORMAT, time_info[1]);
	PRINT2("- Access period end:   %s\n", formatted_time);
	PRINT2("- Third party:         %s\n", third_party->id);
	PRINT2("- Payload file path:   '%ws'\n", file_path);
	PRINT("\n");

	result = createUvaFileCopy(file_path, time_info[0], time_info[1], third_party);

	if (result != 0) {
		fprintf(stderr, "\tError: there was an error while trying to create the .uva file. (errcode: %d)\n", result);
	} else {
		printf("\tThe .uva file was successfully created.\n");
	}

UVA_FILE_MENU_CLEANUP:
	if (NULL != file_path) {
		free(file_path);
		file_path = NULL;
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
	size_t priv_key_filepath_length = 0;
	size_t pub_key_filepath_length = 0;

	//RSA* rsa_keypair = NULL;
	WCHAR* priv_key_filepath = NULL;
	WCHAR* pub_key_filepath = NULL;

	keypair_directory = malloc(MAX_PATH * sizeof(WCHAR));
	if (NULL == keypair_directory) {
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
			if (keypair_directory[keypair_directory_length - 1] != L'/' && keypair_directory[keypair_directory_length - 1] != L'\\') {
				keypair_directory[keypair_directory_length] = L'\\';
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
		PRINT("\tThe specified directory is: '%ws' (%llu wchars)\n", keypair_directory, keypair_directory_length);

		// Get the filename
		keypair_filename = malloc((MAX_PATH - keypair_directory_length) * sizeof(WCHAR));
		if (NULL == keypair_filename) {
			printf("\tError: cannot allocate memory.\n");
			goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
		}

		printf("\n\tEnter below the filename (no extension) for the keypair files. Remember two files with distinguishable suffixes will be created.\n");
		printf("\t--> ");
		if (fgetws(keypair_filename, (MAX_PATH - keypair_directory_length), stdin)) {		// fgets() ensures that string ends with '\0'
			keypair_filename_length = wcscspn(keypair_filename, L"\n");	// Returns the position of first '\n' or the length of the string if it is not contained
			keypair_filename[keypair_filename_length] = L'\0';			// Remove trailing '\n' if exists

			if (MAX_PATH <= keypair_directory_length + keypair_filename_length + priv_key_pem_suffix_length) {
				printf("\tThe specified path is too long to append %ws.\n", PRIV_KEY_PEM_SUFFIX);
				goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
			}
			if (MAX_PATH <= keypair_directory_length + keypair_filename_length + pub_key_pem_suffix_length) {
				printf("\tThe specified path is too long to append %ws.\n", PUB_KEY_PEM_SUFFIX);
				goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
			}
			PRINT("\tThe specified filename is: '%ws' (%llu wchars)\n", keypair_filename, keypair_filename_length);
		}
	}
	priv_key_filepath_length = keypair_directory_length + keypair_filename_length + priv_key_pem_suffix_length + 1;
	priv_key_filepath = malloc((priv_key_filepath_length) * sizeof(WCHAR));
	if (NULL == priv_key_filepath) {
		printf("\tError: cannot allocate memory.\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}
	pub_key_filepath_length = (keypair_directory_length + keypair_filename_length + pub_key_pem_suffix_length + 1);
	pub_key_filepath = malloc(pub_key_filepath_length *sizeof(WCHAR));
	if (NULL == pub_key_filepath) {
		printf("\tError: cannot allocate memory.\n");
		goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	}
	//rsa_keypair = (RSA*)malloc(1 * sizeof(RSA*));
	//if (rsa_keypair == NULL) {
	//	printf("\tError: cannot allocate memory.\n");
	//	goto NEW_RSA_KEY_PAIR_MENU_CLEANUP;
	//}

	wcscpy_s(priv_key_filepath, priv_key_filepath_length, keypair_directory);
	wcscat_s(priv_key_filepath, priv_key_filepath_length, keypair_filename);
	wcscat_s(priv_key_filepath, priv_key_filepath_length, PRIV_KEY_PEM_SUFFIX);

	wcscpy_s(pub_key_filepath, pub_key_filepath_length, keypair_directory);
	wcscat_s(pub_key_filepath, pub_key_filepath_length, keypair_filename);
	wcscat_s(pub_key_filepath, pub_key_filepath_length, PUB_KEY_PEM_SUFFIX);

	if (ERROR_SUCCESS != generateAndWriteKey(RSA_F4, KEY_BYTES_SIZE * 8, priv_key_filepath, pub_key_filepath, NULL)) { //&rsa_keypair)) {
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
	if (NULL == file_path) {
#if ALLOW_MIRRORED_PATHS_ONLY
		error_code = -1;
		fprintf(stderr, "ERROR in createDecipheredFileCopy: could not get the real file path.\n");
		goto DECIPHERED_FILE_COPY_CLEANUP;
#else
		file_path = malloc((wcslen(input_file_path) + 1) * sizeof(WCHAR));
		if (NULL == file_path) {
			error_code = -1;
			fprintf(stderr, "ERROR in createDecipheredFileCopy: could not allocate memory for the file path.\n");
			goto DECIPHERED_FILE_COPY_CLEANUP;
		}
		wcscpy(file_path, input_file_path);
#endif
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
		fprintf(stderr, "ERROR in createDecipheredFileCopy: read handle seeking position FILE_BEGIN (error = %lu)\n", error_code);
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
		fprintf(stderr, "ERROR in createDecipheredFileCopy: write handle seeking position FILE_BEGIN (error = %lu)\n", error_code);
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

	// Register the potentially harmful operation in blockchain
	setBlockchainTrace(file_path_write, "Deciphered copy created");


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
* Assummes the file provided is a .pdf, the time structs are correct, and the third party is a valid pointer.
* Creates a .uva version of the file with the corresponding data and saves it in the same path but changing the extension to ".uva".
* If some error occurs during this function, the uva file is deleted.
*
* @param WCHAR* file_path
*		The full path of the original file (e.g.: "M:/OpticFiber/Datasheet.pdf").
* @param struct tm* access_period_start
*		Date and time of the beginning of the allowed visualization period.
* @param struct tm* access_period_end
*		Date and time of the end of the allowed visualization period.
* @param struct ThirdParty* third_party
*		The struct containing information (including the public key) of the third party with which the file will be shared.
*
* @return int
*		0 if everything is ok, other value if not.
**/
int createUvaFileCopy(WCHAR* pdf_file_path, struct tm* access_period_start, struct tm* access_period_end, struct ThirdParty* third_party) {
	// This function will:
	// - Create a file in the same path changing the extension to ".uva".
	// - Fill the .uva header with necessary metadata.
	//     - Check that the public RSA key file of the third party exists
	//     - Read the public RSA key of the third party
	//     - Create a random key of 8 Bytes (=64 bits) for pdf ciphering
	//     - Create a 0-filled buffer of 470 bytes. This will become 512 Bytes (= uva header size) when ciphered with RSA
	//     - Write the uva version number (UVA_FILE_VERSION) to the buffer
	//     - Write the pdf-ciphering key to the buffer
	//     - Write the allowed visualization frame to the buffer
	//     - The rest of the buffer is left as is (with 0s)
	//     - Cipher the buffer with the public RSA key previously read
	//     - Ensure new ciphered buffer length is 512 Bytes (= uva header size)
	//     - Get the protections associated to the file path and compose the key
	//     - Decrypt the header buffer with the folder protections
	//     // Mark the header buffer with the lvl (-1) and frn. /// This part was changed becaused it was not possible to mark the ciphered buffer due to high entropy /////
	//     - Create an extra buffer (with no info) that can be marked. It was not possible to mark the ciphered buffer due to high entropy created by the RSA ciphering
	//     - Write extra header buffer
	//     - Write the final uva header buffer (folder-deciphered and previously RSA-ciphered) to the .uva file
	// - Open pdf file
	// - Get pdf file size
	// - Ensure that the read handle points to the beginning of the file (maybe getting file size could have moved it?)
	// - Allocate read, ciphering and write buffers
	// - Process the pdf file in chunks (of READ_BUF_SIZE bytes) until all the file is processed
	//     - Read the pdf file
	//     - Ensure that pdf file is cleartext
	//     - Cipher chunk with the pdf key
	//     - Decipher chunk with the folder key
	//     - Append the result to the ".uva" file and check all the bytes read from this iteration have been written
	// - Register the potentially harmful operation in blockchain
	// - Clear the buffers and close the handles used. In the case that something went wrong, remove the uva file (if created)
	// Note: if everything goes well, returns 0. In case something goes wrong, removes newly created file and returns an error code.


	size_t file_path_len = 0;
	WCHAR* uva_file_path = NULL;
	HANDLE write_file_handle = NULL;
	uint8_t* rand_buf = NULL;
	uint64_t pdf_key = 0;
	RSA* rsa_pub_key = NULL;

	uint8_t* cleartext_header_buf = NULL;
	size_t cleartext_header_buf_size = KEY_BYTES_SIZE - PADDING_BYTES_SIZE; //512-42 = 470
	uint8_t* encrypted_header_buf = NULL;
	size_t encrypted_header_buf_size = 0;
	uint8_t* final_uva_header_buf = NULL;
	size_t final_uva_header_buf_size = 0;
	uint8_t* extra_header_buf = NULL;
	size_t extra_header_buf_size = 0;

	struct Protection* protection = NULL;
	struct KeyData* composed_key = NULL;
	int8_t mark_lvl = 0;
	uint32_t frn = 0;

	char* formatted_date = NULL;

	HANDLE read_file_handle = INVALID_HANDLE_VALUE;
	size_t file_size = 0;
	LARGE_INTEGER distance_to_move = { 0 };

	LPVOID read_buf = NULL;
	LPVOID ciph_buf = NULL;
	LPVOID write_buf = NULL;
	DWORD rw_buf_size = 0;
	DWORD bytes_read = 0;
	DWORD bytes_written = 0;
	uint32_t unsused_frn = 0;

	size_t result = 0;
	DWORD err_code = ERROR_SUCCESS;


	// Create the uva path (same as the pdf but different extension)
	file_path_len = wcslen(pdf_file_path);
	//PRINT("wcslen = %llu\n", file_path_len);

	uva_file_path = malloc((file_path_len + 1) * sizeof(WCHAR));	// +1 for '\0'
	if (NULL == uva_file_path) {
		fprintf(stderr, "ERROR: could not allocate space for the uva file path\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	wcscpy_s(uva_file_path, file_path_len + 1, pdf_file_path);
	wcscpy_s(&(uva_file_path[file_path_len - 3]), 4, L"uva");
	PRINT("The uva will be created in: '%ws'\n", uva_file_path);

	// Create a file in the same path changing the extension to ".uva".

	write_file_handle = CreateFileW(uva_file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == write_file_handle) {
		fprintf(stderr, "ERROR: could not create the empty uva file\n");
		err_code = ERROR_OPEN_FAILED;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}

	// Check that the public RSA key file of the third party exists
	FILE* tmp_file;
	PRINT("The third party key file is: '%s'\n", third_party->key_file);
	tmp_file = fopen(third_party->key_file, "r");
	if (NULL == tmp_file) {
		fprintf(stderr, "ERROR: the RSA key file does not exist\n");
		err_code = ERROR_OPEN_FAILED;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	fclose(tmp_file);


	// Read the public RSA key of the third party
	rsa_pub_key = readPublicKey(third_party->key_file);
	if (NULL == rsa_pub_key) {
		printf("ERROR: could not read pub key (%s)\n", third_party->key_file);
		err_code = ERROR_READ_FAULT;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
#pragma warning(suppress : 4996)
	if (KEY_BYTES_SIZE != RSA_size(rsa_pub_key)) {
#pragma warning(suppress : 4996)
		printf("ERROR: the size of the RSA key was %d instead of the expected 512\n", RSA_size(rsa_pub_key));
	}


	// Create a random key of 8 Bytes (=64 bits) for pdf ciphering
	printf("\nNOTE: A 64-bit system is assummed from this poiint onwards\n\n");
	rand_buf = malloc(8 * sizeof(uint8_t));
	if (NULL == rand_buf) {
		fprintf(stderr, "ERROR: could not allocate space for the random key\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
#if TRUE	// TRUE generates random number; FALSE uses a fixed pdf key
	getRandom(rand_buf, 8);
#else
	uint8_t testing_not_rand_buf[8] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 }; // In decimal: 578.721.382.704.613.384
	memcpy(rand_buf, testing_not_rand_buf, 8 * sizeof(uint8_t));
#endif
	pdf_key = ((uint64_t*)rand_buf)[0]; // Little Endianness is assumend here. Anyway, this is being developed for Windows systems which only use Little Endian enabled CPUs
	PRINT_HEX(rand_buf, 8 * sizeof(uint8_t));
	PRINT("The pdf key is (as uint64_t): %llu\n", pdf_key);


	// Create a 0-filled buffer of 470 bytes. This will become 512 Bytes (= uva header size) when ciphered with RSA
	cleartext_header_buf = (uint8_t*)calloc(cleartext_header_buf_size, sizeof(uint8_t));
	if (NULL == cleartext_header_buf) {
		fprintf(stderr, "ERROR: could not allocate space for the uva file header\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Write the uva version number (UVA_FILE_VERSION) to the buffer
	cleartext_header_buf[0] = (uint8_t)UVA_FILE_VERSION;


	// Write the pdf-ciphering key to the buffer
	cleartext_header_buf[1] = rand_buf[0];
	cleartext_header_buf[2] = rand_buf[1];
	cleartext_header_buf[3] = rand_buf[2];
	cleartext_header_buf[4] = rand_buf[3];
	cleartext_header_buf[5] = rand_buf[4];
	cleartext_header_buf[6] = rand_buf[5];
	cleartext_header_buf[7] = rand_buf[6];
	cleartext_header_buf[8] = rand_buf[7];


	// Write the allowed visualization frame to the buffer
	formatted_date = malloc(DATE_FORMAT_SIZE * sizeof(uint8_t));
	if (NULL == formatted_date) {
		fprintf(stderr, "ERROR: could not allocate space for formatting the date\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	strftime(formatted_date, DATE_FORMAT_SIZE, DATE_FORMAT, access_period_start);
	strcpy(&(cleartext_header_buf[9]), formatted_date);
	strftime(formatted_date, DATE_FORMAT_SIZE, DATE_FORMAT, access_period_end);
	strcpy(&(cleartext_header_buf[9+ DATE_FORMAT_SIZE]), formatted_date);


	// The rest of the buffer is left as is (with 0s)
	// DO NOTHING because space was allocated with calloc() function, which already fills with zeros


	PRINT("from 512 to 1024 in cleartext and without RSA ciphering (470 Bytes only)\n");
	PRINT_HEX(cleartext_header_buf, cleartext_header_buf_size);

	// Cipher the buffer with the public RSA key previously read
#pragma warning(suppress : 4996)
	encrypted_header_buf = (unsigned char*)malloc(RSA_size(rsa_pub_key));
	if (NULL == encrypted_header_buf) {
		fprintf(stderr, "ERROR: could not allocate space for the RSA-encrypted uva file header\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
#pragma warning(suppress : 4996)
	encrypted_header_buf_size = RSA_public_encrypt(cleartext_header_buf_size, cleartext_header_buf, encrypted_header_buf, rsa_pub_key, RSA_PKCS1_OAEP_PADDING); //RSA_NO_PADDING);
	if (-1 == encrypted_header_buf_size) {
		err_code = ERR_get_error();
		printf("ERROR: could not encrypt the uva header with the RSA key --> %s\n", ERR_error_string(err_code, NULL));
		err_code = -1;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Ensure new ciphered buffer length is 512 Bytes (= uva header size)
	if (KEY_BYTES_SIZE != encrypted_header_buf_size) {
		printf("ERROR: encrypted_header_buf_size must always be %d\n", KEY_BYTES_SIZE);
		err_code = -2;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	PRINT("from 512 to 1024 which is cleartext ciphered with RSA (512 Bytes in total)\n");
	PRINT_HEX(encrypted_header_buf, encrypted_header_buf_size);



	// Get the protections associated to the file path and compose the key
	//PRINT("uva_file_path='%ws'\n", uva_file_path == NULL ? L"NULL" : uva_file_path);
	//PRINT("pdf_file_path='%ws'\n", pdf_file_path);
	//PRINT("third_party->key_file='%s'\n", third_party->key_file);
	protection = getProtectionFromFilePath(pdf_file_path);
	if (protection == NULL) {
		fprintf(stderr, "ERROR: could not get protection associated to the path.\n");
		err_code = -3;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	composed_key = protection->key;
	result = makeComposedKey(protection->challenge_groups, composed_key);
	if (0 != result) {
		fprintf(stderr, "ERROR: could not compose the key for deciphering (error code: %llu)\n", result);
		err_code = -4;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Decrypt the header buffer with the folder protections
	final_uva_header_buf_size = encrypted_header_buf_size; // The "folder/challenges" decryption does not change size, so it will stay 512 Bytes
	final_uva_header_buf = malloc(final_uva_header_buf_size);
	if (NULL == final_uva_header_buf) {
		fprintf(stderr, "ERROR: could not allocate space for the folder-decrypted, RSA-encrypted uva file header\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	frn = createFRN();
	invokeDecipher(protection->cipher, final_uva_header_buf, encrypted_header_buf, encrypted_header_buf_size, MARK_LENGTH, protection->key, frn);
	if (512 != encrypted_header_buf_size) {
		printf("ERROR: the size of the encrypted header buffer is %llu instead of the expected 512\n", encrypted_header_buf_size);
	}
	PRINT("from 512 to 1024 cleartext, ciphered with RSA, and then deciphered with challenges (512 Bytes in total)\n");
	PRINT_HEX(encrypted_header_buf, encrypted_header_buf_size);


	///// This part was changed becaused it was not possible to mark the ciphered buffer due to high entropy /////
	// Mark the header buffer with the lvl (-1) and frn.
	/*
	PRINT_HEX(final_uva_header_buf, final_uva_header_buf_size);
	mark_lvl = mark(final_uva_header_buf, -1, frn);
	if (-1 != mark_lvl) {
		fprintf(stderr, "ERROR: could not mark the folder-decrypted, RSA-encrypted uva file header. mark_lvl=%d\n", mark_lvl);
		err_code = -5;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	*/

	// Create an extra buffer (with no info) that can be marked. It was not possible to mark the ciphered buffer due to high entropy created by the RSA ciphering
	extra_header_buf_size = MARK_LENGTH;
	extra_header_buf = calloc(extra_header_buf_size, sizeof(uint8_t));
	if (NULL == final_uva_header_buf) {
		fprintf(stderr, "ERROR: could not allocate space for the markable extra header buffer for the uva file header\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	PRINT("from 0 to 512 cleartext (zeros)\n");
	PRINT_HEX(extra_header_buf, extra_header_buf_size);
	mark_lvl = mark(extra_header_buf, -1, frn);
	if (-1 != mark_lvl) {
		fprintf(stderr, "ERROR: could not mark the folder-decrypted, RSA-encrypted uva file header. mark_lvl=%d\n", mark_lvl);
		err_code = -5;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	PRINT("from 0 to 512 cleartext (zeros) but marked (note it is not deciphered because it contains no info)\n");
	PRINT_HEX(extra_header_buf, extra_header_buf_size);


	// Write extra header buffer
	WriteFile(write_file_handle, extra_header_buf, extra_header_buf_size, &bytes_written, NULL);
	if (MARK_LENGTH != bytes_written) {
		printf("ERROR: could not write the expected %d bytes (%lu were written)\n", MARK_LENGTH, bytes_written);
		err_code = ERROR_WRITE_FAULT;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Write the final uva header buffer (folder-deciphered and previously RSA-ciphered) to the .uva file
	WriteFile(write_file_handle, final_uva_header_buf, final_uva_header_buf_size, &bytes_written, NULL);
	if (KEY_BYTES_SIZE != bytes_written) {
		printf("ERROR: could not write the expected %d bytes (%lu were written)\n", KEY_BYTES_SIZE, bytes_written);
		err_code = ERROR_WRITE_FAULT;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Open pdf file
	read_file_handle = CreateFileW(pdf_file_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (NULL == read_file_handle || INVALID_HANDLE_VALUE == read_file_handle) {
		err_code = GetLastError();
		fprintf(stderr, "ERROR: could not create handle (error = %lu)\n", err_code);
		err_code = ERROR_OPEN_FAILED;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Get pdf file size
	err_code = getFileSize(&file_size, read_file_handle, pdf_file_path);
	if (0 != err_code) {
		fprintf(stderr, "ERROR: could not get file size (error = %lu)\n", err_code);
		err_code = ERROR_READ_FAULT;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	if (0 == file_size) {
		PRINT("File size is 0\n");
		err_code = -6;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Ensure that the read handle points to the beginning of the file (maybe getting file size could have moved it?)
	if (!SetFilePointerEx(read_file_handle, distance_to_move, NULL, FILE_BEGIN)) {
		err_code = GetLastError();
		fprintf(stderr, "ERROR: could not set read handle pointer to position FILE_BEGIN (error = %lu)\n", err_code);
		err_code = -7;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// Allocate read, ciphering and write buffers
	rw_buf_size = (DWORD)MIN(READ_BUF_SIZE, file_size);
	read_buf = malloc(rw_buf_size * sizeof(byte));
	if (read_buf == NULL) {
		fprintf(stderr, "ERROR: could not allocate memory for reading buffer.\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	ciph_buf = malloc(rw_buf_size * sizeof(byte));
	if (ciph_buf == NULL) {
		fprintf(stderr, "ERROR: could not allocate memory for the pdf-ciphered buffer.\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}
	write_buf = malloc(rw_buf_size * sizeof(byte));
	if (write_buf == NULL) {
		fprintf(stderr, "ERROR: could not allocate memory for writing buffer.\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CREATE_UVA_FILE_COPY_CLEANUP;
	}


	// - Iterate enough times to process all the file
	//     - Read original file
	//     - Ensure that pdf file is cleartext
	//     - Cipher chunk with the pdf key
	//     - Decipher chunk with the folder key.
	//     - Append the result to the ".uva" file.
	for (size_t i = 0; i < 1 + (size_t)(file_size / READ_BUF_SIZE); i++) {
		// Read the pdf file
		if (!ReadFile(
			read_file_handle,
			read_buf,
			rw_buf_size,
			&bytes_read,
			NULL)
			) {
			fprintf(stderr, "ERROR: trying to read the original file\n");
			err_code = ERROR_READ_FAULT;
			goto CREATE_UVA_FILE_COPY_CLEANUP;
		}

		// Ensure that pdf file is cleartext
		if (file_size >= MARK_LENGTH) {

			// If it is the first iteration, take care of the possible mark
			if (i == 0) {
				// Check if it is marked
				mark_lvl = unmark(read_buf, &unsused_frn);

				// If mark level is not 0. Abort
				if (0 != mark_lvl) {
					fprintf(stderr, "ERROR: the pdf file is not cleartext\n");
					err_code = -8;
					goto CREATE_UVA_FILE_COPY_CLEANUP;
				}
			}
		}


		// Cipher chunk with the pdf key
		thirdPartyPdfBufferCipher(ciph_buf, read_buf, (size_t)bytes_read, pdf_key);


		// Decipher chunk with the folder key
		invokeDecipher(protection->cipher, write_buf, ciph_buf, rw_buf_size, MARK_LENGTH + KEY_BYTES_SIZE + (i * READ_BUF_SIZE), composed_key, frn);

		// For traces only
		if (i == 0) {
			PRINT("from 1024 to 1088 cleartext (corresponds to the first 64 Bytes of the PDF)\n");
			PRINT_HEX(read_buf, 64);
			PRINT("from 1024 to 1088 ciphered with the pdf key (corresponds to the first 64 Bytes of the PDF)\n");
			PRINT_HEX(ciph_buf, 64);
			PRINT("from 1024 to 1088 ciphered with the pdf key, and then deciphered with challenges (corresponds to the first 64 Bytes of the PDF)\n");
			PRINT_HEX(write_buf, 64);
		}

		// Append the result to the ".uva" file and check all the bytes read from this iteration have been written
		if (!WriteFile(
			write_file_handle,
			write_buf,
			bytes_read,
			&bytes_written,
			NULL)
			) {
			fprintf(stderr, "ERROR: could not write the data\n");
			err_code = ERROR_WRITE_FAULT;
			goto CREATE_UVA_FILE_COPY_CLEANUP;
		}
		if (bytes_read != bytes_written) {
			err_code = -9;
			fprintf(stderr, "ERROR: could not write all the read bytes\n");
			goto CREATE_UVA_FILE_COPY_CLEANUP;
		}
	}


	// Register the potentially harmful operation in blockchain
	setBlockchainTrace(uva_file_path, "UVA file creation");
	err_code = ERROR_SUCCESS;



	// Clear all the buffers and handles used. In the case that something went wrong, remove the uva file (if created)
CREATE_UVA_FILE_COPY_CLEANUP:
	if (NULL != rand_buf) {
		free(rand_buf);
	}
	if (NULL != formatted_date) {
		free(formatted_date);
	}

	if (NULL != cleartext_header_buf) {
		free(cleartext_header_buf);
	}
	if (NULL != encrypted_header_buf) {
		free(encrypted_header_buf);
	}
	if (NULL != final_uva_header_buf) {
		free(final_uva_header_buf);
	}

	if (NULL != read_buf) {
		free(read_buf);
	}
	if (NULL != ciph_buf) {
		free(ciph_buf);
	}
	if (NULL != write_buf) {
		free(write_buf);
	}

	if (NULL != write_file_handle && INVALID_HANDLE_VALUE != write_file_handle) {
		CloseHandle(write_file_handle);
	}
	if (NULL != read_file_handle && INVALID_HANDLE_VALUE != read_file_handle) {
		CloseHandle(read_file_handle);
	}

	if (NULL != uva_file_path) {
		if (ERROR_SUCCESS != err_code) { // Delete the file if there was an error
			if (!PathFileExistsW(uva_file_path)) { // Only if file exists
				if (0 == DeleteFileW(uva_file_path)) {
					fprintf(stderr, "ERROR: could not remove the newly created file\n");
				}
			}
		}
		free(uva_file_path);
	}
	return err_code;
}

void setBlockchainTrace(const WCHAR* wcs_sharable_file_name, const char* operation_type) {
	LPCVOID write_buf = NULL;
	DWORD err_code = ERROR_SUCCESS;
	char* file_path = NULL;
	size_t file_path_len = 0;
	int extra_byte = 0; // for the slash between the folder and filename

	FILE *fptr = NULL;

	char* blockchain_ts = getBlockchainTimestamp();

	char* username = getenv("USERNAME");
	char* default_trace_folder = getenv(ENVVAR_BLOCKCHAIN_TRACE_FOLDER);
	char* base_filename = BLOCKCHAIN_TRACE_BASE_FILENAME;
	char* sharable_file_name = NULL;
	size_t sharable_file_name_len = 0;

	// Crete a char version of the WCHAR sharable file name
	sharable_file_name_len = wcslen(wcs_sharable_file_name);
	sharable_file_name = malloc((sharable_file_name_len + 1) * sizeof(char));
	if (NULL == sharable_file_name) {
		fprintf(stderr, "ERROR: could not allocate memory for the sharable filepath.\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	wcstombs(sharable_file_name, wcs_sharable_file_name, sharable_file_name_len);
	// Clean backward slashes
	for (size_t i = 0; i < sharable_file_name_len; i++) {
		if ('\\' == sharable_file_name[i]) {
			sharable_file_name[i] = '/';
		}
	}

	printf("GENERATING BLOCKCHAIN TRACE WITH THE FOLLOWING DATA:\n\t%s\n\t%s\n\t'%s'\n\t%s\n",
		((NULL != blockchain_ts) ? blockchain_ts : "????-??-??_??????"),
		((NULL != username) ? username : "UNKNOWN USER"),
		sharable_file_name,
		operation_type);


	if (NULL == blockchain_ts) {
		fprintf(stderr, "ERROR: could get timestamp.\n");
		err_code = -1;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (NULL == default_trace_folder) {
		fprintf(stderr, "ERROR: could find environment variable %s.\n", ENVVAR_BLOCKCHAIN_TRACE_FOLDER);
		err_code = ERROR_ENVVAR_NOT_FOUND;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}

	// Concatenate base file path
	if (default_trace_folder[strlen(default_trace_folder) - 1] != '/' &&
		default_trace_folder[strlen(default_trace_folder) - 1] != '\\') {
		extra_byte = 1;
	}
	file_path_len = strlen(default_trace_folder) + extra_byte + strlen(base_filename) + 1 + (BLOCKCHAIN_DATETIME_FORMAT_SIZE - 1) + 4;
	file_path = calloc(file_path_len + 1, sizeof(char));
	if (NULL == file_path) {
		fprintf(stderr, "ERROR: could not allocate memory for the file path.\n");
		err_code = ERROR_NOT_ENOUGH_MEMORY;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	strcpy(file_path, default_trace_folder);
	if (1 == extra_byte) {
		strcat(file_path, "\\");
	}
	strcat(file_path, base_filename);
	strcat(file_path, "_");
	strcat(file_path, blockchain_ts);
	strcat(file_path, ".json");


	// Size is basically:
	//		1 for the curly brace opening ('{')
	//		1 for every line feed ('\n')
	//		1 for every tab ('\t') or space (' ')
	//		1 for every colon (':')
	//		1 for every quotation marks ('"')
	//		1 for every comma (',')
	//		1 for the curly brace closing ('}')
	//		Note: ending '\0' will NOT be included, due to variable being writing size of a binary file, not a string


	// Open a file in writing mode
	PRINT("file_path: '%s'\n", file_path);
	//char* file_path_test = "C:\\blockchain_traces/blockchainTask adf 00";
	fptr = fopen(file_path, "w");
	if (NULL == fptr) {
		fprintf(stderr, "ERROR in setBlockchainTrace: opening file for writing (fopen: %s)\n", strerror(errno));
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}

	/*{
		"timestamp":"2023-11-07 - 12:05:00",
		"username":"Sergio",
		"file_name":"C:\Users\Pepito\Trabajo\Router_Model_R12A1_Specs.uva"
		"operation_type":"UVA creation",
	}*/


	// Write
	if (0 >= fprintf(fptr, "{\n")) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (0 >= fprintf(fptr, "\t\"%s\":\"%s\",\n", BLOCKCHAIN_JSON_FIELD_TIMESTAMP, blockchain_ts)) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (0 >= fprintf(fptr, "\t\"%s\":\"%s\",\n", BLOCKCHAIN_JSON_FIELD_USERNAME, username)) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (0 >= fprintf(fptr, "\t\"%s\":\"%s\",\n", BLOCKCHAIN_JSON_FIELD_FILE, sharable_file_name)) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (0 >= fprintf(fptr, "\t\"%s\":\"%s\"\n", BLOCKCHAIN_JSON_FIELD_OPTYPE, operation_type)) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}
	if (0 >= fprintf(fptr, "}\n")) {
		fprintf(stderr, "ERROR: could not write the data\n");
		err_code = ERROR_WRITE_FAULT;
		goto SET_BLOCKCHAIN_TRACE_CLEANUP;
	}


	SET_BLOCKCHAIN_TRACE_CLEANUP:
	if (NULL != blockchain_ts) {
		free(blockchain_ts);
	}
	if (NULL != fptr) {
		fclose(fptr);
		free(fptr);
	}
	//if (NULL != username) { //The string pointed to shall not be modified by the program
	//	free(username);
	//}
}

char* getBlockchainTimestampOLD() {
	time_t current_time = 0;
	struct tm* current_time_info = NULL;
	char* formatted_time = NULL;

	// Get current time
	if (time(&current_time) == -1) {
		fprintf(stderr, "ERROR: could not retrieve current time\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}
	current_time_info = localtime(&current_time);
	if (NULL == current_time_info) {
		fprintf(stderr, "ERROR: could not convert current time (time_t to time_info)\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}

	// Format the timestamp
	formatted_time = malloc(DATETIME_FORMAT_SIZE * sizeof(char));
	if (NULL == formatted_time) {
		fprintf(stderr, "ERROR: could not allocate memory for the formatted time\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}
	strftime(formatted_time, DATETIME_FORMAT_SIZE, DATETIME_FORMAT, current_time_info);

	return formatted_time;

GET_BLOCKCHAIN_TIMESTAMP:
	//if (NULL != current_time_info) {	// Must not be freed because it is created the first time and reused every other time the function localtime() get called
	//	free(current_time);
	//}
	if (NULL != current_time_info) {
		free(formatted_time);
	}
	return NULL;
}
char* getBlockchainTimestamp() {
	time_t current_time = 0;
	struct tm* current_time_info = NULL;
	char* formatted_time = NULL;

	// Get current time
	if (time(&current_time) == -1) {
		fprintf(stderr, "ERROR: could not retrieve current time\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}
	current_time_info = localtime(&current_time);
	if (NULL == current_time_info) {
		fprintf(stderr, "ERROR: could not convert current time (time_t to time_info)\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}

	// Format the timestamp
	formatted_time = malloc(BLOCKCHAIN_DATETIME_FORMAT_SIZE * sizeof(char));
	if (NULL == formatted_time) {
		fprintf(stderr, "ERROR: could not allocate memory for the formatted time\n");
		goto GET_BLOCKCHAIN_TIMESTAMP;
	}
	strftime(formatted_time, BLOCKCHAIN_DATETIME_FORMAT_SIZE, BLOCKCHAIN_DATETIME_FORMAT, current_time_info);

	return formatted_time;

GET_BLOCKCHAIN_TIMESTAMP:
	//if (NULL != current_time_info) {	// Must not be freed because it is created the first time and reused every other time the function localtime() get called
	//	free(current_time);
	//}
	if (NULL != current_time_info) {
		free(formatted_time);
	}
	return NULL;
}


void thirdPartyPdfBufferCipher(LPVOID* dst_buf, LPCVOID* src_buf, size_t buf_size, uint64_t pdf_key) {
	uint64_t* u64_buff = NULL;
	size_t u64_buff_size = 0;

	// Copy the data, as this example does not need extra buffers
	memcpy_s(dst_buf, buf_size, src_buf, buf_size);

	// Cipher all complete blocks of 8 Bytes (64 bits)
	u64_buff = (uint64_t*)dst_buf;
	u64_buff_size = buf_size / sizeof(uint64_t);
	for (size_t i = 0; i < u64_buff_size; i++) {
		u64_buff[i] ^= pdf_key;
	}

	// Cipher the remaining bytes. Actually, this will only happen at the end of the file
	//for (size_t j = 0; j < buf_size - u64_buff_size * sizeof(uint64_t); j++) {
	//	((uint8_t*)dst_buf)[j + u64_buff_size * sizeof(uint64_t)] ^= ((uint8_t*)(&pdf_key))[j];
	//}

	return;
}

void resetAllChallenges() {
	size_t num_ch_groups = _msize(ctx.groups)/sizeof(struct ChallengeEquivalenceGroup*);

	for (size_t i = 0; i < num_ch_groups; i++) {
		// Expire challenge group key
		ctx.groups[i]->subkey->expires = (time_t)0;

		// Remove the minifilter related parental group files if they exist
		updateParentalChSuccessFile(ctx.groups[i], (byte)0xFF);
	}
}
