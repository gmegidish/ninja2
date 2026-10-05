
#include <sdl.h>

#include "../midas/midasdll.h"

#include <windows.h>
#include <fcntl.h>

#include "ninja2.h"
#include "anims.h"
#include "gfx.h"
#include "files.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "load_raw_file.c"
#include "vars.c"

static SDL_Surface *screen, *final;
static unsigned raw_file_length;
static unsigned char *raw_file_data;

static void
ShowFrame (frame)
       uint8 *frame;
{
        if (!frame) {
               return;
        }

        memcpy (sGraphics.pScreen, frame, 320*256);
}

static void
VBlank (void)
{
}

static void
VerticalBlank (void)
{
#define FRAMES_PER_SEC 30
       //ShowScreen();
    static Uint32   next_tick = 0;
    Uint32          this_tick;

    this_tick = SDL_GetTicks();
    if (this_tick < next_tick) {
        SDL_Delay(next_tick - this_tick);
    }
    next_tick = this_tick + (1000 / FRAMES_PER_SEC);
       //sTimer.vPrevious = SDL_GetTicks();
}

static void
ShowScreen (void)
{
       register int y, x;
       register uint8 *p1, *p2;

       #ifndef USE_FINAL

       SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
       VerticalBlank();
       //SDL_Flip (screen);
       Sleep (30);

       #else

       for (y=0; y<256; y++) {
               p1 = (uint8*)screen->pixels+(y*320);
               p2 = (uint8*)final->pixels+(y*1280);
               for (x=0; x<320; x++) {
                    *p2++ = *p1;
                    *p2++ = *p1++;
               }

               memcpy (p2, p2-640, 640);
       }

       SDL_UpdateRect (final, 0, 0, final->w, final->h);

       #endif
}

static void
ClearScreen (void)
{
        SDL_Rect rect;

        rect.x = 0;
        rect.y = 0;
        rect.w = screen->w;
        rect.h = screen->h;
        SDL_FillRect (screen, &rect, 0);

        ShowScreen();
}

static int
GetKB (void)
{
       return 0; //XXX
}

static void
ResetPalette (void)
{
       SDL_Color colors[256];

       memset ((void*)&colors, '\0', sizeof(colors));
       SDL_SetColors (screen, colors, 0, 256);
       ShowScreen();
}

int vFirstTime = TRUE;

static int
time_GetDelta (void)
{
       int delay;

       if (vFirstTime == TRUE) {
               sTimer.vPrevious = SDL_GetTicks();
               vFirstTime = FALSE;
       }

       sTimer.vCurrent = SDL_GetTicks();

       delay = sTimer.vCurrent - sTimer.vPrevious;
       if (delay > 0) {
               delay = 30 - delay;
               if (delay > 30) delay = 30;
               if (delay < 0) delay = 0;
               SDL_Delay (delay);
       }

       sTimer.vDelta = SDL_GetTicks() - sTimer.vPrevious;
       return sTimer.vDelta;
}

#include "ilbm.c"

#include "intro.c"
#include "animplayunpacked.c"
#include "animplay.c"
#include "animplay2.c"
#include "animplay3.c"
#include "animplay4.c"
#include "city.c"
#include "ninjaattack.c"
#include "ninjafalldown.c"
#include "ninjascrollup.c"
#include "ninjajumpup.c"
#include "ninjainair.c"
#include "ninjaintofog.c"
#include "mutantsurprise.c"
#include "mutanthead.c"
#include "logo.c"

