#ifndef context2_h
#define context2_h

#include "windows.h"


/////  FUNCTION PROTOTYPES  /////

static struct App* createApp();
static void destroyApp(struct App** app);

enum Operation getTableOperation(enum IrpOperation irp_operation, WCHAR* app_full_path, WCHAR* file_full_path);

inline struct OpTable* getTable(WCHAR* file_full_path);
inline struct App* getApp(WCHAR* app_full_path);
inline WCHAR getDiskType(WCHAR* file_full_path);
inline enum Operation* getOperations(enum AppType app_type, struct OpTable* table);
void formatPathOLD(char** full_path);
void formatPath(WCHAR** full_path);
int fromDeviceToLetter(WCHAR** full_path);



/////  STRUCTS AND ENUMS  /////

enum IrpOperation {
	ON_READ,
	ON_WRITE
};
#define NUM_IRP_OPERATIONS 2


// FOR FUTURE USE - performance improvement: create a context for each volume (each mirror thread) and a context for each file handle (in PDOKAN_FILE_INFO.Context)
/*
struct VolumeContext {
	union {
		struct Folder* folder;		// Pointer to the struct folder of this volume
		struct Pendrive* pendrive;	// Pointer to the struct folder of this volume
	};
	WCHAR** sync_folders;			// Array of synchronization folders that affect this volume (after conversion from "C:\..." to "M:\...", "N:\...", etc)
	BOOL is_pendrive;				// If this volume is a pendrive (detected from USB and mounted as such)
};

struct FileContext {
	struct App* app;
	BOOL is_sync_folder;
};
*/

#endif //!context2_h