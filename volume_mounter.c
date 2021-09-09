
/////  FILE INCLUDES  /////

#include "volume_mounter.h"




/////  FUNCTION PROTOTYPES  /////

DWORD findNotMountedVolumes(WCHAR volumes_to_mount[NUM_LETTERS][MAX_PATH], size_t* num_volumes_to_mount);
DWORD securelyMountVolumes(WCHAR volumes_to_mount[NUM_LETTERS][MAX_PATH], size_t num_volumes_to_mount, size_t thread_data_first_index, HANDLE threads[NUM_LETTERS], struct ThreadData th_data[NUM_LETTERS]);
void mountVolume(int index, WCHAR* path, WCHAR letter, WCHAR* name, struct Protection* protection, HANDLE threads[NUM_LETTERS], struct ThreadData* th_data);

BOOL hasVolumePaths(__in PWCHAR volume_name);
PWCHAR getVolumePaths(__in PWCHAR volume_name);
void getFreeLetters(DWORD logical_drives_mask, WCHAR available_letters[NUM_LETTERS]);
int getBusType(WCHAR path[]);



/////  FUNCTION DEFINITIONS  /////

void volumeMounterThread(struct VolumeMounterThreadData *vol_mount_th_data) {
	// List of volume names (names are like "\\?\Volume{090dccd9-a5f5-4983-b685-ddd6331ef319}\"). GUIDs are characters from index 10 ('{') to  47('}')
	WCHAR volumes_to_mount[NUM_LETTERS][MAX_PATH] = { 0 };

	// The number of volumes in the "volumes_to_mount" list 
	size_t num_volumes_to_mount = 0;

	// Variable to hold an error value
	DWORD error = ERROR_SUCCESS;

	// The number of static mounted volumes. Which is the number of indexes occupied in "th_data" and "threads" arrays
	size_t thread_data_first_index = 0;

	// The handle of every wrapper thread launched
	HANDLE *threads = NULL;					// We know it is an array of NUM_LETTERS elements but we use a pointer so not to allocate memory for it
	//HANDLE(*threads_p)[NUM_LETTERS] = NULL;

	// The data for every wrapper thread launched
	struct ThreadData* th_data = NULL;		// We know it is an array of NUM_LETTERS elements but we use a pointer so not to allocate memory for it
	//struct ThreadData(*th_data_p)[NUM_LETTERS] = NULL;

	// Extract info from the vol_mount_th_data structure. threads and th_data come as pointer to array, so indirection is needed
	thread_data_first_index = vol_mount_th_data->index;
	threads = *(vol_mount_th_data->threads_p);
	th_data = *(vol_mount_th_data->th_data_p);

	// Keep checking forever if there have been new USB devices connected
	while (TRUE) {
		num_volumes_to_mount = 0;

		// Find the volumes that have not been mounted yet and put them in the list
		error = findNotMountedVolumes(volumes_to_mount, &num_volumes_to_mount);

		if (error == ERROR_SUCCESS){
			if (num_volumes_to_mount > 0) {
				error = securelyMountVolumes(volumes_to_mount, num_volumes_to_mount, thread_data_first_index, threads, th_data);

				if (error == ERROR_SUCCESS) {
					PRINT("Volume mounted correctly\n");
				} else {
					PRINT("Error in volume mounter. Mounting volume (%lu)\n", error);
				}
			} // Not finding volumes is not an error
		} else {
			PRINT("Error in volume mounter. Volumes found=%llu (%lu)\n", num_volumes_to_mount, error);
		}

		Sleep(2000);
	}
}


