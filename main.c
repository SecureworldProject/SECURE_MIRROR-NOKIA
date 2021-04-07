
#include <Windows.h>
#include "dokan.h"
#include <synchapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "context.c"


struct ThreadData {
	int thread_id;
	char letter;
	char* path;
};


int main(int argc, char* argv[]);
int threadDokan(struct ThreadData* th_data);
int threadWinFSP(struct ThreadData* th_data);


int main(int argc, char* argv[]) {

	int i;
	HANDLE threads[25];
	struct ThreadData th_data[25];
	char line[500];
	int choice;
	char file_path[MAX_PATH];
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
		printf(" 3) Exit (also closes mirrors)\n");
		if (fgets(line, sizeof(line), stdin)) {
			if (1 == sscanf(line, "%d", &choice)) {
				switch (choice) {
					case 1:
						printf("Selected DECIPHER mode.\n");
						// go to decipher menu
						break;
					case 2:
						printf("Selected CIPHER mode.\n");
						// go to cipher menu
						break;
					case 3:
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


int threadDokan(struct ThreadData *th_data) {
	//dokanMapAndLaunch(th_data.path, th_data.letter);		////////////////// UNCOMMENT
	while (TRUE) {
		printf("Hello, thread %d reporting alive.\n", th_data->thread_id);
		Sleep(3000);
	}

	return 0;
}

int threadWinFSP(struct ThreadData *th_data) {
	//winFSPMapAndLaunch(th_data.path, th_data.letter);		////////////////// UNCOMMENT

	return 0;
}