static int
load_graphics (void)
{
        raw_file_length = load_raw_file ("ninja.000", (void**)&raw_file_data);
        if (!raw_file_length) {
               return 0;
        }

        /* initialize pointers */
        pIntroLayer1 = (raw_file_data + file_offsets[gSEQ1_LAYER1]);
        pIntroLayer2 = (raw_file_data + file_offsets[gSEQ1_LAYER2]);
        pIntroLayer3 = (raw_file_data + file_offsets[gSEQ1_LAYER3]);

        pCityLayer1 = (raw_file_data + file_offsets[gSEQ3_LAYER1]);
        pCityLayer2 = (raw_file_data + file_offsets[gSEQ3_LAYER2]);
        pCityLayer3 = (raw_file_data + file_offsets[gSEQ3_LAYER3]);

        pGraphics = raw_file_data;
        sGraphics.pScreen = screen->pixels;

        pLayer1 = (uint8*)malloc (400000);
        pLayer2 = (uint8*)malloc (400000);
        pLayer3 = (uint8*)malloc (400000);
        pVspace = (uint8*)malloc (320*400);

        return 1;
}

static void
melon_2035 (void)
{
        float nFade;
        float nCount;
        int isFadeUp;

        nFade = -64;
        nCount = 0;
        isFadeUp = TRUE;

        ResetPalette();

        time_GetDelta();

        while(1)
        {
                time_GetDelta();

                DepackILBM(pGraphics+file_offsets[gMELONSCOOP],pLayer1,0);
                ILBM_Palette(pGraphics+file_offsets[gMELONSCOOP],64,0,(int8)nFade);
                DrawLayerVert(pLayer1,0);
                ShowScreen();

                if(isFadeUp)
                {
                        nFade += ((float)sTimer.vDelta)/100;
                        if(nFade>=0)
                                isFadeUp = FALSE;
                }
                else
                {
                        nCount += ((float)sTimer.vDelta)/100;

                        if(nCount>=160)
                        {
                                nFade -= ((float)sTimer.vDelta)/100;
                                if(nFade<=-64)
                                        break;
                        }
                }
        }


        nFade = -64;
        nCount = 0;
        isFadeUp = TRUE;

        ResetPalette();

        memset(pLayer1,0,320*256);
        ClearScreen();

        time_GetDelta();

        while(1)
        {
                time_GetDelta();

                DepackILBM(pGraphics+file_offsets[g2035],pLayer1,0);
                ILBM_Palette(pGraphics+file_offsets[g2035],64,0,(int8)nFade);
                DrawLayerVert(pLayer1,0);
                ShowScreen();

                if(isFadeUp)
                {
                        nFade += ((float)sTimer.vDelta)/100;
                        if(nFade>=0)
                                isFadeUp = FALSE;
                }
                else
                {
                        nCount += ((float)sTimer.vDelta)/100;

                        if(nCount>=40)
                        {
                                nFade -= ((float)sTimer.vDelta)/100;
                                if(nFade<=-64)
                                        break;
                        }
                }
        }

        ResetPalette();
}

#define XSIZE 320
#define YSIZE 256

