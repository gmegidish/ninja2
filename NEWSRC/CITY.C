
void City(void)
{
        int             i=0;
        bool            isScrolling;
        float           nFCount;

        nLayer1Ypos = 0;
        nLayer2Ypos = 0;
        nLayer3Ypos = 0;

        ILBM_Palette(pCityLayer1,64,0,-64);
        ILBM_Palette(pCityLayer2,64,64,-64);
        ILBM_Palette(pCityLayer3,64,128,-64);

        DepackILBM(pCityLayer1,pLayer3,0);
        DepackILBM(pCityLayer2,pLayer2,64);
        DepackILBM(pCityLayer3,pLayer1,128);

        DrawLayerVert(pLayer1,nLayer1Ypos);
        DrawLayerVert(pLayer2,nLayer2Ypos);
        DrawLayerVert(pLayer3,nLayer3Ypos);

        ShowScreen();

        isScrolling = TRUE;
        nFCount = 0;

        nLayer1Ypos = 0;
        nLayer2Ypos = 11;
        nLayer3Ypos = 0;

        time_GetDelta();

        while(GetKB()!=1)
        {
                VBlank();
                time_GetDelta();

                if(isScrolling)
                {
                        if(i<=64)
                        {
                                ILBM_Palette(pCityLayer1,64,0,-64+(i));
                                ILBM_Palette(pCityLayer2,64,64,-64+(i));
                                ILBM_Palette(pCityLayer3,64,128,-64+(i));
                        }

                        i++;

                        nLayer3Ypos += ((float)sTimer.vDelta)/72;               // 46
                        nLayer2Ypos += ((float)sTimer.vDelta)/101;              // 65
                        nLayer1Ypos += ((float)sTimer.vDelta)/303;

                        if(nLayer3Ypos >= 344)
                                isScrolling = FALSE;
                }

                DrawLayerVert(pLayer1,(uint32)nLayer1Ypos);
                DrawLayerVert(pLayer2,(uint32)nLayer2Ypos);
                DrawLayerVert(pLayer3,(uint32)nLayer3Ypos);

                ShowScreen();

                nFCount += ((float)sTimer.vDelta)/56;

                if(nFCount >= 482)
                {
                        DrawLayerVert(pLayer1,nLayer1Ypos);
                        DrawLayerVert(pLayer2,nLayer2Ypos);
                        DrawLayerVert(pLayer3,nLayer3Ypos);

                        memcpy(pVspace,sGraphics.pScreen,320*256);
                        AnimPlayUnpacked(aSEQ3_CHASE,45,205,0,pVspace);

                        memcpy(sGraphics.pScreen,pVspace,320*256);
                        ShowScreen();

                        nLayer1Ypos = 0;
                        nLayer2Ypos = 0;
                        nLayer3Ypos = 0;

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
