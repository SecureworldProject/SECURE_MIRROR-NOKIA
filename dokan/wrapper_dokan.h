#ifndef WRAPPER_DOKAN_H
#define WRAPPER_DOKAN_H


/////  DEFINITIONS  /////

// Uncomment to print create operations
//#define PRINT_OP_CREATE
// Uncomment to print cleanup operations
//#define PRINT_OP_CLEANUP
// Uncomment to print close operations
//#define PRINT_OP_CLOSE




/////  FUNCTION PROTOTYPES  /////

int dokanMapAndLaunch(WCHAR* path, WCHAR letter, WCHAR* volume_name, struct Protection* protection);


#endif // !WRAPPER_DOKAN_H
