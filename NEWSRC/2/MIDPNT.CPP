// v 1.0
//       MidpNT.cpp
// MIDAS Module Player for Windows NT main program file ver 0.2.4
// $Id: MidpNT.cpp 1.15 1997/01/25 13:17:44 pekangas Exp $
// Copyright 1996 Petteri Kangaslampi

#include <windows.h>
#include <ddeml.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "midpres.h"
#include "midasdll.h"
#include "MidpNT.h"
#include "MidpList.h"
#include "MidpMode.h"
#include "MidpView.h"

#include "midproto.h"
#include "SongInfo.h"
#include "InstList.h"
#include "ViewList.h"
#include "Archiver.h"
#include "Registry.h"
#include "midpecho.h"
#include "echoedit.h"

#include "toolbar.h"

#define POLLRATE 100

#define NUMLINES 100
#define MAXLINE 255

#define MIDP_ADDTEXT WM_USER+100

HANDLE          instance;      // program instance
HANDLE          accelHandle;   // accelerator

static char     *mainWinClass = "midpNTClass";  /* main window class name */

int             oversample = 0;

static char *copyRight=
       "MIDAS Module Player for Windows NT\n"
       "Copyright (file interface) 1997 Alexander Milukov\n"
       "Portions Copyright (MIDAS .dll) 1996,1997 Petteri Kangaslampi\n"
       "Version " FULLVERSTR ", built " __DATE__ ".\n";

static char *copyRight2=
       "Copyright 1996,1997 Housemarque Inc.\n";

UINT	hTimer=NULL;
HWND  mainWinHandle,
      editWinHandle,
      tbhwnd, // toolbar
      listFrom, listToPlay, listToPlayPath, listDrive, listDir,
      btnADD, btnPLAY, btnCLEAR,
      staticPos, staticArc, hwndTrack;
int     isArcname=0;

char    cArcname[80];     // name of archive containing .s3m .xm etc.
char    curDrive='c',
        curDir[512]="\0"; // default directory for file selection

int             editWidth, editHeight;

static char     *textLines[NUMLINES];   /* all text lines */
static char     *text;
static int      numLines;
static char     loadFileName[MAX_PATH + 16];    /* fudge factor */


MIDASmodule     module = NULL;
MIDASmodulePlayHandle playHandle = 0;
MIDASplayStatus MPstatus, MPstatusO;
MIDASmoduleInfo modInfo;

int PlayIndex=0;

char            defaultDir[MAX_PATH];
char            unzipDir[MAX_PATH];
char            defaultPList[MAX_PATH];

LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam);

void StartDDEServer(void);
void PlayFile(char *fileName);
int PlayDDE(char *fileName);
void ReadRegistry(void);
void WriteRegistry(void);
void RestoreMainWindow(void);
void SaveViews(void);
void RestoreViews(void);



DWORD           ddeInstance;
int             ddeInit = 0;



SongInfoView    *songInfoView = NULL;
InstListView    *instListView = NULL;
EchoEditView    *echoEditView = NULL;

midpList        midpModelessList;
midpViewWindowList viewWindowList;
midpViewList    viewList;

char            *iniFile = "MidpNT.ini";
char            *defaultDirectory = "";

char            *baseKey = "Software\\Sahara Surfers\\MidpNT";
char            *midasKey = "Software\\Sahara Surfers\\MidpNT\\MIDAS Setup";

static int      mainWinX, mainWinY, mainWinWidth, mainWinHeight;

int             viewsChildren = 1;
int             unzipMinimized = 0;
int             minimalUI = 0;
unsigned        filterMode = 0;

unsigned        defaultStereoSeparation = 0;

midpEchoEffectList echoEffectList;


void PrintEffects()
{
    midpEchoEffect *effect;
    midpEcho *echo;
    unsigned n;

    effect = echoEffectList.FirstEffect();
    while ( effect != NULL )
    {
        printf("Effect: %s:\n", effect->GetName());
        printf("Feedback %f, %u echoes\n", effect->GetFeedback(),
               effect->GetNumEchoes());
        for ( n = 0; n < effect->GetNumEchoes(); n++ )
        {
            echo = effect->GetEcho(n);
            printf("    Echo %u: Delay %f, Gain %f, Reverse %i\n",
                   n, echo->GetDelay(), echo->GetGain(),
                   echo->GetReverse());
        }
        effect = echoEffectList.NextEffect();
    }
}

void EchoEffectTest()
{
    midpEchoEffect *effect;
    midpEcho *echo;

    PrintEffects();

    effect = echoEffectList.NewEffect();
    effect->Rename("Superkaiku");
    effect->SetNumEchoes(1);
    effect->SetFeedback(0.5);
    echo=effect->GetEcho(0);
    echo->SetDelay(250);
    echo->SetGain(0.4);
    echo->SetReverse(0);
    effect->SaveToRegistry();

    PrintEffects();
}


// Function:     int WinMain(HANDLE thisInstance, HANDLE prevInstance, LPSTR
//                   commandLine, int showCommand);
//
// Description:  Windows main function - common initialization and message loop

