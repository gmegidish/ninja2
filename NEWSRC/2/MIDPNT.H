// v 1.0
//      MidpNT.h
// MIDAS Module Player for Windows NT global definitions
// $Id: midpnt.h,v 1.9 1997/02/19 10:09:33 pekangas Exp $
// Copyright 1996 Petteri Kangaslampi

#ifndef __MIDPNT_H
#define __MIDPNT_H

#define VERSION 1.0
#define FULLVERSION 1.0.2
#define VERSTR "1.0"
#define FULLVERSTR "1.0.2"

extern HANDLE   instance;
extern int      mixRate, stereo, force8bit;
extern MIDASmodule module;
// 1.11 extern MIDASmodulePlayHandle playHandle;
extern HWND     mainWinHandle;
extern DWORD    ddeInstance;
extern int      ddeInit;
extern int      viewsChildren;
extern int      minimalUI;
extern char     unzipDir[];
extern unsigned filterMode;

/* Custom window messages: */
enum midpWindowMessages
{
    MIDPMSG_SONGCHANGED = WM_USER+200
};

#endif

/*
 * $Log: midpnt.h,v $
 * Revision 1.9  1997/02/19 10:09:33  pekangas
 * Updated version number
 *
 * Revision 1.8  1997/01/25 13:17:01  pekangas
 * Rewrote archive support, now much better
 *
 * Revision 1.7  1997/01/17 00:08:39  pekangas
 * Added minimalUI flag, changed version to 0.2.1
 *
 * Revision 1.6  1997/01/14 17:42:08  pekangas
 * Changed to use MIDAS DLL API
 *
 * Revision 1.5  1996/10/14 20:42:38  pekangas
 * Changed version to 0.1.7
 *
 * Revision 1.4  1996/09/25  18:43:46  pekangas
 * Changed version number to 0.1.6
 *
 * Revision 1.3  1996/09/13  06:48:38  pekangas
 * Changed version number to 0.1.5
 *
 * Revision 1.2  1996/09/01  20:57:40  pekangas
 * Changed MidpNT revision number, converted to LFs, added RCS keywords
 *
*/
