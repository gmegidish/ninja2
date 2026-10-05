/*      sdl.c
 *
 * Open Sound System Sound Device
 *
 * $Id: sdl.c,v 1.20.2.2 1998/02/26 20:52:56 pekangas Exp $
 *
 * Copyright 1996-1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include "SDL.h"
#include "SDL_audio.h"

#include "lang.h"

#include "midas.h"
#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "dsm.h"
#include "mglobals.h"
#include "gmplayer.h"

RCSID(char const *sdl_rcsid = "$Id: sdl.c,v 1.20.2.2 1998/02/26 20:52:56 pekangas Exp $";)

//#define DUMPBUFFER

/* Sound Device information */

/* Sound Device internal static variables */
static unsigned int mixRate, outputMode;
static unsigned int mixElemSize;

static unsigned int updateMix;              /* number of elements to mix between
                                               two updates */
static unsigned int mixLeft;                /* number of elements to mix before
                                               next update */
#ifdef DUMPBUFFER
static FILE     *buff;
#endif


/****************************************************************************\
*       enum sdlFunctIDs
*       -----------------
* Description:  ID numbers for SDL Sound Device functions
\****************************************************************************/

enum sdlFunctIDs
{
    ID_sdlDetect = ID_sdl,
    ID_sdlInit,
    ID_sdlClose,
    ID_sdlGetMode,
    ID_sdlSetUpdRate,
    ID_sdlStartPlay,
    ID_sdlPlay
};


/* Local prototypes: */
int CALLING sdlSetUpdRate(unsigned updRate);


/****************************************************************************\
*
* Function:     int sdlDetect(int *result)
*
* Description:  Detects a SDL Sound Device
*
* Input:        int *result             pointer to detection result
*
* Returns:      MIDAS error code. Detection result (1 if detected, 0 if not)
*               is written to *result.
*
\****************************************************************************/

int CALLING sdlDetect(int *result)
{
    if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
        *result = 0;
    } else {
        *result = 1;
    }
    return OK;
}



static void sdlMixAudio(void *unused, Uint8 *stream, int len)
{
    Uint8 *mixbuf;
    int mixlen;
    int mixable;
    int error;

    /* Calculate number of mixing elements left: */
    mixbuf = stream;
    mixlen = (len / mixElemSize);
    
    /* Mix the data: */
    while ( mixlen > 0 ) {
        mixable = mixlen;

        if ( mixable > mixLeft ) {
            mixable = mixLeft;
        }
        mixLeft -= mixable;

        dsmMixData(mixable, mixbuf);

        if ( mixLeft == 0 ) {
            if ( midasGMPInit )
            {
                if ( (error = gmpPlay()) != OK )
                    midasError(error);
            }
            mixLeft = updateMix;
        }
        mixbuf += mixable * mixElemSize;
        mixlen -= mixable;
    }

#ifdef DUMPBUFFER
    fwrite(stream, len, 1, buff);
#endif
}

/****************************************************************************\
*
* Function:     int sdlInit(unsigned mixRate, unsigned mode)
*
* Description:  Initializes SDL Sound Device
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           output mode
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING sdlInit(unsigned int _mixRate, unsigned int mode)
{
    int error;
    int mixMode;
    SDL_AudioSpec wanted, actual;

    wanted.freq = _mixRate;

    if ( mode & sd8bit )
        wanted.format = AUDIO_U8;
    else
        wanted.format = AUDIO_U8;
    
    if ( mode & sdMono )
        wanted.channels = 1;
    else
        wanted.channels = 2;

    /* Good value for music */
    wanted.samples = 4096;

    /* Set up our mixing callback */
    wanted.callback = sdlMixAudio;
    wanted.userdata = NULL;

    /* Open the audio device and see what format we got */
    if ( SDL_OpenAudio(&wanted, &actual) < 0 ) {
        return(errSDFailure);
    }

    /* Check what format we got and use it */   

    mixRate = actual.freq;

    outputMode = 0;
    switch (actual.format) {
        case AUDIO_U8:
            outputMode |= sd8bit;
            break;
        case AUDIO_S16:
            outputMode |= sd16bit;
            break;
        default:
            return(errSDFailure);
    }

    switch (actual.channels) {
        case 1:
            outputMode = sdMono;
            break;
        case 2:
            outputMode = sdStereo;
            break;
        default:
            return(errSDFailure);
    }

    /* Calculate one mixing element size: */
    if ( outputMode & sd16bit )
        mixElemSize = 2;
    else
        mixElemSize = 1;
    if ( outputMode & sdStereo )
        mixElemSize <<= 1;

    /* Check correct mixing mode: */
    mixMode = 0;
    if ( outputMode & sdStereo )
        mixMode = dsmMixStereo;
    else
        mixMode = dsmMixMono;

    if ( outputMode & sd16bit )
        mixMode |= dsmMix16bit;
    else
        mixMode |= dsmMix8bit;

    if ( mOversampling )
        mixMode |= dsmMixInterpolation;
    
    /* Initialize Digital Sound Mixer: */
    if ( (error = dsmInit(mixRate, mixMode)) != OK )
        PASSERROR(ID_sdlInit)

    /* Set update rate to 50Hz: */
    if ( (error = sdlSetUpdRate(5000)) != OK )
        PASSERROR(ID_sdlInit)