#pragma argsused
int WINAPI WinMain(HANDLE thisInstance, HANDLE prevInstance,
    LPSTR commandLine, int showCommand)
{
    MSG         msg;
    WNDCLASS    wc;
    midpModeless *modeless;
    char        *c, *d, *end;
    int         inQuotes = 0;
    int         hadCharacters = 0;

    StartupMIDAS();

    if ( !MIDASreadConfigRegistry((DWORD) HKEY_CURRENT_USER, midasKey) )
    {
        if ( MIDASconfig() )
            MIDASwriteConfigRegistry((DWORD) HKEY_CURRENT_USER, midasKey);
    }

    loadFileName[0] = 0;
    if ( strlen(commandLine) > 0 )
    {
        c = commandLine;

        while ( (*c != 0) && (isspace(*c)) ) c++;

        // switches
        if ( *c == '-' )
        {
            c++;
            switch ( *(c++) )
            {
                case 'm':
                    minimalUI = 1;
                    break;
            }

            while ( (*c != 0) && (!isspace(*c)) ) c++;
        }

        d = loadFileName;
        end = d;

        while ( *c != 0 )
        {
            if ( *c == '\"' )
            {
                inQuotes ^= 1;
                hadCharacters = 1;
            }
            else
            {
                if ( (*c == ' ') && (!hadCharacters) )
                {
                    c++; continue;
                }
                *(d++) = *c;
                if ( inQuotes || (*c != ' ') ) end = d;
                hadCharacters = 1;
            }
            c++;
        }
        *end = 0;
    }

    instance = thisInstance;

    if ( PlayDDE(loadFileName) ) return 0;

    /* This is the first instance - now set up a DDE server: */
    StartDDEServer();

    /* Let's be a Very Important Process: */
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    ReadRegistry();

    songInfoView = new SongInfoView;
    viewList.AddView(songInfoView);
    instListView = new InstListView;
    viewList.AddView(instListView);
    echoEditView = new EchoEditView;
    viewList.AddView(echoEditView);

    /* Set up and register window class for the main window: */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = mainWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, "MIDP_ICON");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    //wc.hbrBackground = NULL;
    wc.lpszMenuName = "MAINMENU";
    wc.lpszClassName = mainWinClass;
    if ( RegisterClass(&wc) == 0 ) return FALSE;

    if ( !minimalUI )
        RestoreMainWindow();
    else
    {
        mainWinX = mainWinY = 100;
        mainWinWidth = 320;
        mainWinHeight = 0;
    }

    /* Create main window: */

    mainWinHandle = CreateWindow(
        mainWinClass,                           /* class */
        "MIDAS Module Player for Windows NT",   /* caption */
        WS_OVERLAPPEDWINDOW,                    /* style */
        mainWinX,                               /* init. x pos */
        mainWinY,                               /* init. y pos */
        mainWinWidth,                           /* init. x size */
        mainWinHeight,                          /* init. y size */
        NULL,                                   /* parent window */
        NULL,                                   /* menu handle */
        instance,                               /* program handle */
        NULL                                    /* create parms */
    );

    if ( !mainWinHandle ) return FALSE;
    hTimer = SetTimer( mainWinHandle, ID_TIMERI, 500, NULL);

    if ( (accelHandle = LoadAccelerators(instance, "MAINACCELERATORS")) ==
        NULL )
    {
        MessageBox(NULL, "Coulnd't load accelerators", "MidpNT Initialization error",
            MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    /* Display main window: */
    ShowWindow(mainWinHandle, showCommand);
    UpdateWindow(mainWinHandle);

    if ( !minimalUI )
        RestoreViews();

    tbhwnd = CreateToolbarEx( mainWinHandle,
    WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS, TBAR_ID,
    16, // count of Bitmaps
    instance, TBAR_BM, // ãäå ëåæàò .BMP
    tbBtn, 16, // array and size
    24,24,24,24, // button and bitmap size
    sizeof( TBBUTTON ));

    SendMessage( tbhwnd, TB_AUTOSIZE, 0, 0);

    /* Da Message Loop: */
    while ( GetMessage(&msg, NULL, NULL, NULL) )
    {
        if ( !TranslateAccelerator(mainWinHandle, accelHandle, &msg) )
        {
            modeless = (midpModeless*) midpModelessList.GetFirst();
            while ( modeless != NULL )
            {
                if ( IsDialogMessage(modeless-> hwnd, &msg) )
                    break;
                modeless = (midpModeless*) midpModelessList.GetNext();
            }

            if ( modeless == NULL )
            {
                TranslateMessage (&msg) ;
                DispatchMessage (&msg) ;
            }
        }
    }

    WriteRegistry();

    if ( ddeInit ) DdeUninitialize(ddeInstance);
    if ( hTimer ) KillTimer( mainWinHandle, ID_TIMERI );

    return(msg.wParam);
}


void RestoreViews(void)
{
    int         i, n;
    Registry    reg;
    char        s[256];
    char        v[64];
    midpViewWindow *win;
    midpView    *view;

    reg.OpenKey(baseKey);
    n = reg.ValueDWORD("NumViewWindows", 0);

    for ( i = 0; i < n; i++ )
    {
        sprintf(s, "%s\\ViewWindow%i", baseKey, i);

        if ( reg.KeyExists(s) )
        {
            reg.OpenKey(s);
            reg.ValueString("ViewClass", "none", v, 63);

            view = viewList.FindView(v);
            win = view->CreateViewWindow(&reg);
        }
    }
}

void SaveViews(void)
{
    int         i;
    Registry    reg;
    char        s[256];
    midpViewWindow *win;

    reg.OpenKey(baseKey);
    reg.WriteDWORD("NumViewWindows", viewWindowList.NumWindows());

    for ( i = 0, win = viewWindowList.GetFirst(); win != NULL; i++,
        win = viewWindowList.GetNext() )
    {
        sprintf(s, "%s\\ViewWindow%i", baseKey, i);

        if ( !reg.KeyExists(s) )
            reg.CreateKey(s);
        reg.OpenKey(s);

        win->SaveState(&reg);
    }
}

void ReadRegistry(void)
{
    Registry    reg;

    if ( !reg.KeyExists(baseKey) )
        reg.CreateKey(baseKey);
    else
        reg.OpenKey(baseKey);

    reg.ValueString("DefaultDirectory", "c:\\", defaultDir, MAX_PATH);
    reg.ValueString("UnzipDirectory", "c:\\$MidpNT$", unzipDir, MAX_PATH);
    reg.ValueString("DefaultPlayList", "c:\\default.pls", defaultPList, MAX_PATH);
    viewsChildren = reg.ValueDWORD("OpenViewsAsChildWindows", 1);
    unzipMinimized = reg.ValueDWORD("RunUnZipAsMinimizedWindow", 0);
    defaultStereoSeparation = reg.ValueDWORD("DefaultStereoSeparation", 32);
    MIDASsetOption(MIDAS_OPTION_DEFAULT_STEREO_SEPARATION,
                   defaultStereoSeparation);
}


void WriteRegistry(void)
{
    Registry    reg;

    reg.OpenKey("Software\\Sahara Surfers\\MidpNT");
    reg.WriteString("DefaultDirectory", defaultDir);
    reg.WriteString("UnzipDirectory", unzipDir);
    reg.WriteString("DefaultPlayList", defaultPList);
    reg.WriteDWORD("RunUnZipAsMinimizedWindow", unzipMinimized);
    reg.WriteDWORD("OpenViewsAsChildWindows", viewsChildren);
    reg.WriteDWORD("DefaultStereoSeparation", defaultStereoSeparation);
}

void ShowTextBottom(void)
{
    int         firstVisible;
    int         lines;
    int         newFirst;
    int         textAreaHeight;
    int         visibleLines;
    HFONT       oldFont, font;
    HDC         hdc;
    TEXTMETRIC  textMetric;

    font = (HFONT) SendMessage(editWinHandle, WM_GETFONT, 0, 0);
    firstVisible = (int) SendMessage(editWinHandle, EM_GETFIRSTVISIBLELINE, 0, 0);
    lines = (int) SendMessage(editWinHandle, EM_GETLINECOUNT, 0, 0);
    textAreaHeight = editHeight - GetSystemMetrics(SM_CYHSCROLL);

    hdc = GetDC(editWinHandle);
    oldFont = SelectObject(hdc, font);
    if ( GetTextMetrics(hdc, &textMetric) == FALSE )
        return; /* FIXME */
    SelectObject(hdc, oldFont);

    if ( textMetric.tmHeight > 0 )
        visibleLines = textAreaHeight / textMetric.tmHeight;
    else
        visibleLines = 1;

    newFirst = lines - visibleLines - 1;
    if ( newFirst < 0 )
        newFirst = 0;
    if ( newFirst > (lines-1) )
        newFirst = lines-1;

    SendMessage(editWinHandle, EM_LINESCROLL, 0, newFirst - firstVisible);
}

void BuildText(void)
{
    int         i;

    text[0] = 0;
    for ( i = 0; i < numLines; i++ )
    {
        strcat(text, textLines[i]);
        strcat(text, "\r\n");
    }
}


// adds the ASCIIZ line of text into array of pointers
// updated by Mil for '\n' handling: now I can add some lines at once
void AddTextLine(char *textLine)
{
    int         i;
    char        *temp;

  xnew:
    if ( numLines == NUMLINES )
    {
        temp = textLines[0];
        for ( i = 0; i < (NUMLINES - 1); i++ )
            textLines[i] = textLines[i+1];
        textLines[NUMLINES-1] = temp;
        numLines--;
    }

    temp = textLines[numLines];
    i=0;
    while (i<MAXLINE-1)  // carefull
    {
        *temp++ = *textLine++;
        if ( !temp[-1] ) break;  // '\0' found

        if ( temp[-1]=='\n' )
        {
           temp[-1]='\0'; numLines++; goto xnew;
        }
    }
    numLines++;

    BuildText();

    SendMessage(editWinHandle, WM_SETREDRAW, (WPARAM) FALSE, 0);
    SendMessage(editWinHandle, WM_SETTEXT, 0, (LPARAM) text);
    ShowTextBottom();
    SendMessage(editWinHandle, WM_SETREDRAW, (WPARAM) TRUE, 0);
    InvalidateRect(editWinHandle, NULL, FALSE);
    UpdateWindow(editWinHandle);
}

void InitText(void)
{
    int         i;

    text = (char*) malloc(NUMLINES * (MAXLINE+1));
    text[0] = 0;

    for ( i = 0; i < NUMLINES; i++ )
    {
        textLines[i] = (char*) malloc(MAXLINE+1);
        textLines[i][0] = 0;
    }
}



#define NUMMIXRATES 8
int             mixRates[NUMMIXRATES] =
        { 8000, 11025, 16000, 22050, 28000, 32000, 38000, 44100 };

BOOL CALLBACK SoundOptionsDialogProc(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam)
{
    char        strBuffer[32];
    int         i;

    lparam = lparam;                    // get rid of a stupid warning

    switch ( message )
    {
        case WM_INITDIALOG:
            for ( i = 0; i < NUMMIXRATES; i++ )
            {
                itoa(mixRates[i], strBuffer, 10);
                SendDlgItemMessage(hwnd, SOUNDOPTIONS_MIXRATE, CB_ADDSTRING,
                    0, (LPARAM) strBuffer);
            }
            SendDlgItemMessage(hwnd, SOUNDOPTIONS_STEREO, BM_SETCHECK,
                (WPARAM) stereo, 0);
            SendDlgItemMessage(hwnd, SOUNDOPTIONS_8BIT, BM_SETCHECK,
                (WPARAM) force8bit, 0);

            for ( i = 0; i < NUMMIXRATES; i++ )
            {
                if ( mixRates[i] == mixRate )
                {
                    SendDlgItemMessage(hwnd, SOUNDOPTIONS_MIXRATE,
                        CB_SETCURSEL, i, 0);
                    break;
                }
            }

            itoa(500, strBuffer, 10);
            SendDlgItemMessage(hwnd, SOUNDOPTIONS_BUFFERSIZE, WM_SETTEXT, 0,
                (LPARAM) strBuffer);

            itoa(16, strBuffer, 10);
            SendDlgItemMessage(hwnd, SOUNDOPTIONS_BUFFERBLOCKS, WM_SETTEXT, 0,
                (LPARAM) strBuffer);

            return TRUE;

        case WM_COMMAND:
            switch ( LOWORD(wparam) )
            {
                case IDOK:
                    SendDlgItemMessage(hwnd, SOUNDOPTIONS_MIXRATE,
                        WM_GETTEXT, 31, (LPARAM) strBuffer);
                    if ( atoi(strBuffer) != 0 )
                        mixRate = atoi(strBuffer);
                    stereo = SendDlgItemMessage(hwnd, SOUNDOPTIONS_STEREO,
                        BM_GETCHECK, 0, 0);
                    force8bit = SendDlgItemMessage(hwnd,
                        SOUNDOPTIONS_8BIT, BM_GETCHECK, 0, 0);

                    MIDASsetOption(MIDAS_OPTION_MIXRATE, mixRate);

                    if ( stereo )
                    {
                        if ( force8bit )
                            MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,
                                MIDAS_MODE_8BIT_STEREO);
                        else
                            MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,
                                MIDAS_MODE_16BIT_STEREO);
                    }
                    else
                    {
                        if ( force8bit )
                            MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,
                                MIDAS_MODE_8BIT_MONO);
                        else
                            MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,
                                MIDAS_MODE_16BIT_MONO);
                    }

                    SendDlgItemMessage(hwnd, SOUNDOPTIONS_BUFFERSIZE,
                        WM_GETTEXT, 31, (LPARAM) strBuffer);
                    if ( atoi(strBuffer) != 0 )
                        MIDASsetOption(MIDAS_OPTION_MIXBUFLEN,
                            atoi(strBuffer));

                    SendDlgItemMessage(hwnd, SOUNDOPTIONS_BUFFERBLOCKS,
                        WM_GETTEXT, 31, (LPARAM) strBuffer);
                    if ( atoi(strBuffer) != 0 )
                        MIDASsetOption(MIDAS_OPTION_MIXBUFBLOCKS,
                            atoi(strBuffer));

                    EndDialog(hwnd, 0);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
    }

    return FALSE;
}



