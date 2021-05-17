#ifndef WRAPPER_DOKAN_H
#define WRAPPER_DOKAN_H


/////  FUNCTION PROTOTYPES  /////

int dokanMapAndLaunch(int index, WCHAR* path, WCHAR letter, WCHAR* volume_name, struct Cipher* cipher);
WCHAR* getAppPathDokan(PDOKAN_FILE_INFO dokan_file_info);


#endif // !WRAPPER_DOKAN_H
