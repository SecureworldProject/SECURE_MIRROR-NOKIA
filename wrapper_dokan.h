#ifndef WRAPPER_DOKAN_H
#define WRAPPER_DOKAN_H



/////  FUNCTION PROTOTYPES  /////

int dokanMapAndLaunch(WCHAR* path, WCHAR letter, int index);
WCHAR* getAppPathDokan(PDOKAN_FILE_INFO dokan_file_info);


#endif // !WRAPPER_DOKAN_H