void SoundOptions(void)
{
    DialogBox(instance, "SOUNDOPTIONS", mainWinHandle, (DLGPROC)
        &SoundOptionsDialogProc);
}

#pragma argsused
BOOL CALLBACK PreferencesDialogProc(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam)
{
    static char str[20];
    switch ( message )
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, PREFERENCES_DEFAULTDIR, WM_SETTEXT,
                0, (LPARAM) defaultDir);
            SendDlgItemMessage(hwnd, PREFERENCES_UNZIPDIR, WM_SETTEXT,
                0, (LPARAM) unzipDir);
            SendDlgItemMessage(hwnd, PREFERENCES_DEFPLIST, WM_SETTEXT,
                0, (LPARAM) defaultPList);
            SendDlgItemMessage(hwnd, PREFERENCES_VIEWSCHILDREN, BM_SETCHECK,
                (WPARAM) viewsChildren, 0);
            SendDlgItemMessage(hwnd, PREFERENCES_UNZIPMINI, BM_SETCHECK,
                (WPARAM) unzipMinimized, 0);
            itoa(defaultStereoSeparation, str, 10);
            SendDlgItemMessage(hwnd, PREFERENCES_DEFAULTSTEREOSEPARATION,
                               WM_SETTEXT, 0, (LPARAM) str);
            return TRUE;

        case WM_COMMAND:
            switch ( LOWORD(wparam) )
            {
                case IDOK:
                    SendDlgItemMessage(hwnd, PREFERENCES_DEFAULTDIR,
                        WM_GETTEXT, MAX_PATH - 1, (LPARAM) defaultDir);
                    SendDlgItemMessage(hwnd, PREFERENCES_UNZIPDIR,
                        WM_GETTEXT, MAX_PATH - 1, (LPARAM) unzipDir);
                    SendDlgItemMessage(hwnd, PREFERENCES_DEFPLIST,
                        WM_GETTEXT, MAX_PATH - 1, (LPARAM) defaultPList);
                    viewsChildren = SendDlgItemMessage(hwnd,
                        PREFERENCES_VIEWSCHILDREN, BM_GETCHECK, 0, 0);
                    unzipMinimized = SendDlgItemMessage(hwnd,
                        PREFERENCES_UNZIPMINI, BM_GETCHECK, 0, 0);
                    SendDlgItemMessage(hwnd,
                                       PREFERENCES_DEFAULTSTEREOSEPARATION,
                                       WM_GETTEXT, 31, (LPARAM) str);
                    if ( atoi(str) < 0 )
                        defaultStereoSeparation = 0;
                    else
                        defaultStereoSeparation = atoi(str);
                    if ( defaultStereoSeparation > 64 )
                        defaultStereoSeparation = 64;
                    MIDASsetOption(MIDAS_OPTION_DEFAULT_STEREO_SEPARATION,
                                   defaultStereoSeparation);
                    EndDialog(hwnd, 0);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
    }

    return FALSE;
}



