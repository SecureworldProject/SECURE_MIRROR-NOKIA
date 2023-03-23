#ifndef __MAIN_H
#define __MAIN_H

/////  DEFINITIONS  /////

// Uncomment to enable WinFSP functionality
#define ENABLE_WINFSP

// Uncomment to launch volume mounter thread
#define RUN_VOLUME_MOUNTER

// Uncomment to launch fmi table test at the beginning
//#define RUN_FMI_TABLE_TEST

// Uncomment to print context at the beginning
//#define RUN_PRINT_CONTEXT

// Uncomment to start debug mode (includes debugging options in the sharing menu and not quitting when dll fails to load)
#define SECUREMIRROR_DEBUG_MODE


#define NUM_LETTERS 26

// Returns the index (position in the alphabet starting at 0) associated to the letter (allows lower/upper case and char/WCHAR types)
#define DEVICE_LETTER_TO_INDEX(LET)	(('a' <= (LET) && (LET) <= 'z') ? ((LET)-'a') :		\
									('A' <= (LET) && (LET) <= 'Z') ? ((LET)-'A') :		\
									(L'a' <= (LET) && (LET) <= L'z') ? ((LET)-L'z') :	\
									(L'A' <= (LET) && (LET) <= L'Z') ? ((LET)-L'Z') :	\
									-1)
// Returns the upper case (char type) letter associated to the index (position in the alphabet starting at 0)
#define INDEX_TO_DEVICE_LETTER(IND) (IND)+'A'




/////  FILE INCLUDES  /////

#include <Windows.h>
#include "dokan/DokanFiles/dokan.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")
#ifdef ENABLE_WINFSP
	#include "winfsp/winfspFiles/winfsp.h"
#endif //ENABLE_WINFSP





/////  STRUCTS AND ENUMS  /////

struct ThreadData {
	int index;
	WCHAR* path;
	WCHAR letter;
	WCHAR* name;
	struct Protection* protection;
	WCHAR volume_name[MAX_PATH];		// Volume names are like "\\?\Volume{090dccd9-a5f5-4983-b685-ddd6331ef319}\")
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

struct ExecuteChallengeData {
	CRITICAL_SECTION critical_section;
	BOOL request_running;
	struct ChallengeEquivalenceGroup* ch_group;
	int ch_index;
	int result;
};





/////  GLOBAL VARS  /////

extern struct LetterDeviceMap* letter_device_table;
extern BOOL testing_mode_on;





/////  FUNCTION PROTOTYPES  /////

int main(int argc, char* argv[]);
int threadDokan(struct ThreadData* th_data);
int threadWinFSP(struct ThreadData* th_data);
void decipherFileMenu();
void uvaFileMenu();
void initLetterDeviceMapping();
void initChallenges();
void initCiphers();
void initCritSectPyIfNeeded(HMODULE lib_handle);
void challengeExecutorLoop();
int execChallengeFromMainThread(struct ChallengeEquivalenceGroup* ch_group, int ch_index);


#endif // !__MAIN_H
