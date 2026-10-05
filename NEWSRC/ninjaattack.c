void NinjaAttack(void)
{
        //memset(pLayer2,0,320*30);

        DepackILBM(pGraphics+file_offsets[gSEQ11_MBLURR],pLayer1,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ11_MBLURR],64,0,0);
        DepackILBM(pGraphics+file_offsets[gSEQ18_ATTACK],pLayer2,64);
        ILBM_Palette(pGraphics+file_offsets[gSEQ18_ATTACK],64,64,0);
        memcpy(pLayer1+(320*256),pLayer1,320*256);

        time_GetDelta();

        nLayer1Ypos = 256;
        nLayer2Ypos = 0;

        while(GetKB()!=2)
        {
                VBlank();
                time_GetDelta();

                DrawLayerVert(pLayer1,nLayer1Ypos);
                DrawLayerVert(pLayer2,nLayer2Ypos);

                nLayer1Ypos -= ((float)sTimer.vDelta)/2;
                nLayer2Ypos += ((float)sTimer.vDelta)/60;

                if(nLayer2Ypos>=40)
                {
                        VerticalBlank();
                        ResetPalette();
                        ClearScreen();
                        VerticalBlank();
                        return;
                }

                if(nLayer1Ypos<=0)
                        nLayer1Ypos = 256;

                ShowScreen();
        }

}