int
main (void)
{
       int ok, i;
       SDL_Surface *image;
       const SDL_VideoInfo *vidinfo;
       void *module;

//       setbuf (stdout, 0);

       ok = SDL_Init (SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
       if (ok != 0) {
               printf ("SDL failed\n");
               return 0;
       }

       MIDASstartup();

       ok = MIDASinit();
       if (ok) {
               printf ("midas detected soundcard\n");
       } else {
               printf ("midas hasnt detected\n");
               return 0;
       }

       //MIDASconfig();

       module = MIDASloadModule ("ninja2.xm");
       if (!module) {
               printf ("module failed on loading\n");
       }

       ok = SDL_DOUBLEBUF;
       vidinfo = SDL_GetVideoInfo();
       if (0) { //vidinfo->hw_available) {
               ok |= SDL_HWSURFACE;
       } else {
               ok |= SDL_SWSURFACE;
       }

       //SDL_ShowCursor (0);
       //SDL_WM_GrabInput (SDL_GRAB_ON);

       ok = SDL_VideoModeOK (XSIZE, YSIZE, 8, ok);
       if (ok == 0) {
                 printf ("SDL_VideoModeOK failed\n");
                 return 0;
       }

       final = SDL_SetVideoMode (XSIZE, YSIZE, 8, ok);
       printf ("setvideomode returned %x\n", final);
       if (!final) return 0;

       //SDL_WM_ToggleFullScreen (final);

       #ifdef USE_FINAL
       screen = SDL_AllocSurface (SDL_SWSURFACE, 320, 256, 8, 0, 0, 0, 0);
       if (!screen) return 0;
       #else
       screen = final;
       #endif

       SDL_WM_SetCaption ("Ninja 2", 0);
       //SDL_WM_ToggleFullScreen (final);

       if (!load_graphics()) {
                 printf ("init graphics failed!\n");
                 SDL_Quit();
                 return 0;
       }

       //melon_2035();

       printf ("MIDASplay returned %x\n", (void*)MIDASplayModule (module, 0));

       MIDASstartBackgroundPlay (50);

       Intro();
       Logo();
       City();
       AnimPlay2 (aSEQ4_CHASE, 28, 148, 0, pGraphics + file_offsets [gSEQ4_LAYER1], pGraphics+file_offsets[gSEQ4_LAYER2]);
       AnimPlay (aSEQ5_MKID, 6, 160, 6, pGraphics + file_offsets [gSEQ5_BACK]);
       AnimPlay3 (aSEQ6_LEGS, 16, 245, 40, pGraphics + file_offsets [gSEQ6_BACK]);
       NinjaScrollUp();
       MutantSurprise();

       AnimPlay(aSEQ8,4,205,1,pGraphics+file_offsets[gSEQ8_BACK]);

       NinjaJumpUp();
       NinjaInAir();
       NinjaFallDown();

       AnimPlay4 (aSEQ13, 4, 255, 0, pGraphics+file_offsets[gSEQ13_BACK]);
       AnimPlay (aSEQ13+2, 2, 185, 3, pGraphics+file_offsets[gSEQ13_BACK]);
       AnimPlay (aSEQ14, 6, 110, 4, NULL);
       AnimPlay (aSEQ15, 2, 110, 9, NULL);
       AnimPlay (aSEQ16, 6, 110, 4, NULL);
       AnimPlay (aSEQ17, 2, 110, 9, NULL);

       NinjaAttack();

       AnimPlay (aSEQ19, 17, 120, 0, NULL);

       for (i=0; i<64; i+=3)
       {
                VerticalBlank();
                ILBM_Palette (pGraphics+file_offsets[aSEQ19],64,0,i);
                ILBM_Palette (pGraphics+file_offsets[gSEQ20_STAND],64,0,i);
                ILBM_Palette (pGraphics+file_offsets[gSEQ6_BACK],64,64,i);
       }

       for( i=0; i<35; i++)
                VerticalBlank();

        // LALALALALALA //
        DepackILBM(pGraphics+file_offsets[gSEQ20_STAND],pLayer2,0);
        DepackILBM(pGraphics+file_offsets[gSEQ6_BACK],pLayer1,64);
        DrawLayerVert(pLayer1,0);
        DrawLayerVert(pLayer2,0);
        ShowScreen();

        for(i=0; i<64; i+=2)
        {
                VerticalBlank();
                ILBM_Palette(pGraphics+file_offsets[gSEQ20_STAND],64,0,64-i);
                ILBM_Palette(pGraphics+file_offsets[gSEQ6_BACK],64,64,64-i);
        }

        for(i=0; i<180; i++)
                VerticalBlank();

        VerticalBlank();
        ResetPalette();
        ClearScreen();
        VerticalBlank();

        // And the rest //
        AnimPlay4(aSEQ21_LAUGH,2,175,9,pGraphics+file_offsets[gSEQ13_BACK]);
        AnimPlay4(aSEQ21_CUT,9,295,0,pGraphics+file_offsets[gSEQ13_BACK]);
        AnimPlay(aSEQ21_CUT+9,2,175,7,pGraphics+file_offsets[gSEQ13_BACK]);

        for(i=0; i<25; i++)
                VerticalBlank();

        MutantHead();

        NinjaIntoFog();

        //Credits();

       SDL_Quit();
       return 0;
}