// if we know full pathname of archive and the modulename in it, then
// fileName will be like a "d:\music\xmARCHIVE:best.rar\dj_best.s3m"
// else assume that archive only contain the same-named module
// example:                "d:\music\xm\best.rar"
void PlayFile(char *fileName)
{
    char *arc;
    if ( module != NULL )
    {
        StopFreeModule(playHandle, module); module = NULL; playHandle = 0;
    }

    if (( arc=strstr( fileName, "ARCHIVE:" )) !=NULL )
    {
       *arc++ = '\\';
       strcpy( arc, arc+7 ); // cut out the "ARCHIVE:"
       arc = strchr( arc, '\\' );
       *arc++ = '\0';
       module = LoadArchive( fileName, arc ); // archive and its module
       if ( module != NULL )
            PlayModule(module);
       viewWindowList.SongChanged();
       return;
    }

    if ( IsArchive(fileName) )
    {
        module = LoadArchive(fileName, "" );

        if ( module != NULL )
            PlayModule(module);
    }
    else
    {
        module = LoadModule(fileName);

        if ( module != NULL )
            PlayModule(module);
    }

    viewWindowList.SongChanged();
}


void RestoreMainWindow(void)
{
    Registry    reg;

    reg.OpenKey(baseKey);

    if ( reg.ValueExists("MainWindowX") )
    {
        mainWinX = reg.ValueDWORD("MainWindowX", 100);
        mainWinY = reg.ValueDWORD("MainWindowY", 100);
        mainWinWidth = reg.ValueDWORD("MainWindowWidth", 200);
        mainWinHeight = reg.ValueDWORD("MainWindowHeight", 200);
    }
    else
        mainWinX = mainWinY = mainWinWidth = mainWinHeight = CW_USEDEFAULT;
}


void SaveMainWindow(void)
{
    Registry    reg;
    RECT        rect;

    reg.OpenKey(baseKey);

    GetWindowRect(mainWinHandle, &rect);
    reg.WriteDWORD("MainWindowX", rect.left);
    reg.WriteDWORD("MainWindowY", rect.top);
    reg.WriteDWORD("MainWindowWidth", rect.right - rect.left);
    reg.WriteDWORD("MainWindowHeight", rect.bottom - rect.top);
}



void ListInstruments(void)
{
    char        str[256];
    unsigned    i;
    MIDASmoduleInfo moduleInfo;
    MIDASinstrumentInfo instrumentInfo;

    if ( module != NULL )
    {
        MIDASgetModuleInfo(module, &moduleInfo);

        for ( i = 0; i < moduleInfo.numInstruments; i++ )
        {
            MIDASgetInstrumentInfo(module, i, &instrumentInfo);
            sprintf(str, "%02X: %s", i + 1, instrumentInfo.instName);
            AddTextLine(str);
        }
    }
}

// adds a next item to visible list
void PlayAdd( char *name)
{
   SendMessage( listToPlay, LB_ADDSTRING, 0, (LONG)(LPSTR) name );
   // MessageBox( NULL, name, "Add name", 0 );
}

// adds a next item (full path) to invisible list
void PlayAddP( char *path )
{
   SendMessage( listToPlayPath, LB_ADDSTRING, 0, (LONG)(LPSTR) path );
   // MessageBox( NULL, path, "Add path", 0 );
}

typedef struct
{
   HFILE handle;
   long  fsize;
   long  fpos;
   char  buf[512];
   int   cbuf;
   int   p;
} F_ILE;

int f_open( F_ILE *f, char *name, int mode )
{
   if (( f->handle=_lopen( name, mode | OF_SHARE_DENY_NONE ))==HFILE_ERROR )
      return 0;

   f->fsize=_llseek( f->handle, 0, 2 );
            _llseek( f->handle, 0, 0 );
   f->fpos=0;
   f->cbuf=512;
   f->p   =512;
   return 1;
}

int f_create( F_ILE *f, char *name)
{
   if (( f->handle=_lcreat( name, 0 ))==HFILE_ERROR )
      return 0;

   f->fsize=0;
   f->fpos=0;
   f->cbuf=512;
   f->p   =512;
   return 1;
}

void f_close( F_ILE *f )
{
   _lclose(f->handle);
   f->fsize=0;
   f->fpos=0;
}

int f_getc( F_ILE *f )
{
   int c; long rd;
   if ( f->p < f->cbuf )
   {
      c= f->buf[f->p++] & 0x0FF;
      return c;
   }
   rd= f->fsize - f->fpos;
   if ( rd>512L ) rd=512L;
   _llseek( f->handle, f->fpos, 0);
   _lread( f->handle, f->buf, rd);
   f->cbuf = rd;
   f->fpos += rd;
   f->p    = 0;
   if ( rd )
   {
      c= f->buf[f->p++] & 0x0FF;
      return c;
   } else return -1;
}

void LoadPlayList( char *name )
{
   F_ILE in;
   char s[256]; int i, c, n;

   if ( !f_open( &in, name, OF_READ )) return;
   i=0;
   n=1;
   while (( c=f_getc( &in )) != -1 )
   {
      s[i++]=c;
      if ( c== 0x0D || c== 0x0A || i>250 )
      {
         if ( i ) s[--i]=0;
         if (i)
         {
            if ( n ) PlayAdd( s ); else PlayAddP( s );
            n = 1-n;
         }
         i=0;
      }
   }
   f_close(&in);
}

void f_puts( F_ILE *f, char *s )
{
   int cr=0x0A0D;
   _lwrite( f->handle, s, strlen(s));
   _lwrite( f->handle, (const char *)&cr, 2);
}

void SavePlayList( char *name )
{
   F_ILE out;
   char s[256]; int i, c, n, total;

   if ( !f_create( &out, name )) return;
   total=SendMessage( listToPlay, LB_GETCOUNT, 0, 0 );

   for ( i=0; i<total; i++ )
   {
       SendMessage( listToPlay, LB_GETTEXT,
                    i, (LONG)(LPSTR) s );
       f_puts( &out, s );
       SendMessage( listToPlayPath, LB_GETTEXT,
                    i, (LONG)(LPSTR) s );
       f_puts( &out, s );
   }
   f_close(&out);
}

#pragma argsused
void WMDropFiles( WPARAM WParam, HWND hWnd)
{
	int TotalNumberOfFiles;
	int nFileLength;
        int i,j;
        char name[128]; char *path;

//msg.WParam contains the Handle of internal data structure which has
//information about the dropped file(s)
        HDROP hDrop = (HANDLE) WParam;

//	Get a total number of files dropped, by passing 0xFFFF for
//	index parameter to DragQueryFile
        TotalNumberOfFiles = DragQueryFile( hDrop , 0xFFFFFFFF, NULL, 0 );

        for (i = 0; i< TotalNumberOfFiles; i++ )
        {
//	Get the length of a filename by telling DragQueryFile
//	which file your querying about ( i ), and passing NULL as buffer
	nFileLength  = DragQueryFile( hDrop , i , NULL, 0 );
//      Copy a file name.
        DragQueryFile( hDrop , i, name, nFileLength + 1 );

        j=nFileLength;

           path = &name[j];
           while (j--)
           {
              if ( *path == '\\')
              {
                 path++; // ASCIIZ filename
                 break;
              }
              path--;
           }
           PlayAdd( path ); *path = '\0';
           PlayAddP( name );
        }

//	Release the memory shell allocated for this handle
	DragFinish( hDrop );
}


