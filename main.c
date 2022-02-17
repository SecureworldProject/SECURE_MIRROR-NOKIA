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

#include "qrcodegen.h"
static void printQr(const uint8_t qrcode[]);
// Prints the given QR Code to the console.
static void printQr(const uint8_t qrcode[]) {
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
static void doBasicDemo(void);
// Creates a single QR Code, then prints it to the console.
static void doBasicDemo(void) {
	const char* text = "secureworld://test/esto_es_un_qr_de_la_cmd";                // User-supplied text
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

	// Make and print the QR Code symbol
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	BOOL ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
		qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (ok)
		printQr(qrcode);
}



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
	//doBasicDemo(); return 0;

	// Test the FMI table
	#ifdef RUN_FMI_TABLE_TEST
		testFMItable();
	#endif //RUN_FMI_TABLE_TEST

	// Fill the table of equivalences between harddiskvolumes and letters
	initLetterDeviceMapping();

	// Load context from config.json
	loadContext();

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

	// Initialize the parameters for the challenges
	initChallenges();

	// Initialize the parameters for the ciphers
	initCiphers();

	// Forever loop checking for new pendrives
	Sleep(2000);

	// Sharing menu
	sharingMainMenu();
}


int threadDokan(struct ThreadData *th_data) {
	dokanMapAndLaunch(th_data->index, th_data->path, th_data->letter, th_data->name, th_data->protection);

	return 0;
}

int threadWinFSP(struct ThreadData *th_data) {
	#ifdef ENABLE_WINFSP
		winfspMapAndLaunch(th_data->index, th_data->path, th_data->letter, th_data->name, th_data->protection);
	#else
		PRINT("WinFSPMapAndLaunch parameters:   index=%2d     letter=%wc     path='%ws' \t\t\t (not implemented yet)\n", th_data->index, th_data->letter, th_data->path);
	#endif // ENABLE_WINFSP

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


	for (size_t i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
		for (size_t j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct Challenge*); j++) {
			// define function pointer corresponding with init() input and output types
			init_func = (init_func_type)GetProcAddress(ctx.groups[i]->challenges[j]->lib_handle, "init");

			// Add parameters if necessary
			if (init_func!=NULL) {
				result = init_func(ctx.groups[i], ctx.groups[i]->challenges[j]);
				if (result != 0) {
					PRINT("WARNING: error trying to initialize the challenge '%ws'\n", ctx.groups[i]->challenges[j]->file_name);
				} else {
					break;		// Stop initializing more challenges in the group when one is already working
				}
			} else{
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
			} else {
				break;		// Stop initializing more challenges in the group when one is already working
			}
		} else {
			PRINT("WARNING: error accessing the address to the init() function of the cipher '%ws' (error: %d)\n", ctx.ciphers[i]->file_name, GetLastError());
		}
	}
}
