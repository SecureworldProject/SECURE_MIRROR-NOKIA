#ifndef __MAIN_H
#define __MAIN_H


/////  FILE INCLUDES  /////

#include <Windows.h>
#include "dokan/DokanFiles/dokan.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")
#include "winfsp/winfspFiles/winfsp.h"



/////  DEFINITIONS  /////

// Uncomment to enable WinFSP functionality
#define ENABLE_WINFSP

// Uncomment to launch volume mounter thread
//#define RUN_VOLUME_MOUNTER

// Uncomment to launch fmi table test at the beginning
//#define RUN_FMI_TABLE_TEST

#define NUM_LETTERS 26




/////  STRUCTS AND ENUMS  /////

struct ThreadData {
	int index;
	WCHAR* path;
	WCHAR letter;
	WCHAR* name;
	struct Protection* protection;
};

struct VolumeMounterThreadData {
	size_t index;
	HANDLE (*threads_p)[NUM_LETTERS];
	struct ThreadData (*th_data_p)[NUM_LETTERS];
};

struct LetterDeviceMap {
	WCHAR letter;
	WCHAR device[MAX_PATH];
};




/////  GLOBAL VARS  /////

struct LetterDeviceMap* letter_device_table;




/////  FUNCTION PROTOTYPES  /////

int main(int argc, char* argv[]);
int threadDokan(struct ThreadData* th_data);
int threadWinFSP(struct ThreadData* th_data);
void decipherFileMenu();
void uvaFileMenu();
void initLetterDeviceMapping();
void initChallenges();
void initCiphers();

#endif // !__MAIN_H