// local function to fill the files list
void FillFileList(void)
{
    SendMessage( listFrom, LB_RESETCONTENT, 0, 0);
    SendMessage( listFrom, LB_DIR,
                 DDL_ARCHIVE | DDL_HIDDEN | DDL_READONLY,
                 (LPARAM)(LPSTR)"*.*");
}

// local function to fill the directory list
void FillDirList(void)
{
    SendMessage( listDir, LB_RESETCONTENT, 0, 0);
    SendMessage( listDir, LB_DIR, DDL_DIRECTORY | DDL_EXCLUSIVE,
                        (LPARAM)(LPSTR)"*.*");
}

// local function to fill the drives list, return last drive index
int FillDriveList(void)
{
    SendMessage( listDrive, CB_RESETCONTENT, 0, 0);
    return SendMessage( listDrive, CB_DIR,
                        DDL_DRIVES | DDL_EXCLUSIVE,
                        (LONG)(LPSTR)"*");
}

// local function to simplify creating buttons etc.
HWND CreateWnd( char *cLass, char *title, long style, HWND hwnd, HMENU id )
{
    return CreateWindow( cLass, title,
                WS_BORDER | WS_CHILD | WS_VISIBLE | style,
                0, 0, 0, 0,             // set size later in WM_SIZE msg
                hwnd,                   // parent
                (HMENU) id,        // control ID
                instance,
                NULL);                  // no window creation data
}

// local function to simplify creating buttons etc.
HWND CreateWndx( char *cLass, char *title, long style, HWND hwnd, HMENU id )
{
    return CreateWindowEx( WS_EX_CLIENTEDGE,
                cLass, title,
                WS_BORDER | WS_CHILD | WS_VISIBLE | style,
                0, 0, 0, 0,             // set size later in WM_SIZE msg
                hwnd,                   // parent
                (HMENU) id,        // control ID
                instance,
                NULL);                  // no window creation data
}

// height of toolbar plus gap
#define W_TOP (10+32)