DWORD findNotMountedVolumes(WCHAR volumes_to_mount[NUM_LETTERS][MAX_PATH], size_t* num_volumes_to_mount) {
	// VARIABLES //
	// List of letters available as mount points (A:\, B:\, D:\, E:\, etc.)
	WCHAR available_letters[NUM_LETTERS] = { 0 };

	// Handle for the volume search
	HANDLE search_handle = INVALID_HANDLE_VALUE;

	// String for containing the volume name
	WCHAR volume_name[MAX_PATH] = L"";

	// Variable to hold an error value
	DWORD error = ERROR_SUCCESS;

	// Boolean that indicates when to add the current found volume to the list
	BOOL add_to_list = TRUE;

	// Will contain the length of a volume_name
	size_t len = 0;

	// Will contain the enum bus type
	int bus_type;

	// Boolean that indicates when to stop the volume search
	BOOL success = TRUE;


	// CODE //
	// Set the number of volumes to mount initially to zero
	*num_volumes_to_mount = 0;

	// Start volume search
	search_handle = FindFirstVolumeW(volume_name, ARRAYSIZE(volume_name));

	// Check the search handleis valid
	if (search_handle == INVALID_HANDLE_VALUE) {
		error = GetLastError();
		PRINT("FindFirstVolumeW failed with error code %d\n", error);
		return error;
	}

	// Do operations and iterate over the rest of volumes
	do {
		PRINT("\nThe volume name is: %ws\n", volume_name);

		add_to_list = TRUE;

		// Check bus type is USB
		len = wcslen(volume_name);
		volume_name[len-1] = L'\0';		// Remove last character ('\')
		bus_type = getBusType(volume_name);
		PRINT1("BUS TYPE is (%d), which means that it %s a USB drive\n", bus_type, ((bus_type == BusTypeUsb) ? "IS" : "is NOT"));
		volume_name[len-1] = L'\\';		// Set again the removed '\'
		if (bus_type != BusTypeUsb) {
			add_to_list = FALSE;
		}

		// Check if the volume already has a mount path
		if (add_to_list) {
			if (hasVolumePaths(volume_name)) {
				add_to_list = FALSE;
			}
		}

		// If add_to_list is true, copy the string in the list of volumes to mount and increment index
		if (add_to_list) {
			wcscpy_s(volumes_to_mount[*num_volumes_to_mount], MAX_PATH, volume_name);
			(*num_volumes_to_mount)++;
		}

		// Find the next volume
		success = FindNextVolumeW(search_handle, volume_name, ARRAYSIZE(volume_name));

	} while (success);		// Continue iterating until search does not produce more volume results

	// Close the volume search
	FindVolumeClose(search_handle);
	search_handle = INVALID_HANDLE_VALUE;

	// Check cause of iteration finish and print error if necessary
	error = GetLastError();

	if (error != ERROR_NO_MORE_FILES) {
		PRINT("FindNextVolumeW failed with error code %d\n", error);
		return error;
	}
	error = ERROR_SUCCESS;


	// Print all volumes "volumes_to_mount" list
	/*
	PRINT("\n\nThe volumes to mount are the following:\n");
	for (size_t i = 0; i < *num_volumes_to_mount; i++) {
		PRINT1("%ws \n", volumes_to_mount[i]);
	}
	PRINT("\n\n");
	*/

	return error;
}


