
#include "ninja2.h"

void Intro (void)
{
        uint8   aFadeDown[35] = {
                0,0,0,5,14,44,44,14,5,2,5,14,44,44,14,10,5,2,1,0,0,0,0,0,0,0,0
        };

        float nFadePos=0;
        float   nFrameCounter=0;

        Sprite_t sSprite;
        uint8 *apStandAnim[16];

        bool bScroll = TRUE;
        float   nFrameNumber=0;

        int i=0;
        uint32 nLayer1XSize;
        uint32 nLayer2XSize;
        uint32 nLayer3XSize;

        nLayer1Xpos = 0;
        nLayer2Xpos = 0;
        nLayer3Xpos = 0;

        nLayer1XSize = ILBM_GetXSize(pIntroLayer1);
        nLayer2XSize = ILBM_GetXSize(pIntroLayer2);
        nLayer3XSize = ILBM_GetXSize(pIntroLayer3);

        ILBM_Palette(pIntroLayer1,64,0,-64);
        ILBM_Palette(pIntroLayer2,64,64,-64);
        ILBM_Palette(pIntroLayer3,64,128,-64);
        ILBM_Palette(pGraphics+file_offsets[aSEQ1_NINJA],64,192,-64);

        DepackILBM(pIntroLayer1,pLayer1,0);
        DepackILBM(pIntroLayer2,pLayer2,64);
        DepackILBM(pIntroLayer3,pLayer3,128);

        for(i=0; i<16; i++)
                apStandAnim[i] = pGraphics+file_offsets[aSEQ1_NINJA+i];

        sSprite.screen          = sGraphics.pScreen;
        sSprite.vsprite         = pVspace;
        sSprite.xscreen1  = 0;
        sSprite.yscreen1        = 0;
        sSprite.xscreen2        = 320;
        sSprite.yscreen2        = 256;
        sSprite.modulo          = 320;
        sSprite.xpos                    = 0;
        sSprite.ypos                    = 0;
        sSprite.xsize                   = 320;
        sSprite.ysize                   = 256;
        sSprite.xflip                   = 0;
        sSprite.yflip                   = 0;

        time_GetDelta();

        nFadePos = -64;

        while(GetKB()!=2)
        {
                VBlank();
                /* Get TIME delta */
                time_GetDelta();

                if (SDL_MUSTLOCK(screen)) {
                       printf ("MUSTLOCK\n");
                       SDL_LockSurface (screen);
                }

                if(nFrameCounter<=120)
                {
                        if(nFadePos<=0)
                        {
                                ILBM_Palette(pIntroLayer3,64,0,(int8)nFadePos);
                                ILBM_Palette(pIntroLayer3,64,64,(int8)nFadePos);
                                ILBM_Palette(pIntroLayer3,64,128,(int8)nFadePos);
                                ILBM_Palette(pGraphics+file_offsets[9],64,192,-(int8)nFadePos);
                        }
                        nFadePos+=((float)sTimer.vDelta)/100;
                }

                if(nFrameCounter>=150 & nFrameCounter <=180)
                        nFadePos = 0;

                /* Lightning 1 */
                if(nFrameCounter>=180 & nFrameCounter <=210)
                {
                        if(nFadePos<=25)
                        {
                                ILBM_Palette(pIntroLayer3,64,0,aFadeDown[(int)(nFadePos)]);
                                ILBM_Palette(pIntroLayer3,64,64,aFadeDown[(int)(nFadePos)+1]>>1);
                                ILBM_Palette(pIntroLayer3,64,128,aFadeDown[(int)(nFadePos)+2]>>2);
                                ILBM_Palette(pGraphics+file_offsets[9],64,192,aFadeDown[(int)(nFadePos)+2]>>2);
                        }
                        nFadePos+=((float)sTimer.vDelta)/50;
                }

                if(nFrameCounter>=250 & nFrameCounter<=340)
                        nFadePos=0;

                /* Lightning 2 */
                if(nFrameCounter>=365 & nFrameCounter<=400)
                {
                        if(nFadePos<=25)
                        {
                                ILBM_Palette(pIntroLayer3,64,0,aFadeDown[(int)(nFadePos)]);
                                ILBM_Palette(pIntroLayer3,64,64,aFadeDown[(int)(nFadePos)+1]>>1);
                                ILBM_Palette(pIntroLayer3,64,128,aFadeDown[(int)(nFadePos)+2]>>2);
                                ILBM_Palette(pGraphics+file_offsets[9],64,192,aFadeDown[(int)(nFadePos)+2]>>2);
                        }
                        nFadePos+=((float)sTimer.vDelta)/50;
                }

                if(nFrameCounter>=400 & nFrameCounter<= 410)
                        nFadePos=0;

                if(nFrameCounter>=420 & nFrameCounter<=455)
                {
                        if(nFadePos<=64)
                        {
                                ILBM_Palette(pIntroLayer3,64,0,-(int8)nFadePos);
                                ILBM_Palette(pIntroLayer3,64,64,-(int8)nFadePos);
                                ILBM_Palette(pIntroLayer3,64,128,-(int8)nFadePos);
                                ILBM_Palette(pGraphics+file_offsets[9],64,192,-(int8)nFadePos);
                        }
                        nFadePos+=((float)sTimer.vDelta)/30;
                }

                if(nFrameCounter>=475)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }

                /* Draw Layers */
                DrawLayerHor(pLayer1,(uint32)nLayer1Xpos,nLayer1XSize,0);
                DrawLayerHor(pLayer2,(uint32)nLayer2Xpos,nLayer2XSize,0);
                DrawLayerHor(pLayer3,(uint32)nLayer3Xpos,nLayer3XSize,55);

                /* Depack animation */
                DepackILBM(apStandAnim[(uint32)nFrameNumber],pVspace,192);
                sSprite.xpos = -(uint32)nLayer3Xpos+1080;
                Sprite_Draw((Sprite_t*)&sSprite);

                nFrameNumber += ((float)sTimer.vDelta)/120;

                if(nFrameNumber>=15)
                        nFrameNumber = 0;

                /* UNLOCK */
                if (SDL_MUSTLOCK(screen)) {
                       printf ("UNLOCK\n");
                       SDL_UnlockSurface (screen);
                }

                /* Show screen */
                ShowScreen();

                if(bScroll==TRUE)
                {
                        nLayer3Xpos += ((float)sTimer.vDelta)/25;
                        nLayer2Xpos += ((float)sTimer.vDelta)/67;
                        nLayer1Xpos += ((float)sTimer.vDelta)/83;
                }

                if(bScroll==FALSE)
                        nLayer1Xpos -= ((float)sTimer.vDelta)/100;

                if(nLayer2Xpos >= (700-320))
                        bScroll = FALSE;

                nFrameCounter += ((float)sTimer.vDelta)/67;
        }

        VerticalBlank();
        ResetPalette();
        ClearScreen();
        VerticalBlank();
}

