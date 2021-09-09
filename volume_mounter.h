#ifndef VOLUME_MOUNTER_H
#define VOLUME_MOUNTER_H


/////  FILE INCLUDES  /////

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")         // Same as including library manually in: Project->Properties->Linker->Input->Additional dependencies

#include "main.h"



/////  FILE INCLUDES  /////

void volumeMounterThread(struct VolumeMounterThreadData* vol_mount_th_data);


#endif // !VOLUME_MOUNTER_H
