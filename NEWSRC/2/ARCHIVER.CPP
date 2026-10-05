// v 1.0
// Archivers.cpp
// MIDAS Module Player for Windows NT archiver support
// portions Copyright 1996 Petteri Kangaslampi
// added .ARJ .ZIP .RAR archive support Copyright 1997 Alexander Milukov
// ported to bcpp from Pascal sources 'ZIP View (c) D.Vinogradov  1993'

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <dos.h>

#include "midasdll.h"
#include "midproto.h"

#ifdef __VC32__
#include <io.h>
#endif

#include "archiver.h"

extern HWND  mainWinHandle;
extern char  unzipDir[MAX_PATH];
extern char  cArcname[80];     // name of archive containing .s3m .xm etc.
extern int   isArcname;

int             numArchives = 7;
Archive         archives[7] =
    {
        { ".mdz", "pkunzip -o \"%s\" -d %s %s" },
        { ".s3z", "pkunzip -o \"%s\" -d %s %s" },
        { ".xmz", "pkunzip -o \"%s\" -d %s %s" },
        { ".itz", "unzip -o \"%s\" -d %s %s" },
        { ".zip", "pkunzip -o \"%s\" -d %s %s\\" },
        { ".arj", "arj e -y \"%s\" %s %s\\" },
        { ".rar", "rar e -y -std \"%s\" %s %s\\" }
    };

// Function:     int IsArchive(char *fileName);
// Description:  Checks if a file is an archive (based on the extension)
// Returns:      1 if the file is an archive, 0 if not

int IsArchive(char *fileName)
{
    int         i;
    char        ext[_MAX_EXT];

    _splitpath(fileName, NULL, NULL, NULL, ext);

    for ( i = 0; i < numArchives; i++ )
    {
        if ( !stricmp(ext, archives[i].ext) ) return 1;
    }
    return 0;
}

// summary of decompression error(s)
MIDASmodule DecompErr( char *msg )
{
    MessageBox(NULL, msg, "Decompression error",
        MB_OK | MB_ICONERROR);
    return NULL;
};

extern int unzipMinimized;

// Function:     MIDASmodule LoadArchive(char arcName, char *fileName);
// Description:  Loads a module from an archive
// Returns:      MIDAS module handle for the module or NULL if failed
#pragma argsused
MIDASmodule LoadArchive(char *arcName, char *fileName)
{
    int         i;
    char        ext[_MAX_EXT];
    char        file[_MAX_FNAME];
    char        *decompress;
    char        commandLine[_MAX_PATH + 128];
    STARTUPINFO startInfo;
    PROCESS_INFORMATION procInfo;
    DWORD       exitCode;
    WIN32_FIND_DATA findData;
    HANDLE      findHandle;
    DWORD       creationFlags;
    SECURITY_ATTRIBUTES secAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE};
    int         fileFound;
    MIDASmodule module;

    AddTextLine("Decompressing module");
    SendMessage(mainWinHandle, WM_SETTEXT, 0,
        (LPARAM) "MidpNT - Decompressing...");

    GetCurrentDirectory(MAX_PATH+127, commandLine);
    if ( SetCurrentDirectory(unzipDir) )
    {
        SetCurrentDirectory(commandLine);
        sprintf(commandLine, "Directory \"%s\" already exists", unzipDir);
        return DecompErr( commandLine );
    }

    _splitpath(arcName, NULL, NULL, file, ext);

    decompress = NULL;
    for ( i = 0; i < numArchives; i++ )
    {
        if ( !stricmp(ext, archives[i].ext) )
            decompress = archives[i].cmd;
    }

    if ( decompress == NULL ) Panic("Decompress: Unknown extension!");

    sprintf(commandLine, decompress, arcName, fileName, unzipDir);
    AddTextLine(commandLine);

    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.lpReserved = NULL;
    startInfo.lpDesktop = NULL;
    startInfo.lpTitle = commandLine;
    startInfo.dwFlags = STARTF_USESHOWWINDOW;
    startInfo.wShowWindow = (unzipMinimized) ? SW_HIDE : SW_SHOWDEFAULT;
    startInfo.cbReserved2 = 0;
    startInfo.lpReserved2 = NULL;

    creationFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;

    if ( !CreateProcess(NULL, commandLine, NULL, NULL, FALSE,
        creationFlags, NULL, NULL,
        &startInfo, &procInfo) )
    {
        sprintf(commandLine, "Decompress: CreateProcess failed: %i",
            GetLastError());
        Panic(commandLine);
    }

    exitCode = STILL_ACTIVE;
    while ( exitCode == STILL_ACTIVE )
    {
        Sleep(250);
        if ( !GetExitCodeProcess(procInfo.hProcess, &exitCode) )
        {
            sprintf(commandLine, "Decompress: GetExitCodeProcess failed: %i",
                GetLastError());
            Panic(commandLine);
        }
    }

    if ( exitCode != 0 )
    {
        sprintf(commandLine, "Decompressor failed: %i",
            exitCode);
        return DecompErr( commandLine );
    }

    AddTextLine("Loading Module");
    SendMessage(mainWinHandle, WM_SETTEXT, 0, (LPARAM) "MidpNT - Loading...");

    sprintf(commandLine, fileName[0] ? "%s\\%s" : "%s\\*.*",
                         unzipDir, fileName);

    if ( (findHandle = FindFirstFile(commandLine, &findData))
          == INVALID_HANDLE_VALUE )
        return DecompErr( "Decompressed file not found");

    fileFound = 0;
    do
    {
        if ( !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
            sprintf(commandLine, "%s\\%s", unzipDir, findData.cFileName);

            if ( !fileFound )
            if ( (module = MIDASloadModule(commandLine)) != NULL )
            {
                fileFound = 1;
            }

            if ( !DeleteFile(commandLine) )
                return DecompErr("Couldn't delete file");
        }
    } while ( FindNextFile(findHandle, &findData) );

    FindClose(findHandle);

    RemoveDirectory(unzipDir);

    if ( fileFound )
        return module;
    else
    return DecompErr("No valid module files found");
}


