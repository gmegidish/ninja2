void NinjaIntoFog(void)
{
        uint32 nFogXSize;
        float   nFade=0;
        float   nFrameNumber=0;
        int             nFCount=0;
        bool    isAnimating;
        bool    isGone;

        DepackILBM(pGraphics+file_offsets[gSEQ9_LAYER1],pLayer1,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER1],64,0,0);
        DepackILBM(pGraphics+file_offsets[gSEQ9_FOG],pLayer2,0);
        DepackILBM(pGraphics+file_offsets[gSEQ22_LAYER1],pLayer3,64);
        ILBM_Palette(pGraphics+file_offsets[gSEQ22_LAYER1],64,64,0);
        ILBM_Palette(pGraphics+file_offsets[aSEQ22_NINJA],64,128,0);

        DepackILBM(pGraphics+file_offsets[aSEQ22_NINJA],pVspace,128);

        nFogXSize = ILBM_GetXSize(pGraphics+file_offsets[gSEQ9_FOG]);

        time_GetDelta();

        nFrameNumber = 0;

        nLayer3Xpos  = 70;

        time_GetDelta();

        isAnimating = FALSE;
        isGone = FALSE;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                ShowFrame(pLayer1);

                DrawFog(pLayer2,(uint32)nLayer3Xpos,nFogXSize,0);
                DrawLayerVert(pLayer3,0);

                if(isAnimating)
                        DepackILBM(pGraphics+file_offsets[aSEQ22_NINJA+(uint32)nFrameNumber],pVspace,128);

                if(isGone==FALSE)
                        DrawLayerVert(pVspace,0);

                if(nLayer3Xpos >= 250)
                {
                        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER1],64,0,-(int8)nFade);
                        ILBM_Palette(pGraphics+file_offsets[gSEQ22_LAYER1],64,64,-(int8)nFade);
                        ILBM_Palette(pGraphics+file_offsets[aSEQ22_NINJA],64,128,-(int8)nFade);

                        nFade += ((float)sTimer.vDelta)/60;

                        if(nFade>=64)
                        {
                                VerticalBlank();
                                ResetPalette();
                                ClearScreen();
                                VerticalBlank();
                                return;
                        }
                }

                ShowScreen();

                nLayer3Xpos += ((float)sTimer.vDelta)/90;

                if(nLayer3Xpos>=155 & nLayer3Xpos<=160)
                        isAnimating = TRUE;

                if(isAnimating)
                        nFrameNumber += ((float)sTimer.vDelta)/145;

                if(nFrameNumber>=27)
                {
                        isAnimating = FALSE;
                        isGone = TRUE;
                }
        }

        VerticalBlank();
        ResetPalette();
        ClearScreen();
        VerticalBlank();
}