DWORD securelyMountVolumes(
	WCHAR volumes_to_mount[NUM_LETTERS][MAX_PATH], size_t num_volumes_to_mount,
	size_t thread_data_first_index, HANDLE threads[NUM_LETTERS], struct ThreadData th_data[NUM_LETTERS]
){
	// Starts in 0 and increments every time a volume is mounted. It is static so keeps its value across severall calls.
	static size_t adding_thread_data_index = 0;

	// Base path in which the folders for the volumes will be created
	const WCHAR base_path[] = L"C:\\drives\\";

	// Path to the folder where a volume will be mounted
	WCHAR new_folder_path[MAX_PATH] = { 0 };

	// List of letters available as mount points (A:\, B:\, D:\, E:\, etc.)
	WCHAR available_letters[NUM_LETTERS] = { 0 };

	// Will contain the first available letter in the list
	WCHAR first_available_letter = '\0';



	// Get the letters available (letters not used)
	getFreeLetters(GetLogicalDrives(), available_letters);


	// Iterate over the volumes to mount
	PRINT("\nIterating over the volumes to mount...\n");
	for (size_t i = 0; i < NUM_LETTERS; i++) {
		///  For every volume in "volumes_to_mount": create a folder, mount the volume on the new folder and securely mirror it on a letter  ///

		// Check name of volume (if there is no volume, continue with next one)
		PRINT1("VOL: '%ws'\n", volumes_to_mount[i]);
		PRINT2("len: %llu\n", wcslen(volumes_to_mount[i]));
		if (wcslen(volumes_to_mount[i]) == 0) {
			PRINT2("Skipping...\n");		// This should never happen (I guess)...
			continue;
		}

		// Check if the base_path exists. Create it if it does not
		if (!PathIsDirectoryW(base_path)) {
			PRINT2("Base path is NOT ok. Creating it...\n");
			CreateDirectoryW(base_path, NULL);
			PRINT2("Base path created.\n");
		} else {
			PRINT2("Base path is OK.\n");
		}

		// Create new folder inside base_path with the name of the volume GUID
		wcscpy_s(new_folder_path, MAX_PATH, base_path);
		wcscat_s(new_folder_path, MAX_PATH - 10, &(volumes_to_mount[i][10]));	// 10 is the length of the string "\\?\Volume" that precedes the GUID
		//PRINT("FP: %ws\n", new_folder_path);

		#pragma warning(disable:6054)   // new_folder_path will be always '\0' terminated
		CreateDirectoryW(new_folder_path, NULL);
		#pragma warning(default:6054)


		// Mount the volume in the new folder
		// eg.: SetVolumeMountPointW("C:\\drives\\{090dccd9-a5f5-4983-b685-ddd6331ef319}\\", "\\\\?\\Volume{090dccd9-a5f5-4983-b685-ddd6331ef319}\\");
		SetVolumeMountPointW(new_folder_path, volumes_to_mount[i]);
		PRINT1("MOUNTED %ws in %ws\n", volumes_to_mount[i], new_folder_path);


		// Get the first available letter that is allowed by configuration (ctx.pendrive->mount_points)
		for (size_t j = 0; j < NUM_LETTERS; j++) {
			if (available_letters[j] != (WCHAR)'\0' && wcschr(ctx.pendrive->mount_points, available_letters[j]) != NULL) {
				first_available_letter = available_letters[j];
				available_letters[j] = '\0';
				PRINT2("LETTER: %wc \n", first_available_letter);
				break;
			}
		}

		// Mount the created folder in the selected letter
		mountVolume(thread_data_first_index + adding_thread_data_index, new_folder_path, first_available_letter, L"Automounted", ctx.pendrive->protection, threads, th_data);
		adding_thread_data_index++;
	}

	return 0;
}


void mountVolume(int index, WCHAR* path, WCHAR letter, WCHAR* name, struct Protection* protection, HANDLE threads[NUM_LETTERS], struct ThreadData* th_data) {
	struct ThreadData data;
	data.index = index;
	data.path = path;
	data.letter = letter;
	data.name = name;
	data.protection = protection;

	switch (ctx.pendrive->driver) {
		case DOKAN:
			threads[index] = CreateThread(NULL, 0, threadDokan, &th_data[index], 0, NULL);
			break;
		case WINFSP:
			threads[index] = CreateThread(NULL, 0, threadWinFSP, &th_data[index], 0, NULL);
			break;
		default:
			break;
	}

	return;
}



/// LOWER LEVEL FUNCTIONS ///

/**
* Returns all the paths associated to the volume passed as parameter.
* Note: memory for the returned ponter is allocated inside, remember to free after using.
*
* @param PWCHAR volume_name
*       The name of the volume "\\?\Volume{GUID}\" to get the associated paths from
*
* @return PWCHAR
*       A pointer to WCHAR with all the associated paths sepparated by '\0' characters and with an extra trailing '\0' at the end.
		Memory for the ponter is allocated inside, remember to free after using.
**/
PWCHAR getVolumePaths(__in PWCHAR volume_name) {
	DWORD  char_count = MAX_PATH + 1;    // +1 for the null character
	PWCHAR names = NULL;
	BOOL   success = FALSE;

	for (;;) {
		// Allocate a buffer to hold the paths.
		names = malloc(char_count * sizeof(WCHAR));

		// If memory can't be allocated, return.
		if (!names) {
			return NULL;
		}

		// Obtain all of the paths for this volume.
		success = GetVolumePathNamesForVolumeNameW(volume_name, names, char_count, &char_count);
		if (success) {
			break;
		}

		if (GetLastError() != ERROR_MORE_DATA) {
			break;
		}

		// Try again with the new suggested size.
		free(names);
		names = NULL;
	}

	return names;
}