char *FileNameFilters =
    "All Modules\0*.arj;*.mod;*.s3m;*.xm;*.mdz;*.rar;*.s3z;*.xmz;*.zip\0"
    "All Files\0*.*\0"
    "PlayList Files\0*.pls\0"
    "Unpacked Modules\0*.mod;*.s3m;*.xm\0"
    "Packed Modules\0*.arj;*.mdz;*.rar;*.s3z;*.xmz;*.zip\0"
    "\0\0";

extern char            defaultDir[MAX_PATH];


static OPENFILENAME *ofn = NULL;
char *GetFileName(char *windowTitle, int *to_path)
{
    DWORD       error;

    if ( ofn == NULL )
    {
        if ( (ofn = (OPENFILENAME*) malloc(sizeof(OPENFILENAME))) == NULL )
            return NULL;

        ofn->lStructSize = sizeof(OPENFILENAME);
        ofn->hwndOwner = 0;
        ofn->hInstance = 0;
        ofn->lpstrFilter = (LPCSTR) FileNameFilters;
        ofn->lpstrCustomFilter = NULL;
        ofn->nMaxCustFilter = 0;
        ofn->nFilterIndex = 1;
        if ( (ofn->lpstrFile = (char*) malloc(1024)) == NULL )
        {
            ofn = NULL;
            return NULL;
        }
        memset(ofn->lpstrFile, 0, 1024);
        ofn->nMaxFile = 1023;
        ofn->lpstrFileTitle = NULL;
        ofn->nMaxFileTitle = 0;
        ofn->lpstrInitialDir = defaultDir;  //!
        ofn->lpstrTitle = windowTitle;
        ofn->nFileOffset = 0;
        ofn->nFileExtension = 0;
        ofn->lpstrDefExt = 0;
        ofn->Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    }
    else
        ofn->lpstrInitialDir = NULL;

    ofn->lpstrTitle = windowTitle;

    if ( GetOpenFileName(ofn) == FALSE )
    {
        error = CommDlgExtendedError();
        return NULL;
    }

    if (to_path != NULL) *to_path = ofn->nFileOffset;

    return ofn->lpstrFile;
}

extern HANDLE staticArc;
//---------------------------- ARJ support ---------------------------------
typedef short int uint; // must be unsigned also, but NOT 32-bit
typedef unsigned long ulong;
typedef unsigned char uchar;

