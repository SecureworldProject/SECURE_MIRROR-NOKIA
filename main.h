#ifndef __MAIN_H
#define __MAIN_H


/////  FILE INCLUDES  /////

#include <Windows.h>
#include "dokan.h"
#include <synchapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")
#include <fileapi.h>


/////  DEFINITIONS  /////

#define NUM_LETTERS 26



/////  STRUCTS AND ENUMS  /////

struct ThreadData {
	int thread_id;
	WCHAR letter;
	WCHAR* path;
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
void decipherMenu();
void initLetterDeviceMapping();


#endif // !MAIN_H
