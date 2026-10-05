void NinjaJumpUp(void)
{
        uint32 nFogXSize;
        float   nFrameNumber=0;
        int             nFCount=0;

        DepackILBM(pGraphics+file_offsets[gSEQ9_LAYER1],pLayer1,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER1],64,0,0);
        DepackILBM(pGraphics+file_offsets[gSEQ9_FOG],pLayer2,0);
        DepackILBM(pGraphics+file_offsets[gSEQ9_LAYER2],pLayer3,64);
        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER2],64,64,0);
        ILBM_Palette(pGraphics+file_offsets[aSEQ9_NINJA],64,128,0);

        nFogXSize = ILBM_GetXSize(pGraphics+file_offsets[gSEQ9_FOG]);

        time_GetDelta();

        nFrameNumber = 0;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                DrawLayerVert(pLayer1,0);

                DrawFog(pLayer2,(uint32)nLayer3Xpos,nFogXSize,0);
                DrawLayerVert(pLayer3,0);
                DepackILBM(pGraphics+file_offsets[aSEQ9_NINJA+(uint32)nFrameNumber],pVspace,128);
                DrawLayerVert(pVspace,0);

                ShowScreen();

                nLayer3Xpos += ((float)sTimer.vDelta)/90;

                nFrameNumber += ((float)sTimer.vDelta)/95;

                if(nFCount<2)
                {
                        if(nFrameNumber>=4)
                        {
                                nFrameNumber = 0;
                                nFCount++;
                        }
                }
                else
                {
                        if(nFrameNumber>=18)
                        {
                                nFrameNumber = 0;
                                nFCount = 0;
                                VerticalBlank();
                                ResetPalette();
                                ClearScreen();
                                VerticalBlank();
                                return;
                        }
                }
        }
}