struct
{
  uchar  Method;
  uint   Filler1;
  ulong  Date_Time, Size_c, Size_o, Crc32;
  ulong  Filler2;
  uint   Filler3;
} ARJ;

// fills an existing listbox 'wnd' with a contents of .ARJ archive
void fill_ARJ( HANDLE wnd, LPCSTR arcName )
{
   HFILE in;
   long filesize, SavePos;
   int Done;
   uchar I, Index, FileName[128+1];
   int sign; // 32-bit, but 16 enough

   SendMessage( wnd, LB_RESETCONTENT, 0, 0);  // clear the list
   SendMessage( staticArc, WM_SETTEXT, 0, (LPARAM) arcName);
   if (( in=_lopen( arcName, OF_READ | OF_SHARE_DENY_NONE ))==HFILE_ERROR )
      return;

   strcpy( cArcname, arcName );
   isArcname=1;

   filesize=_llseek( in, 0, 2 );
   _llseek( in, 2, 0 );  // skip the header

   Done = 0;

   sign = 0;
   while (!Done && _llseek( in, 0, 1 ) < filesize )
   {
     SavePos = _llseek( in, 0, 1 );
     sign >>= 8;
     _lread( in, ((char *)(&sign))+1, 1 );

     if (sign == 0xEA60)
     {
       _lread( in, &Index, 1 );
       if (Index != 0)
       {
	 _llseek(in, SavePos + 8, 0);
	 _lread( in, &ARJ, sizeof(ARJ));
	 I = 0;
	 do
	 {
	   _lread( in, &FileName[I++], 1);
	 } while ( FileName[I-1] );
         SendMessage( wnd, LB_ADDSTRING, 0, (LONG)(LPSTR) FileName );

         _llseek( in, SavePos + ARJ.Size_c + Index + 9, 0);
       }
       else Done = 1;
     };
  };
  _lclose(in);
}

//---------------------------- ZIP support ---------------------------------

struct
{
   uint    Version_Made_By;      // System id/program vers.
   uint    Version;              // Vers. needed to extract
   uint    BitFlag;              // General flags
   uint    Compression_Method;   // Compression type used
   uint    Time;                 // File creation time
   uint    Date;                 // File creation date
   ulong   Crc32;                // 32-bit CRC of file
   ulong   Size_c;               // Compressed size of file
   ulong   Size_o;               // Original size of file
   uint    FileName_Length;      // Length of file name
   uint    ExtraField_Length;    // Length of extra stuff
   uint    CommentField_Length;  // Length of comments
   uint    Diskstart_Number;     // Disk # file starts on
   uint    Internal_Attributes;  // Text/non-text flags
   ulong   External_Attributes;  // File system attributes
   ulong   LocalHeader_Offset;   // Where local hdr starts
} Zip_Central_Header;

struct
{
    uint  Version;                   // Vers. needed to extract
    uint  BitFlag;                   // General flags
    uint  Compression_Method;        // Compression type used
    uint  Time;                      // File creation time
    uint  Date;                      // File creation date
    ulong Crc32;                     // 32-bit CRC of file
    ulong Size_c;                    // Compressed size of file
    ulong Size_o;                    // Original size of file
    uint  FileName_Length;           // Length of file name
    uint  ExtraField_Length;         // Length of extra stuff
} Zip_Local_Header;

ulong
  Zip_Central_Header_Signature  = 0x02014B50,
  Zip_Local_Header_Signature    = 0x04034B50,
  Zip_End_Central_Dir_Signature = 0x06054B50;

