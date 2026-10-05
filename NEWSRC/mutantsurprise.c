void MutantSurprise(void)
{
        int     anFrames[] = {
                0,0,0,1,2,3,3,3,3,3,3,4,5,6,7,8,8,8,8,8,8,8,8,8,8,8
        };

        int i=0;
        uint8   *apMutantAnim[10];
        float   nFrameNumber=0;

        for(i=0; i<9; i++)
                apMutantAnim[i] = pGraphics+file_offsets[aANGRY+i];

        DepackILBM(pGraphics+file_offsets[gANGRY],pLayer1,0);

        ILBM_Palette(apMutantAnim[0],64,64,0);
        ILBM_Palette(pGraphics+file_offsets[gANGRY],64,0,0);

        time_GetDelta();

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                ShowFrame(pLayer1);
                DepackILBM(apMutantAnim[anFrames[(uint32)nFrameNumber]],pLayer2,64);
                DrawLayerVert(pLayer2,0);

                ShowScreen();

                nFrameNumber += ((float)sTimer.vDelta)/135;

                if(nFrameNumber>=23)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }
        }

        VerticalBlank();
        ResetPalette();
        ClearScreen();
}

