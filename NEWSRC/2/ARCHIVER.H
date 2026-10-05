// v 1.0
//  Archivers.h
//  MIDAS Module Player for Windows NT archiver support
//  Copyright 1997 Petteri Kangaslampi

#ifndef __archivers_h
#define __archivers_h

typedef struct
{
    char  *ext;
    char  *cmd;
} Archive;

extern int      numArchives;
extern Archive  archives[];

// Function:     int IsArchive(char *fileName);
// Checks if a file is an archive (based on the extension)
// Returns:      1 if the file is an archive, 0 if not

int IsArchive(char *fileName);

// Function:     MIDASmodule LoadArchive(char *arcName, char *fileName);
// Loads a module from an archive
// Returns:      MIDAS module handle for the module or NULL if failed

MIDASmodule LoadArchive(char *arcName, char *fileName);

// fills an existing listbox 'wnd' with a contents of .RAR archive
void fill_RAR( HANDLE wnd, LPCSTR arcName );

// fills an existing listbox 'wnd' with a contents of .ZIP archive
void fill_ZIP( HANDLE wnd, LPCSTR arcName );

// fills an existing listbox 'wnd' with a contents of .ARJ archive
void fill_ARJ( HANDLE wnd, LPCSTR arcName );

#endif
