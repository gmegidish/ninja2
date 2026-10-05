void NinjaScrollUp(void)
{
        uint8   aFadeDown[35] = {
                0,0,0,5,14,44,44,14,5,2,5,14,44,44,14,10,5,2,1,0,0,0,0,0,0,0,0
        };

        float nFadePos=0;
        float   nFrameCounter=0;

        float   nHeadAnim=0;
        float   nBodyAnim=0;
        Sprite_t sSprite;
        bool    isScrolling = TRUE;

        VBlank();


        ILBM_Palette(pGraphics+file_offsets[gSEQ7_BACK],64,0,0);
        DepackILBM(pGraphics+file_offsets[gSEQ7_BACK],pLayer1,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ7_NINJA],64,64,0);
        DepackILBM(pGraphics+file_offsets[gSEQ7_NINJA],pLayer2,64);

        time_GetDelta();

        nLayer1Ypos = 544;
        nLayer2Ypos = 544;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                if(nFrameCounter>=135 & nFrameCounter<=162)
                {
                        if(nFadePos<=25)
                        {
                                ILBM_Palette(pGraphics+file_offsets[gSEQ7_BACK],64,0,aFadeDown[(int)(nFadePos)]);
                                ILBM_Palette(pGraphics+file_offsets[gSEQ7_NINJA],64,64,aFadeDown[(int)(nFadePos)]>>2);
                        }
                        nFadePos+=((float)sTimer.vDelta)/35;
                }

                if(nFrameCounter>=167)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }

                DrawLayerVert(pLayer1,(uint32)nLayer1Ypos);
                DrawLayerVert(pLayer2,(uint32)nLayer2Ypos);

                sSprite.screen          = sGraphics.pScreen;
                sSprite.vsprite         = pVspace;
                sSprite.xscreen1  = 0;
                sSprite.yscreen1        = 0;
                sSprite.xscreen2        = 320;
                sSprite.yscreen2        = 256;
                sSprite.modulo          = 320;
                sSprite.xpos                    = 0;
                sSprite.ypos                    = (uint32)-nLayer2Ypos;
                sSprite.xsize                   = 320;
                sSprite.ysize                   = 256;
                sSprite.xflip                   = 0;
                sSprite.yflip                   = 0;
                DepackILBM(pGraphics+file_offsets[aSEQ7_HEAD+(uint32)nHeadAnim],pVspace,64);
                Sprite_Draw((Sprite_t*)&sSprite);

                sSprite.screen          = sGraphics.pScreen;
                sSprite.vsprite         = pVspace;
                sSprite.xscreen1  = 0;
                sSprite.yscreen1        = 0;
                sSprite.xscreen2        = 320;
                sSprite.yscreen2        = 256;
                sSprite.modulo          = 320;
                sSprite.xpos                    = 67;
                sSprite.ypos                    = (uint32)(544-355)-nLayer2Ypos;
                sSprite.xsize                   = 320;
                sSprite.ysize                   = 256;
                sSprite.xflip                   = 0;
                sSprite.yflip                   = 0;
                DepackILBM(pGraphics+file_offsets[aSEQ7_BODY+(uint32)nBodyAnim],pVspace,64);
                Sprite_Draw((Sprite_t*)&sSprite);

                ShowScreen();

                nHeadAnim += ((float)sTimer.vDelta)/85;
                if(nHeadAnim>=8)
                        nHeadAnim = 0;

                nBodyAnim += ((float)sTimer.vDelta)/89;
                if(nBodyAnim>=6)
                        nBodyAnim = 0;

                if(isScrolling == TRUE)
                {
                        nLayer2Ypos -= ((float)sTimer.vDelta)/23;

                        if(nLayer2Ypos>=350)
                                nLayer1Ypos -= ((float)sTimer.vDelta)/23;
                        if(nLayer2Ypos<=350 & nLayer2Ypos>=300)
                                nLayer1Ypos -= ((float)sTimer.vDelta)/33;
                        if(nLayer2Ypos<=300)
                                nLayer1Ypos -= ((float)sTimer.vDelta)/38;

                        if(nLayer1Ypos<=0)
                        {
                                nLayer1Ypos = 0;
                        }

                        if(nLayer2Ypos<=60)
                        {
                                nLayer2Ypos = 60;
                                isScrolling = FALSE;
                        }
                }
                nFrameCounter += ((float)sTimer.vDelta)/72;
        }
}

