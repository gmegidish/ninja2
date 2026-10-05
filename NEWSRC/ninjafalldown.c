#define YPOS    (320*20)

void NinjaFallDown(void)
{
        uint32  nFogXSize;
        float           nFrameNumber=0;
        float           nFCount=0;
        int                     nCount=0;
        bool            isLooping;
        bool            isShaking;
        float           nShake;
        uint32  aYshake[14] = {
                0,-(320*10),+(320*10),-(320*7),+(320*7),-(320*5),+(320*5),
                -(320*3),+(320*3),-(320*2),+(320*2),-320,+320,0
        };

        memset(pLayer1,0,320*286);
        memset(pLayer2,0,320*286);
        memset(pLayer3,0,320*286);
        memset(pVspace,0,320*286);

        DepackILBM(pGraphics+file_offsets[gSEQ9_LAYER1],pLayer1+YPOS,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER1],64,0,0);
        DepackILBM(pGraphics+file_offsets[gSEQ9_FOG],pLayer2,0);
        DepackILBM(pGraphics+file_offsets[gSEQ9_LAYER2],pLayer3+YPOS,64);
        ILBM_Palette(pGraphics+file_offsets[gSEQ9_LAYER2],64,64,0);
        ILBM_Palette(pGraphics+file_offsets[aSEQ12_NINJA],64,128,0);

        nFogXSize = ILBM_GetXSize(pGraphics+file_offsets[gSEQ9_FOG]);

        nFrameNumber = 0;

        nLayer3Xpos = 70;

        time_GetDelta();

        isLooping = FALSE;
        isShaking = TRUE;

        nShake = 0;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                DrawLayerVert(pLayer1+aYshake[(uint32)nShake]+YPOS,0);
                DrawFog(pLayer2,(uint32)nLayer3Xpos,nFogXSize,0);
                DrawLayerVert(pLayer3+aYshake[(uint32)nShake]+YPOS,0);
                DepackILBM(pGraphics+file_offsets[aSEQ12_NINJA+(uint32)nFrameNumber],pVspace+YPOS,128);
                DrawLayerVert(pVspace+aYshake[(uint32)nShake]+YPOS,0);

                ShowScreen();

                nLayer3Xpos += ((float)sTimer.vDelta)/90;

                if(nFCount>=90)
                        nFrameNumber += ((float)sTimer.vDelta)/120;

                if(nFrameNumber>=31)
                        isLooping = TRUE;

                if(isShaking)
                {
                        if(nFrameNumber>=11)
                        {
                                nShake += ((float)sTimer.vDelta)/70;

                                if(nShake>=14)
                                {
                                        isShaking = FALSE;
                                        nShake = 0;
                                }
                        }
                }

                if(isLooping)
                {
                        if(nFrameNumber>=35)
                        {
                                nFrameNumber=31;
                                nCount++;
                        }

                        if(nCount>=6)
                        {
                                VerticalBlank();
                                ResetPalette();
                                ClearScreen();
                                VerticalBlank();
                                return;
                        }

                }

/*
                if(nFrameNumber>=35)
                {
                        nFrameNumber = 0;
                        nFCount = 0;
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }
 */

                nFCount += ((float)sTimer.vDelta)/35;
        }
}