// fills an existing listbox 'wnd' with a contents of .ZIP archive
void fill_ZIP( HANDLE wnd, LPCSTR arcName )
{
   HFILE in;
   long filesize, SavePos;
   int Done;
   uchar FileName[128+1];
   ulong sign;

   SendMessage( wnd, LB_RESETCONTENT, 0, 0);  // clear the list
   SendMessage( staticArc, WM_SETTEXT, 0, (LPARAM) arcName);
   if (( in=_lopen( arcName, OF_READ | OF_SHARE_DENY_NONE ))==HFILE_ERROR )
      return;

   strcpy( cArcname, arcName );
   isArcname=1;

   filesize=_llseek( in, 0, 2 );
   _llseek( in, 0, 0 );  // goto at header

   Done = 0;

   sign = 0;
   while (!Done && (_llseek( in, 0, 1 ) < filesize) )
   {
     sign >>= 8;
     _lread( in, ((char *)(&sign))+3, 1 );

    if ((sign & 0xFFFF) == 0x4B50)
    switch ( sign >> 16)
    {
    case 0x0403: // Zip_Local_Header_Signature
        _lread( in, &Zip_Local_Header, sizeof(Zip_Local_Header));
        _llseek(in, Zip_Local_Header.Size_c +
                    Zip_Local_Header.FileName_Length, 1);
	break;
    case 0x0201: // Zip_Central_Header_Signature
        _lread( in, &Zip_Central_Header, sizeof(Zip_Central_Header));
        _lread( in, FileName, Zip_Central_Header.FileName_Length);
        FileName[ Zip_Central_Header.FileName_Length ]=0;
        _llseek(in, Zip_Central_Header.CommentField_Length +
                      Zip_Central_Header.ExtraField_Length, 1); // +current
        SendMessage( wnd, LB_ADDSTRING, 0, (LONG)(LPSTR) FileName );
	break;
    case 0x0605: // Zip_End_Central_Dir_Signature
	Done = 1; // Skip dir information
    };
  };
  _lclose(in);
}

//---------------------------- RAR support ---------------------------------

 struct
{
    uint  HEAD_CRC;    //   2 bytes     CRC of total block or block part
    uchar HEAD_TYPE;   //   1 byte      Block type
    uint  HEAD_FLAGS;  //   2 bytes     Block flags
    uint  HEAD_SIZE;   //   2 bytes     Block size
  //  ulong ADD_SIZE;       4 bytes     Optional field - added block size
} RARblk;

struct
{
        ulong UNP_SIZE; //      Uncompressed file size
        uchar HOST_OS;  //      Operating system used for archiving
        ulong FILE_CRC; //      File CRC
        ulong FTIME;    //      Date and time in standard MS DOS format
        uchar UNP_VER;  //      RAR version needed to extract file
        uchar METHOD;   //      Packing method
        uint  NAME_SIZE;//      File name size
        ulong ATTR;     //      File attributes
} RARfile;
// followed by FILE_NAME       File name - string of NAME_SIZE bytes size

// fills an existing listbox 'wnd' with a contents of .RAR archive
void fill_RAR( HANDLE wnd, LPCSTR arcName )
{
   HFILE in;
   long filesize, addsize;
   int Done;
   uchar FileName[128+1];
   ulong sign;

   SendMessage( wnd, LB_RESETCONTENT, 0, 0);  // clear the list
   SendMessage( staticArc, WM_SETTEXT, 0, (LPARAM) arcName);
   if (( in=_lopen( arcName, OF_READ | OF_SHARE_DENY_NONE ))==HFILE_ERROR )
      return;

   strcpy( cArcname, arcName );
   isArcname=1;

   filesize=_llseek( in, 0, 2 );
   _llseek( in, 0, 0 );  // seek the header

   Done = 0;

   sign = 0;
   while (!Done && _llseek( in, 0, 1 ) < filesize )
   {
        addsize=0;
        _lread( in, &RARblk, sizeof(RARblk) );
        if ( RARblk.HEAD_FLAGS & 0x8000 )
        {
            _lread( in, &addsize, sizeof(addsize) );
            addsize -= 4;
        }
        switch ( RARblk.HEAD_TYPE )
        {
            case 0x72: break;  // marker block
            case 0x73: break;  // archive header
            case 0x74:         // file header
                 _lread( in, &RARfile, sizeof(RARfile));
                 addsize -= sizeof(RARfile);
                 _lread( in, &FileName, RARfile.NAME_SIZE );
                 addsize -= RARfile.NAME_SIZE;
                 FileName[ RARfile.NAME_SIZE ]= 0;
                 if (( RARblk.HEAD_FLAGS & 0xE0 ) != 0xE0 )
                 SendMessage( wnd, LB_ADDSTRING, 0, (LONG)(LPSTR) FileName );
                 break;  // file header
            case 0x75: break;  // comment header
            case 0x76: break;  // extra information
            case 0x77: break;  // subblock
            case 0x78: break;  // recovery record
        }
        _llseek( in, RARblk.HEAD_SIZE+addsize-sizeof(RARblk), 1 );
  };
  _lclose(in);
}