#ifdef DUMPBUFFER
    buff = fopen("buffer.raw", "wb");
#endif
    return OK;
}




/****************************************************************************\
*
* Function:     sdlClose(void)
*
* Description:  Uninitializes SDL Sound Device
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int sdlClose(void)
{
    int         error;

#ifdef DUMPBUFFER
    fclose(buff);
#endif

    SDL_CloseAudio();

    /* Uninitialize Digital Sound Mixer: */
    if ( (error = dsmClose()) != OK )
        PASSERROR(ID_sdlClose)

    return OK;
}




/****************************************************************************\
*
* Function:     int sdlGetMode(unsigned *mode)
*
* Description:  Reads the current output mode
*
* Input:        unsigned *mode          pointer to output mode
*
* Returns:      MIDAS error code. Output mode is written to *mode.
*
\****************************************************************************/

int CALLING sdlGetMode(unsigned int *mode)
{
    *mode = outputMode;

    return OK;
}


/****************************************************************************\
*
* Function:     int sdlStartPlay(void)
*
* Description:  Prepares for playing - checks how many bytes we can mix
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING sdlStartPlay(void)
{
    SDL_PauseAudio(0);
    return OK;
}




/****************************************************************************\
*
* Function:     int sdlPlay(int *callMP);
*
* Description:  Plays the sound - mixes the correct amount of data with DSM
*               and copies it to output buffer with post-processing.
*               Also takes care of sending fully mixed buffer to the 
*               output device.
*
* Input:        int *callMP             pointer to music player calling flag
*
* Returns:      MIDAS error code. If enough data was mixed for one updating
*               round and music player should be called, 1 is written to
*               *callMP, otherwise 0 is written there. Note that if music
*               player can be called, sdlPlay() should be called again
*               with a new check for music playing to ensure the mixing buffer
*               gets filled with new data.
*
\****************************************************************************/

int CALLING sdlPlay(int *callMP)
{
    /* The audio is played asynchronously - don't do anything here */
    *callMP = 0;

    return OK;
}


int CALLING sdlSuspend(void)
{
    SDL_PauseAudio(1);
    return OK;
}

int CALLING sdlResume(void)
{
    SDL_PauseAudio(0);
    return OK;
}


/****************************************************************************\
*
* Function:     int sdlSetUpdRate(unsigned updRate);
*
* Description:  Sets the channel value update rate (depends on song tempo)
*
* Input:        unsigned updRate        update rate in 100*Hz (eg. 50Hz
*                                       becomes 5000).
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING sdlSetUpdRate(unsigned int updRate)
{
    /* Calculate number of elements to mix between two updates: (even) */
    mixLeft = updateMix = ((unsigned) ((100L * (ulong) mixRate) /
        ((ulong) updRate)) + 1) & 0xFFFFFFFE;

    return OK;    /* We lie here - we can't change the mixing tempo */
}

    /* SDL Sound Device structure: */

SoundDevice     SDL = {
    0,                                  /* tempoPoll = 0 */
    sdUseMixRate | sdUseOutputMode | sdUseDSM,  /* configBits */
    0,                                  /* port */
    0,                                  /* IRQ */
    0,                                  /* DMA */
    1,                                  /* cardType */
    1,                                  /* numCardTypes */
    sdMono | sdStereo | sd8bit | sd16bit,       /* modes */

    "Simple DirectMedia Layer",         /* name */
    NULL,                               /* cardNames */
    0,                                  /* numPortAddresses */
    NULL,                               /* portAddresses */

    &sdlDetect,
    &sdlInit,
    &sdlClose,
    &sdlSuspend,
    &sdlResume,
    &dsmGetMixRate,
    &sdlGetMode,
    &dsmOpenChannels,
    &dsmCloseChannels,
    &dsmClearChannels,
    &dsmMute,
    &dsmPause,
    &dsmSetMasterVolume,
    &dsmGetMasterVolume,
    &dsmSetAmplification,
    &dsmGetAmplification,
    &dsmPlaySound,
    &dsmReleaseSound,
    &dsmStopSound,
    &dsmSetRate,
    &dsmGetRate,
    &dsmSetVolume,
    &dsmGetVolume,
    &dsmSetSample,
    &dsmGetSample,
    &dsmSetPosition,
    &dsmGetPosition,
    &dsmGetDirection,
    &dsmSetPanning,
    &dsmGetPanning,
    &dsmMuteChannel,
    &dsmAddSample,
    &dsmRemoveSample,
    &sdlSetUpdRate,
    &sdlStartPlay,
    &sdlPlay
#ifdef SUPPORTSTREAMS
    ,
    &dsmStartStream,
    &dsmStopStream,
    &dsmSetLoopCallback,
    &dsmSetStreamWritePosition,
    &dsmPauseStream
#endif    
};


/*
 * $Log: sdl.c,v $
*/
