#include <Windows.h>
#include "dokan.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "context.c"


int dokanMapAndLaunch(char* path, char letter) {
	PDOKAN_OPTIONS dokan_options;
	PDOKAN_OPERATIONS dokan_operations;

	dokan_options = malloc(sizeof(DOKAN_OPTIONS));
	dokan_operations = malloc(sizeof(DOKAN_OPERATIONS));
	ZeroMemory(&dokan_options, sizeof(DOKAN_OPTIONS));

	//dokan_options->RootDirectory = path;		/////////////////////// UNCOMMENT
	dokan_options->MountPoint = letter;



	/*
	ZeroMemory(&dokan_operations, sizeof(DOKAN_OPERATIONS));
	dokan_operations.ZwCreateFile = MirrorCreateFile;                  // Will be modified
	dokan_operations.Cleanup = MirrorCleanup;
	dokan_operations.CloseFile = MirrorCloseFile;
	dokan_operations.ReadFile = MirrorReadFile;                        // Will be modified
	dokan_operations.WriteFile = MirrorWriteFile;                      // Will be modified
	dokan_operations.FlushFileBuffers = MirrorFlushFileBuffers;
	dokan_operations.GetFileInformation = MirrorGetFileInformation;    // Will be modified parental control??
	dokan_operations.FindFiles = MirrorFindFiles;
	dokan_operations.FindFilesWithPattern = NULL;
	dokan_operations.SetFileAttributes = MirrorSetFileAttributes;
	dokan_operations.SetFileTime = MirrorSetFileTime;
	dokan_operations.DeleteFile = MirrorDeleteFile;
	dokan_operations.DeleteDirectory = MirrorDeleteDirectory;
	dokan_operations.MoveFile = MirrorMoveFile;
	dokan_operations.SetEndOfFile = MirrorSetEndOfFile;
	dokan_operations.SetAllocationSize = MirrorSetAllocationSize;
	dokan_operations.LockFile = MirrorLockFile;
	dokan_operations.UnlockFile = MirrorUnlockFile;
	dokan_operations.GetFileSecurity = MirrorGetFileSecurity;
	dokan_operations.SetFileSecurity = MirrorSetFileSecurity;
	dokan_operations.GetDiskFreeSpace = MirrorDokanGetDiskFreeSpace;
	dokan_operations.GetVolumeInformation = MirrorGetVolumeInformation;
	dokan_operations.Unmounted = MirrorUnmounted;
	dokan_operations.FindStreams = MirrorFindStreams;
	dokan_operations.Mounted = MirrorMounted;
	*/

	int status;
	status = DokanMain(&dokan_options, &dokan_operations);

	switch (status) {
		case DOKAN_SUCCESS:
			fprintf(stderr, "Success\n");
			break;
		case DOKAN_ERROR:
			fprintf(stderr, "Error\n");
			break;
		case DOKAN_DRIVE_LETTER_ERROR:
			fprintf(stderr, "Bad Drive letter\n");
			break;
		case DOKAN_DRIVER_INSTALL_ERROR:
			fprintf(stderr, "Can't install driver\n");
			break;
		case DOKAN_START_ERROR:
			fprintf(stderr, "Driver something wrong\n");
			break;
		case DOKAN_MOUNT_ERROR:
			fprintf(stderr, "Can't assign a drive letter\n");
			break;
		case DOKAN_MOUNT_POINT_ERROR:
			fprintf(stderr, "Mount point error\n");
			break;
		case DOKAN_VERSION_ERROR:
			fprintf(stderr, "Version error\n");
			break;
		default:
			fprintf(stderr, "Unknown error: %d\n", status);
			break;
	}
	return EXIT_SUCCESS;
}