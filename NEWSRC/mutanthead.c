void MutantHead(void)
{
        float   nFrameNumber=0;
        uint32 nLayer1XSize;

        memset(pLayer1,0,500*400);

        DepackILBM(pGraphics+file_offsets[gSEQ21_HEAD],pLayer1+(500*140),0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ21_HEAD],64,0,0);
        ILBM_Palette(pGraphics+file_offsets[aSEQ21_GUSH],64,64,0);

        time_GetDelta();

        nLayer1XSize = ILBM_GetXSize(pGraphics+file_offsets[gSEQ21_HEAD]);

        nLayer1Xpos = 40;
        nLayer1Ypos = 100;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                DepackILBM(pGraphics+file_offsets[aSEQ21_GUSH+(uint32)nFrameNumber],pLayer2,64);
                ShowFrame(pLayer2);
                DrawLayerHor(pLayer1,(((uint32)nLayer1Ypos)*nLayer1XSize)+(uint32)nLayer1Xpos,nLayer1XSize,0);

                ShowScreen();

                nLayer1Xpos += ((float)sTimer.vDelta)/90;
                nLayer1Ypos -= ((float)sTimer.vDelta)/90;

                if(nLayer1Ypos<=20)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }

                nFrameNumber += ((float)sTimer.vDelta)/245;

                if(nFrameNumber>=4)
                        nFrameNumber = 0;
        }

        VerticalBlank();
        ResetPalette();
        ClearScreen();
        VerticalBlank();
}