void PlaceWindows( int x, int y )
{
   int h;
   h=y/3;
   MoveWindow(editWinHandle,
       0, W_TOP+y-2-y/3,    // starting x- and y-coordinates
       x,         // width of client area
       max((h-W_TOP),20),       // height of client area
       TRUE);                  // repaint window
   MoveWindow(listFrom,
       10, isArcname ? W_TOP+30 : W_TOP,    // starting x- and y-coordinates
       x/4,         // width of client area
       isArcname ? h-30 : h,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(staticArc,
       10, W_TOP,    // starting x- and y-coordinates
       isArcname ? x/4 : 0, // width of client area
       isArcname ? 20 : 0,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(listToPlay,
       3*x/4-10, W_TOP,    // starting x- and y-coordinates
       x/4,         // width of client area
       h,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(listDrive,
       10+10+x/4, W_TOP+h,    // starting x- and y-coordinates
       x/4,         // width of client area
       y/4,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(listDir,
       10+10+x/4, W_TOP,    // starting x- and y-coordinates
       x/4,         // width of client area
       h,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(btnADD,
       10, W_TOP+h,    // starting x- and y-coordinates
       120,         // width of client area
       22,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(btnPLAY,
       3*x/4-10, W_TOP+h,    // starting x- and y-coordinates
       50,         // width of client area
       22,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(btnCLEAR,
       50+10+3*x/4-10, W_TOP+h,    // starting x- and y-coordinates
       50,         // width of client area
       22,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(staticPos,
       10+10+x/4, W_TOP+h+40,    // starting x- and y-coordinates
       90,         // width of client area
       22,       // height of client area
       TRUE);                  // repaint window
   MoveWindow(hwndTrack,
       90+10+10+10+x/4, W_TOP+h+40,    // starting x- and y-coordinates
       max(x-130-x/4, 50 ),        // width of client area
       44,       // height of client area
       TRUE);                  // repaint window
    SendMessage( tbhwnd, TB_AUTOSIZE, 0, 0);
}


BOOL CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg,
        WPARAM wParam, LPARAM lParam);

void CallMixer(void)
{
  STARTUPINFO si=
  {
    sizeof(si),
    NULL, // reserved
    NULL, // desktop name (NT only)
    NULL, // title (use name of exe)
    0,0,0,0, // size and position of window
    0,0,     // or console process
    0,       // text attrib for console
    0,       // DWORD   dwFlags;
    0,       // WORD    wShowWindow;
    0,       // WORD    cbReserved2;
    NULL,    // LPBYTE  lpReserved2;
    0,0,0    // HANDLE  hStdInput,hStdOutput,hStdError;
  };

  PROCESS_INFORMATION pi;
  CreateProcess( NULL, // pointer to name of executable module
  "sndvol32.exe",      // pointer to command line string
  NULL,        // pointer to process security attributes
  NULL,        // pointer to thread security attributes
  0,   // handle inheritance flag
  0,   // creation flags
  NULL,        // pointer to new environment block
  NULL,        // pointer to current directory name
  &si,    // pointer to STARTUPINFO
  &pi          // pointer to PROCESS_INFORMATION
  );
}


// Function:     LRESULT mainWindowProc(HWND hwnd, UINT message, WPARAM wparam,
//                   LPARAM lparam);
//
// Description:  Main window message processing function

char *tips[]=
{"Open module", "Open Playlist", "Save Playlist",
 "Look into archive", "Copy to Playlist", "Delete from playlist",
 "Delete file",
 "Sound options", "Mixer", "Help",
 "Play", "Stop", "Play onemore" };

LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam)
{
    char        *fileName;
    int         i,h,total, to_path;
    char        aboutText[256];
    char name[256];
    char path[256];
    MIDASplayStatus playStatus;
    MIDASmoduleInfo moduleInfo;
    midpEchoEffect *effect;
    LPNMHDR nhdr;

    switch ( message )
    {
        case WM_NOTIFY: // âñÿêèå èçâåùåíè
            nhdr = (LPNMHDR)lparam;
            // ÷òî èìåííî è îò êîãî ïðèøëî ?
            if (nhdr->code == TTN_NEEDTEXT &&
                nhdr->idFrom >= TBAR_ID &&
                nhdr->idFrom <= TBAR_ID+14 )
            {
               strcpy( ((LPTOOLTIPTEXT)lparam)->szText,
                       tips[ nhdr->idFrom - TBAR_ID - 1]);
            }
            break;
        case WM_TIMER:
            if ( module != NULL )
            {
                if (MIDASgetPlayStatus( playHandle, &MPstatus))
                {
                   char info[60];
                   sprintf( info, "%2u : %2u : %2u",
                   MPstatus.position, MPstatus.pattern, MPstatus.row);
                   SendMessage( staticPos, WM_SETTEXT, 0,
                      (LPARAM)(LPSTR)info );
                   SendMessage(hwndTrack, TBM_SETPOS, (WPARAM) TRUE, // redraw flag
                     (LPARAM) (long)(50*MPstatus.position/modInfo.songLength));

                   // check if we are at end of song
                   if ( MPstatus.position<MPstatusO.position &&
                       // MPstatus.pattern<MPstatusO.pattern &&
                        MPstatus.row<MPstatusO.row )
                   {
                      if ( module != NULL )
                      {
                          StopFreeModule(playHandle, module);
                          module = NULL;
                          playHandle = 0;
                      }
                      memset( &MPstatusO, 0, sizeof(MPstatusO));
                      total=SendMessage( listToPlay, LB_GETCOUNT, 0, 0 );

                      for ( i=0; i<total; i++ )
                      {
                          if (SendMessage( listToPlay, LB_GETSEL, i, 0 )>0)
                          {
                             PlayIndex=i; break;
                          }
                      }
                      SendMessage( listToPlay, LB_SETSEL, FALSE, PlayIndex );
                      PlayIndex++;
                      if ( PlayIndex >= total) PlayIndex=0;
                      SendMessage( listToPlay, LB_SETSEL, TRUE, PlayIndex );
                      SendMessage( listToPlay, LB_GETTEXT,
                           PlayIndex, (LONG)(LPSTR) name );
                      SendMessage( listToPlayPath, LB_GETTEXT,
                           PlayIndex, (LONG)(LPSTR) path );
                      PlayFile( strcat(path,name) );
                   }
                   else
                   memcpy( &MPstatusO, &MPstatus, sizeof(MPstatus));
                }
            }
            else
            SendMessage( staticPos, WM_SETTEXT, 0,
                      (LPARAM)(LPSTR)" " );
            break;
        case WM_DROPFILES:
            WMDropFiles( wparam, hwnd);
            break;
        case WM_CREATE:
            DragAcceptFiles( hwnd , TRUE );
            /* Create edit box for console text: */
            editWinHandle = CreateWndx( "EDIT", NULL,
                WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE
                | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                hwnd, (HMENU) ID_EDIT );

            listFrom = CreateWndx( "LISTBOX", NULL,
                WS_VSCROLL | WS_HSCROLL | LBS_STANDARD |
                LBS_MULTIPLESEL | LBS_EXTENDEDSEL, hwnd,
                (HMENU) ID_LISTFROM);

            // not auto-sorted
            listToPlay = CreateWndx( "LISTBOX", NULL,
                WS_VSCROLL | WS_HSCROLL | LBS_MULTIPLESEL |
                LBS_EXTENDEDSEL | LBS_HASSTRINGS, hwnd,
                (HMENU) ID_LISTTOPLAY);
            listToPlayPath = CreateWndx( "LISTBOX", NULL,
                WS_VSCROLL | WS_HSCROLL | LBS_MULTIPLESEL |
                LBS_EXTENDEDSEL | LBS_HASSTRINGS, hwnd,
                (HMENU) ID_LISTTOPLAYPATH);

            listDir = CreateWndx( "LISTBOX", NULL,
                WS_VSCROLL | LBS_STANDARD, hwnd,
                (HMENU) ID_LISTDIR);
            FillDirList();

            staticPos = CreateWndx( "STATIC", NULL,
                0, hwnd,
                (HMENU) ID_STATICPOS);

            staticArc = CreateWndx( "STATIC", NULL,
                0, hwnd,
                (HMENU) ID_STATICARC);

            InitCommonControls(); // loads common control's DLL

            hwndTrack = CreateWndx(
                        TRACKBAR_CLASS,         // class name
                        NULL,     // title (caption)
                        WS_CHILD | WS_VISIBLE |
                        TBS_AUTOTICKS,  // style
                        hwnd,                   // parent window
                        (HMENU) ID_TRACKBAR     // control identifier
                        );

            SendMessage(hwndTrack, TBM_SETRANGE,
                        (WPARAM) TRUE,                   // redraw flag
                        (LPARAM) MAKELONG( 0, 50));  // min. & max. positions
            SendMessage(hwndTrack, TBM_SETTICFREQ,
                        (WPARAM) 2,  // every 2'nd                 // redraw flag
                        (LPARAM) 2); // starting at 2
            SendMessage(hwndTrack, TBM_SETPAGESIZE,
                        0, (LPARAM) 10);                  // new page size


            listDrive = CreateWnd( "COMBOBOX", "Drive",
                WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST, hwnd,
                (HMENU) ID_LISTDRIVE);
            i=FillDriveList();

            GetCurrentDirectory( sizeof( curDir ) -1, curDir );
            while (i > -1)
            {
               char drive[20];
               SendMessage( listDrive, CB_GETLBTEXT,
                        i, (LONG)(LPSTR) drive);
               if ( *(CharLower( drive )+2) ==
                    *(CharLower( curDir )))
               {
                   SendMessage( listDrive, CB_SETCURSEL, i, 0);
                   curDrive= *(CharLower( drive )+2); // remember it
               }
               i--;
            }

            btnADD = CreateWnd( "BUTTON", "Add To PlayList", 0,
                hwnd, (HMENU) ID_BTNADD);

            btnPLAY = CreateWnd( "BUTTON", "Play", 0,
                hwnd, (HMENU) ID_BTNPLAY);

            btnCLEAR = CreateWnd( "BUTTON", "Clear", 0,
                hwnd, (HMENU) ID_BTNCLEAR);

            mainWinHandle = hwnd;

            editWidth = editHeight = 0;

            /* Initialize text: */
            InitText();

            AddTextLine( copyRight );
            sprintf(aboutText, "Using MIDAS Sound System v%s,",
                MIDASgetVersionString());
            AddTextLine(aboutText);
            AddTextLine( copyRight2 );

            SendMessage(editWinHandle, WM_SETTEXT, 0, (LPARAM) text);

            InitMIDAS();

            AddTextLine("MIDAS Sound System initialized");

            if ( strlen(loadFileName) != 0 ) PlayFile(loadFileName);

            break;
        case WM_HSCROLL:
            if (( module != NULL ) &&
               (( LOWORD(wparam) == TB_THUMBTRACK ) ||
                ( LOWORD(wparam) == TB_PAGEDOWN   ) ||
                ( LOWORD(wparam) == TB_PAGEUP     ))
                )
            {
                MIDASsetPosition(playHandle,
                modInfo.songLength *
                HIWORD(wparam)/50 );  // position of slider
                memset( &MPstatusO, 0, sizeof(MPstatusO));
            }
            break;
        case WM_SIZE:
            /* Make the edit control the size of the window's client area: */
            editWidth = LOWORD(lparam);
            editHeight = HIWORD(lparam);
            PlaceWindows( LOWORD(lparam), HIWORD(lparam) );
            ShowTextBottom();
            return 0;

        case WM_COMMAND:
            switch ( LOWORD(wparam) )
            {
                case ID_LISTFROM:
                    if (HIWORD(wparam)== LBN_DBLCLK )
                    {
                      RECT rc;
                      GetClientRect( hwnd, (LPRECT) &rc );

                      i=SendMessage( listFrom, LB_GETCURSEL, 0, 0 );
                      SendMessage( listFrom, LB_GETTEXT, i, (LPARAM)(LPSTR)aboutText );
                      if ( strstr( aboutText, ".rar" ))
                      {
                         fill_RAR( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                      else
                      if ( strstr( aboutText, ".arj" ))
                      {
                         fill_ARJ( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                      else
                      if ( strstr( aboutText, ".zip" ))
                      {
                         fill_ZIP( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                      else goto add;
                    }
                    break;
                case ID_LISTDIR:
                    if (HIWORD(wparam)== LBN_DBLCLK )
                    {
                      i=SendMessage( listDir, LB_GETCURSEL, 0, 0 );
                      SendMessage( listDir, LB_GETTEXT, i, (LPARAM)(LPSTR)aboutText );
                      aboutText[strlen(aboutText)-1]=0;
                      if (SetCurrentDirectory( &aboutText[1] ))
                      {
                         FillDirList();
                         FillFileList();
                      }
                    }
                    if (HIWORD(wparam)== LBN_SELCHANGE && isArcname )
                    {
                         RECT rc;
                         GetClientRect( hwnd, (LPRECT) &rc );
                         FillFileList();
                         isArcname = 0;
                         PlaceWindows( rc.right, rc.bottom );
                    }
                    break;
                case ID_LISTDRIVE:
                    if (HIWORD(wparam)== CBN_SELENDOK )
                    {
                      char drive[20];
                      i=SendMessage( listDrive, CB_GETCURSEL, 0, 0 );
                      SendMessage( listDrive, CB_GETLBTEXT, i, (LPARAM)(LPSTR)drive);
                      strcpy( &drive[3], ":" );
                      if (SetCurrentDirectory( &drive[2] ))
                      {
                         FillDirList();
                         FillFileList();
                         SendMessage( listDrive, CB_GETLBTEXT,
                         i, (LONG)(LPSTR) drive);
                         curDrive= *(CharLower( drive )+2); // remember it
                      }
                      else
                      {
                         sprintf( drive, "%c:", curDrive );
                         SetCurrentDirectory( drive );
                         i=FillDriveList();

                         GetCurrentDirectory( sizeof( curDir ) -1, curDir );
                         while (i > -1)
                         {
                            SendMessage( listDrive, CB_GETLBTEXT,
                                     i, (LONG)(LPSTR) drive);
                            if ( *(CharLower( drive )+2) ==
                                 *(CharLower( curDir )))
                            {
                                SendMessage( listDrive, CB_SETCURSEL, i, 0);
                                curDrive= *(CharLower( drive )+2); // remember it
                            }
                            i--;
                         }
                      }
                    }
                    break;

                case TBAR_ID+9:     // Mixer
                    CallMixer();
                    break;

                case TBAR_ID+6:     // delete one file from playlist
                    if ((i=SendMessage( listToPlay, LB_GETCURSEL, 0, 0))
                        != LB_ERR )
                    {
                       int k;
                       SendMessage( listToPlay, LB_DELETESTRING,
                                    (WPARAM) i, 0 );
                       k=SendMessage( listToPlayPath, LB_DELETESTRING,
                                    (WPARAM) i, 0 );
                       if ( k>0 )
                       {
                          SendMessage( listToPlay, LB_SETSEL,
                          TRUE, min(k-1, i));
                       }
                    }
                    break;

                case TBAR_ID+11:
                case ID_BTNPLAY:
                    if ((i=SendMessage( listToPlay, LB_GETCURSEL, 0, 0))
                        != LB_ERR )
                    {
                       if ( SendMessage( listToPlay, LB_GETTEXT, i,
                       (LPARAM)(LPSTR) name) != LB_ERR )
                       {
                          SendMessage( listToPlayPath, LB_GETTEXT, i,
                          (LPARAM)(LPSTR) path);
                          PlayFile( strcat( path, name ));
                          PlayIndex=i;
                       }
                    }
                    break;
                case TBAR_ID+4:
                // open archive content
                    {
                      RECT rc;
                      GetClientRect( hwnd, (LPRECT) &rc );

                      i=SendMessage( listFrom, LB_GETCURSEL, 0, 0 );
                      SendMessage( listFrom, LB_GETTEXT, i, (LPARAM)(LPSTR)aboutText );
                      if ( strstr( aboutText, ".rar" ))
                      {
                         fill_RAR( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                      else
                      if ( strstr( aboutText, ".arj" ))
                      {
                         fill_ARJ( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                      else
                      if ( strstr( aboutText, ".zip" ))
                      {
                         fill_ZIP( listFrom, aboutText );
                         PlaceWindows( rc.right, rc.bottom );
                      }
                    }
                    break;
                case ID_BTNCLEAR:
                    SendMessage( listToPlay, LB_RESETCONTENT, 0, 0);
                    SendMessage( listToPlayPath, LB_RESETCONTENT, 0, 0);
                    break;
                case TBAR_ID+5:
                case ID_BTNADD:
                    add:
                    total=SendMessage( listFrom, LB_GETCOUNT, 0, 0 );
                    GetCurrentDirectory( sizeof( curDir ) -1, curDir );
                    sprintf( path, "%s%s%s\\", curDir,
                       isArcname ? "ARCHIVE:" : "",
                       isArcname ? cArcname : ""
                       );
                    for ( i=0; i<total; i++ )
                    {
                        if (SendMessage( listFrom, LB_GETSEL, i, 0 )>0)
                        {
                           SendMessage( listFrom, LB_GETTEXT,
                           i, (LONG)(LPSTR) name );
                           PlayAdd( name ); PlayAddP( path );
                        }
                    }
                    break;
                case TBAR_ID+1:
                case MENU_FILE_OPEN:
                    fileName = GetFileName("Open a single File", NULL);
                    if ( fileName != NULL )
                        PlayFile(fileName);
                    break;

                case MENU_FILE_PLAYLIST:
                    fileName = GetFileName("Add a File(s) to Playlist",
                               &to_path);
                    if ( fileName != NULL )
                    {
                        // if a single file, looks like "d:\dir\name.ext"
                        if ( to_path>0 && fileName[to_path-1]=='\\' )
                        {
                            PlayAdd( &fileName[ to_path ] );
                            fileName[ to_path ]=0;
                            PlayAddP( fileName );
                        } else
                        // if more file, looks
                        // like "d:\dir\0name.ext\0name2.ext\0\0"
                        while ( fileName[to_path] )
                        {
                         /*  if ( IsArchive(&fileName[to_path]))
                           {
                              sprintf( path, "%sARCHIVE:%s\\",
                                fileName, &fileName[to_path] );
                           }
                           else   */
                           sprintf( path, "%s\\", //%s",
                                fileName, &fileName[to_path] );
                           PlayAddP( path );
                           PlayAdd( &fileName[ to_path ] );
                           while ( fileName[to_path] ) to_path++;
                           to_path++;
                        }
                    }
                    break;

                case TBAR_ID+3:
                case MENU_FILE_PLAYLIST_SAVE:
                    fileName = GetFileName("Save playlist", NULL);
                    if ( fileName != NULL )
                    SavePlayList(fileName);
                    break;
                case TBAR_ID+2:
                case MENU_FILE_PLAYLIST_OPEN:
                    fileName = GetFileName("Load playlist", NULL);
                    if ( fileName != NULL )
                    LoadPlayList(fileName);
                    break;
                case MENU_FILE_CHDIR:
                    fileName = GetFileName("Select song Directory", NULL);
                    if (1) //( fileName != NULL )
                    {
                        // strcpy( pickDir, fileName );
                         FillFileList();
                    }
                    break;
                case TBAR_ID+12:
                case MENU_FILE_CLOSE:
                    if ( module != NULL )
                    {
                        StopFreeModule(playHandle, module);
                        module = NULL;
                        playHandle = 0;
                    }
                    viewWindowList.SongChanged();
                    break;

                case MENU_FILE_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case TBAR_ID+8:
                case MENU_OPTIONS_SOUND:
                    if ( module != NULL )
                    {
                        StopFreeModule(playHandle, module);
                        module = NULL;
                        playHandle = 0;
                    }
                    viewWindowList.SongChanged();
                    effect = currentEffect;
                    if ( currentEffect != NULL )
                        currentEffect->Deactivate();
                    CloseMIDAS();
                    if ( MIDASconfig() )
                 ; //      MIDASwriteConfigRegistry((DWORD) HKEY_CURRENT_USER,
  // 1.11                 SoundOptions();
                    InitMIDAS();
                    if ( effect != NULL )
                        effect->Activate();
                    break;

                case MENU_VIEWS_SONGINFO:
                    songInfoView->CreateViewWindow();
                    break;

                case MENU_VIEWS_INSTLIST:
                    instListView->CreateViewWindow();
                    break;

                case MENU_VIEWS_ECHOEFFECTS:
                    echoEditView->CreateViewWindow();
                    break;

                case MENU_PLAYING_NEXTPOSITION:
                    if ( module != NULL )
                    {
                        MIDASgetPlayStatus(playHandle, &playStatus);
                        MIDASgetModuleInfo(module, &moduleInfo);
                        if ( playStatus.position < (moduleInfo.songLength-1) )
                            MIDASsetPosition(playHandle,
                                             playStatus.position + 1);
                      memset( &MPstatusO, 0, sizeof(MPstatusO));
                    }
                    break;

                case MENU_PLAYING_PREVPOSITION:
                    if ( module != NULL )
                    {
                        MIDASgetPlayStatus(playHandle, &playStatus);
                        MIDASgetModuleInfo(module, &moduleInfo);
                        if ( playStatus.position > 0 )
                            MIDASsetPosition(playHandle,
                                             playStatus.position - 1);
                      memset( &MPstatusO, 0, sizeof(MPstatusO));
                    }
                    break;

                case MENU_PLAYING_CENTERCHANNELS:
                    if ( module != NULL )
                    {
                        MIDASgetModuleInfo(module, &moduleInfo);
                        for ( i = 0; i < (int) moduleInfo.numChannels; i++ )
                        {
                            /* Kluge: */
                            if ( !MIDASsetSamplePanning(
                                (MIDASsamplePlayHandle) i + 1,
                                MIDAS_PAN_MIDDLE))
                                MIDASerror();
                        }
                    }
                    break;

                case MENU_PLAYING_FILTER:
                    filterMode++;
                    if ( filterMode > 2 )
                        filterMode = 0;
                    MIDASsetOption(MIDAS_OPTION_FILTER_MODE, filterMode);
                    sprintf(aboutText, "Filter mode %u", filterMode);
                    AddTextLine(aboutText);
                    break;

                case MENU_OPTIONS_PREFERENCES:
                    DialogBox(instance, "PREFERENCES", hwnd, (DLGPROC)
                        PreferencesDialogProc);
                    break;

                case MENU_HELP_ABOUT:
                    sprintf( curDir, /* aboutText, */ "%s"
                        "Using MIDAS Sound System v%s\n%s",
                        copyRight, MIDASgetVersionString(), copyRight2 );
                //    MessageBox(mainWinHandle, aboutText, "About MidpNT",
                //        MB_OK | MB_ICONINFORMATION);
                    DialogBox( instance, "About",
			       hwnd, (DLGPROC) DlgProcAbout);
                    break;

                case MENU_OPTIONS_SAVE_WIN:
                    SaveViews();
                    SaveMainWindow();
                    break;

                case MENU_OPTIONS_RESTORE_WIN:
                    RestoreViews();
                    RestoreMainWindow();
                    break;

                case MENU_PLAYING_LIST_INSTRUMENTS:
                    ListInstruments();
                    break;
            }
            break;

        case WM_DESTROY:
            CloseMIDAS();
            PostQuitMessage(0);
            break;

        default:
            return(DefWindowProc(hwnd, message, wparam, lparam));
    }
    return 0;
}

// about dialog window proc
#pragma argsused
BOOL CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP bmp, obmp;
    static HDC hdc, chdc;
    static BITMAP bm;
    int x,y;
    long ticks;
    char is[8];
    switch (msg)
    {
	case WM_PAINT:
	    bmp=LoadBitmap(instance,"me");
	    hdc=GetDC(hwndDlg);
	    chdc=CreateCompatibleDC(hdc);
	    obmp=SelectObject(chdc,bmp);

	    GetObject(bmp, sizeof(BITMAP), &bm);
	    for (x=0; x<8; x++ ) is[x]=0xFF; // ¬ áá¨¢ 8x8 ¡¨â
            while ( *(short int *)(&is[0]) != 0 ||
                    *(short int *)(&is[2]) != 0 ||
                    *(short int *)(&is[4]) != 0 ||
                    *(short int *)(&is[6]) != 0  )
	    {
	    rnd:
	       ticks=GetTickCount();
	       x=rand()%8; y=rand()%8;
	       if ((is[x] & (1<<y))==0) goto rnd;

	       is[x] &= ~(1<<y);
	    BitBlt(hdc, 10+bm.bmWidth/8*x, 10+bm.bmHeight/8*y,
		   bm.bmWidth/8, bm.bmHeight/8,
		   chdc, 0+bm.bmWidth/8*x, 0+bm.bmHeight/8*y, SRCAND);
	       while (abs(ticks-GetTickCount())<120);
//	    BitBlt(hdc, 10, 10, bm.bmWidth, bm.bmHeight,
//		   chdc, 0, 0, SRCAND);
	    };
	    SelectObject(chdc,obmp);
	    DeleteObject( bmp );
	    DeleteDC(chdc);
	    ReleaseDC(hwndDlg, hdc );
	    return 0;
	case WM_INITDIALOG:
            SetDlgItemText( hwndDlg,  // handle of dialog box
                102, // identifier of control
                (LPCTSTR)  curDir   // text to set
               );

            return  TRUE;
	case WM_COMMAND:
	    switch ( wParam )
	    {
                case 101:  // ª­®¯ª  Žª ¤¨ «®£ 
		   EndDialog( hwndDlg, TRUE );
		   return TRUE;
	    };
	    break;
	case WM_CLOSE:
	    EndDialog( hwndDlg, TRUE );
	default:
	    return 0;
     };
};


// $Log: MidpNT.cpp $
// Revision 1.15  1997/01/25 13:17:44  pekangas
// Rewrote archive support, added an icon
//
// Revision 1.14  1997/01/17 17:51:53  pekangas
// Fixed a Visual C warning
//
// Revision 1.13  1997/01/17 17:41:35  pekangas
// Text is also added to main window when in minimal UI mode
// (it's not usually visible though)
//
// Revision 1.12  1997/01/17 00:08:23  pekangas
// Added minimal UI mode - command line switch "-m"
//
// Revision 1.11  1997/01/14 22:25:11  pekangas
// Finally fixed (?) all command line arguments with spaces
//
// Revision 1.10  1997/01/14 20:47:40  pekangas
// Fixed channel centering (a stupid off-by-one error)
//
// Revision 1.9  1997/01/14 17:42:08  pekangas
// Changed to use MIDAS DLL API
//
// Revision 1.8  1997/01/14 16:36:14  pekangas
// Fixed support for files names with spaces as command line arguments
//
// Revision 1.7  1996/08/03 13:06:08  pekangas
// Added 1 to "view instruments" instrument numbers
//
// Revision 1.6  1996/08/02  20:14:30  pekangas
// Added View Instruments -function
//
// Revision 1.5  1996/07/16  20:20:01  pekangas
// Added an error message if loading accelerators fails
// (this happens if resources are not found)
//
// Revision 1.4  1996/07/16  19:11:59  pekangas
// Fixed WinMain definition for Visual C
//
// Revision 1.3  1996/07/04  17:48:41  pekangas
// Converted to LFs
//
