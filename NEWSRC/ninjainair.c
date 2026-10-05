#define NUMMBLURR 16

void NinjaInAir(void)
{
        int             nCount=0;
        int             nCount2=0;
        bool    isMBlurr=TRUE;
        float   nFrameNumber=0;

        memset(pLayer1,0,320*1024);
        memset(pLayer3,0,320*1024);

        DepackILBM(pGraphics+file_offsets[gSEQ11_MBLURR],pLayer1+(320*512),0);
        DepackILBM(pGraphics+file_offsets[gSEQ11_MEND],pLayer2,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ11_MBLURR],64,0,0);
        memcpy(pLayer1+(320*768),pLayer1+(320*512),320*256);
        memcpy(pLayer1+(320*323),pLayer2,320*189);
        DepackILBM(pGraphics+file_offsets[gSEQ11_CLOUDS],pLayer2,64);
        ILBM_Palette(pGraphics+file_offsets[gSEQ11_CLOUDS],64,64,0);
        ILBM_Palette(pGraphics+file_offsets[aSEQ11_NINJA],64,128,0);

        nLayer1Ypos = 768;
        nLayer2Ypos = 64;

        isMBlurr = TRUE;
        nCount = 0;
        nCount2= 0;

        time_GetDelta();

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                DepackILBM(pGraphics+file_offsets[aSEQ11_NINJA+(uint32)nFrameNumber],pLayer3+(320*60),128);

                if(nCount2<=10)
                {
                        if(nFrameNumber>=3)
                        {
                                nFrameNumber=0;
                                nCount2++;
                        }
                }

                if(nCount2<=15)
                {
                        if(nFrameNumber>=29)
                        {
                                nFrameNumber = 24;
                                nCount2++;
                        }
                }

                if(nCount2>=15)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }

                DrawLayerVert(pLayer2,(uint32)nLayer2Ypos);

                if(isMBlurr)
                        DrawLayerVert(pLayer1,(uint32)nLayer1Ypos);

                DrawLayerVert(pLayer3,(uint32)nLayer3Ypos);

                if(nCount>=NUMMBLURR)
                {
                        nLayer2Ypos -= ((float)sTimer.vDelta)/80;
                }

                if(nCount2>=13)
                        nLayer3Ypos -= ((float)sTimer.vDelta)/30;

                if(isMBlurr)
                {
                        if(nCount==NUMMBLURR)
                                nLayer1Ypos=256;

                        if(nCount>=NUMMBLURR)
                        {
                                nLayer1Ypos -= ((float)sTimer.vDelta)/1.5;

                                if(nLayer1Ypos<=80)
                                        isMBlurr = FALSE;
                        }

                        if(nCount<=4)
                                nLayer3Ypos += ((float)sTimer.vDelta)/20;

                        if(nCount<=NUMMBLURR)
                        {
                                switch(nCount)
                                {
                                        case 0:nLayer1Ypos -= ((float)sTimer.vDelta)/2;
                                        case 1:nLayer1Ypos -= ((float)sTimer.vDelta)/1.9;
                                        case 2:nLayer1Ypos -= ((float)sTimer.vDelta)/1.8;
                                        case 3:nLayer1Ypos -= ((float)sTimer.vDelta)/1.6;
                                        default:nLayer1Ypos -= ((float)sTimer.vDelta)/1.6;
                                }

                                if(nLayer1Ypos<=512)
                                {
                                        nLayer1Ypos = 768;
                                        nCount++;
                                }
                        }
                }

                nFrameNumber += ((float)sTimer.vDelta)/130;

                ShowScreen();
        }
}