// TO BE UPDATED TO DO THE CHECKING WITHOUT ALLOCATING AND THEN FREEING MEMORY FOR THE PATHS. THEN "getVolumePaths" COULD BE DELETED
/**
* Returns if the volumes passed as parameter has any associated path.
*
* @param PWCHAR volume_name
*       The name of the volume "\\?\Volume{GUID}\" to check if it has any associated path
*
* @return BOOL
*       Returns TRUE if the volume has any path associated; FALSE otherwise.
**/
BOOL hasVolumePaths(__in PWCHAR volume_name) {
	PWCHAR paths = NULL;
	BOOL has_paths = FALSE;

	paths = getVolumePaths(volume_name);

	//PRINT("First path: '%ws'\n", paths);                   // Prints the first path
	//PRINT("Legnth of first path: %llu\n", wcslen(paths));  // Prints the length of the first path (if it is 0, then there are no paths)

	// Check if there is any path (length of first path >= 0)
	if (wcslen(paths) > 0) {
		PRINT("Has at least one path.\n");
		has_paths = TRUE;
	} else {
		//PRINT("Does not have any path.\n");
	}

	free(paths);

	return has_paths;
}


/**
* Fits all available letters in the array (and fills the already assigned with '\0')
*
* @param DWORD logical_drives_mask
*       Obtained from GetLogicalDrives() function. A bitmask representing the available disk drives (From least significant to most: bit 0 is drive A, bit 1 is B, etc.)
* @param WCHAR available_letters[NUM_LETTERS]
*       Array of NUM_LETTERS number of slots to contain the letters available as mountpoints
*/
void getFreeLetters(DWORD logical_drives_mask, WCHAR available_letters[NUM_LETTERS]) {
	for (size_t i = 0; i < NUM_LETTERS; i++) {
		available_letters[i] = (WCHAR)((logical_drives_mask ^ 1 << i) ? 'A' + i : '\0');
	}
}

/**
* Returns the STORAGE_BUS_TYPE value associated to the volume passed as parameter.
*
* @param PWCHAR volume_name
*       The name of the volume without the last backslash ("\\?\Volume{GUID}").
*
* @return int
*       The STORAGE_BUS_TYPE value associated to the volume passed as parameter.
**/
int getBusType(WCHAR path[]) {

	// Handle to the device
	HANDLE device_handle;

	// The number of bytes written to the output buffer in the function DeviceIoControl()
	DWORD bytes_out = 0;

	// The query to ask to the device
	STORAGE_PROPERTY_QUERY query;
	ZeroMemory(&query, sizeof(STORAGE_PROPERTY_QUERY));		// Zero out the input param for query

	// Buffer with the result of the query
	char out_buffer[256] = { 0 };
	PSTORAGE_DEVICE_DESCRIPTOR device_descriptor = (PSTORAGE_DEVICE_DESCRIPTOR)out_buffer;
	device_descriptor->Size = sizeof(out_buffer);

	// Boolean to indicate success or not
	BOOL success = FALSE;


	// Create handle for the device
	device_handle = CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	// Check if the handle is valid
	if (device_handle == INVALID_HANDLE_VALUE) {
		PRINT("INVALID CreateFileW HANDLE\n");
		return BusTypeUnknown;
	}

	// Query using IOCTL_STORAGE_QUERY_PROPERTY
	success = DeviceIoControl(
		device_handle,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY),
		device_descriptor, device_descriptor->Size,
		&bytes_out,
		NULL
	);

	// Close handle
	CloseHandle(device_handle);

	// Return corresponding STORAGE_BUS_TYPE or BusTypeUnknown (=0) if query was unsuccessful
	if (success) {
		return device_descriptor->BusType;
	}

	return BusTypeUnknown;
}
