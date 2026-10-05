void AnimPlayUnpacked(int nNumber, int nFrameNums, int nSpeed, int nLoop, uint8 *pBackground)
{
        int i=0;
        uint8   *apAnim[60];
        float   nFrameNumber=0;

        for(i=0; i<nFrameNums; i++)
                apAnim[i] = pGraphics+file_offsets[nNumber+i];

        if(pBackground!=NULL)
                memcpy(pLayer1,pBackground,320*256);
        else
                memset(pLayer1,320*256,0);

        ILBM_Palette(apAnim[0],64,192,0);

        time_GetDelta();

        i=0;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                ShowFrame(pLayer1);
                DepackILBM(apAnim[(uint32)nFrameNumber],pLayer2,192);
                DrawLayerVert(pLayer2,0);

                ShowScreen();


                nFrameNumber += ((float)sTimer.vDelta)/nSpeed;

                if(nFrameNumber>=nFrameNums)
                {
                        if(i>=nLoop)
                        {
                                VerticalBlank();
                                ResetPalette();
                                ClearScreen();
                                VerticalBlank();
                                VerticalBlank();
                                VerticalBlank();
                                return;
                        }
                        nFrameNumber = 0;
                        i++;
                }
        }

        VerticalBlank();
        ResetPalette();
        ClearScreen();
}

