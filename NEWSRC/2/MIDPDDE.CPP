// v 1.0
#include <windows.h>
#include <ddeml.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "midasdll.h"
#include "midproto.h"

DWORD           ddeInstance;
int             ddeInit = 0;

void DDEError(char *function, UINT error)
{
    char        msg[128];
    sprintf(msg, "DDE Failure - %s: %u", function, error);
    MessageBox(NULL, msg, "MidpNT Initialization error",
        MB_OK | MB_ICONEXCLAMATION);
    CloseMIDAS();
    ExitProcess(17);
}

HDDEDATA CALLBACK DdeServerCallback(
    UINT  uType,	// transaction type
    UINT  uFmt,	// clipboard data format
    HCONV  hconv,	// handle of the conversation
    HSZ  hsz1,	// handle of a string
    HSZ  hsz2,	// handle of a string
    HDDEDATA  hdata,	// handle of a global memory object
    DWORD  dwData1,	// transaction-specific data
    DWORD  dwData2 	// transaction-specific data
   )
{
    int         len;
    char        *str;

    uType = uType;
    uFmt = uFmt;
    hconv = hconv;
    hsz1 = hsz1;
    hsz2 = hsz2;
    hdata = hdata;
    dwData1 = dwData1;
    dwData2 = dwData2;

    switch ( uType )
    {
        case XTYP_CONNECT:
            len = DdeQueryString(ddeInstance, hsz1, NULL, 0, CP_WINANSI);
            if ( len == 0 )
                Panic("DdeServerCallback: Topic string length 0");
            str = new char[len+1];
            if ( !DdeQueryString(ddeInstance, hsz1, str, len+1, CP_WINANSI) )
                Panic("DdeServerCallback: DdeQueryString() failed");

            if ( !memcmp(str, "PlayFile:", 9) )
            {
                if ( strlen(str) > 9 )
                {
//                    AddTextLine(str);
                    PlayFile(str+9);
                }
            }
            else
                Panic("DdeServerCallback: Bad topic");

            return (HDDEDATA) TRUE;
    }

    return (HDDEDATA) 0L;
}

void StartDDEServer(void)
{
    UINT        ddeError;
    HSZ         strHandle;

    if ( (ddeError = DdeInitialize(&ddeInstance, DdeServerCallback,
        APPCLASS_STANDARD, 0)) !=
        DMLERR_NO_ERROR )
        DDEError("Server DdeInitialize", ddeError);

    ddeInit = 1;

    if ( (strHandle = DdeCreateStringHandle(ddeInstance, "MidpNT",
        CP_WINANSI)) == 0L )
        DDEError("DdeCreateStringHandle", DdeGetLastError(ddeInstance));

    if ( DdeNameService(ddeInstance, strHandle, (HSZ) 0L, DNS_REGISTER) == 0 )
        DDEError("DdeNameService", DdeGetLastError(ddeInstance));

    if ( !DdeFreeStringHandle(ddeInstance, strHandle) )
        DDEError("DdeFreeStingHandle", DdeGetLastError(ddeInstance));
}

HDDEDATA CALLBACK DdeClientCallback(
    UINT  uType,	// transaction type
    UINT  uFmt,	// clipboard data format
    HCONV  hconv,	// handle of the conversation
    HSZ  hsz1,	// handle of a string
    HSZ  hsz2,	// handle of a string
    HDDEDATA  hdata,	// handle of a global memory object
    DWORD  dwData1,	// transaction-specific data
    DWORD  dwData2 	// transaction-specific data
   )
{
    uType = uType;
    uFmt = uFmt;
    hconv = hconv;
    hsz1 = hsz1;
    hsz2 = hsz2;
    hdata = hdata;
    dwData1 = dwData1;
    dwData2 = dwData2;

    return (HDDEDATA) 0L;
}

int PlayDDE(char *fileName)
{
    char        *str;
    HSZ         server, topic;
    HCONV       conversation;
    UINT        ddeError;
    int         val;
    static DWORD       ddeInstance;
    char        *dummy;

    if ( strlen(fileName) )
    {
        str = new char[_MAX_PATH + 10];
        strcpy(str, "PlayFile:");
        /*
        if ( _fullpath(str+9, fileName, _MAX_PATH) == NULL )
            Panic("PlayDDE(): _fullpath failed");
        */
        if ( GetFullPathName(fileName, _MAX_PATH, str+9, &dummy) == 0 )
            Panic("PlayDDE(): GetFullPathName() failed");
    }
    else
        str = "PlayFile:";

    if ( (ddeError = DdeInitialize(&ddeInstance, DdeClientCallback,
        APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0)) !=
        DMLERR_NO_ERROR )
        DDEError("Client DdeInitialize", ddeError);

    ddeInit = 1;

    if ( (server = DdeCreateStringHandle(ddeInstance, "MidpNT",
        CP_WINANSI)) == 0L )
        DDEError("DdeCreateStringHandle", DdeGetLastError(ddeInstance));

    if ( (topic = DdeCreateStringHandle(ddeInstance, str,
        CP_WINANSI)) == 0L )
        DDEError("DdeCreateStringHandle", DdeGetLastError(ddeInstance));

    if ( (conversation = DdeConnect(ddeInstance, server, topic, NULL)) == 0L )
    {
        ddeError = DdeGetLastError(ddeInstance);
        if ( ddeError == DMLERR_NO_CONV_ESTABLISHED )
            val = 0;
        else
            DDEError("DdeConnect", ddeError);
    }
    else
    {
        val = 1;

        if ( !DdeDisconnect(conversation) )
            DDEError("DdeDisconnect", DdeGetLastError(ddeInstance));
    }

    if ( !DdeFreeStringHandle(ddeInstance, server) )
        DDEError("DdeFreeStingHandle", DdeGetLastError(ddeInstance));
    if ( !DdeFreeStringHandle(ddeInstance, topic) )
        DDEError("DdeFreeStingHandle", DdeGetLastError(ddeInstance));

    DdeUninitialize(ddeInstance);
    ddeInit = 0;

    return val;
}

void Panic(char *message)
{
    MessageBox(NULL, message, "MIDP PANIC",
        MB_OK | MB_ICONEXCLAMATION);
    CloseMIDAS();
    if ( ddeInit )
        DdeUninitialize(ddeInstance);
    ExitProcess(1);
}
