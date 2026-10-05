#ifndef __Midproto_h
#define __Midproto_h
// common prototypes for MIDAS player for Windows NT
// copyright 1997,98 Alexander Milukov

// uses COMMDLG to get a full filename
char *GetFileName(char *windowTitle, int *path);

void CloseMIDAS(void);
void DDEError(char *function, UINT error);
int  PlayDDE(char *fileName);
void PlayFile( char *fileName);
void ReadRegistry(void);
void RestoreViews(void);
void RestoreMainWindow(void);
void SaveViews(void);
void StartDDEServer(void);
void WriteRegistry(void);

// Description:  Gets the last MIDAS error, displays it, and exits
void MIDASerror(void);

// Description:  Exits MidpNT with a "MIDP Panic" message
void Panic(char *message);

// Description:  Adds a line of text to main window edit control
void AddTextLine(char *textLine);

// Description:  Stops playing a module and deallocates it
// Input:        MIDASmodule module      MIDAS module handle returned by
//                                       LoadPlayModule().
void StopFreeModule( MIDASmodulePlayHandle playHandle, MIDASmodule module);

// Start up interface to MIDAS Sound System
void StartupMIDAS(void);

// Initializes MIDAS Sound System
void InitMIDAS(void);

// Uninitializes MIDAS Sound System
void CloseMIDAS(void);

// Description:  Loads a module file
// Returns:      MIDAS module handle, NULL if loading failed.
MIDASmodule LoadModule(char *fileName);

// Description:  Starts playing a module
// Returns:      MIDAS module handle, NULL if loading failed.
MIDASmodulePlayHandle PlayModule(MIDASmodule module);

// adds the ASCIIZ line of text into array of pointers
// updated by Mil for '\n' handling: now I can add some lines at once
void AddTextLine(char *textLine);

#endif
