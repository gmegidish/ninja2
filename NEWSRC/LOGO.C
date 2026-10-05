#define ZOOM            8000

static void
BitmapZoom (Zoom_t *s)
{
}

void Logo(void)
{
        Zoom_t sZoom;
        float           nXzoom;
        float           nYzoom;
        bool            isZooming;
        bool            isActive;
        bool            isAnimating;
        float           vFrameNumber;
        float           vFadeValue=0.0;

        memset(pLayer1,0,320*400);
        memset(pLayer2,0,320*400);
        memset(pLayer3,0,320*400);

        DepackILBM(pGraphics+file_offsets[gSEQ2_LOGO],pLayer1,0);
        ILBM_Palette(pGraphics+file_offsets[gSEQ2_LOGO],64,0,0);

        nXzoom                          = ZOOM;
        nYzoom                          = ZOOM;
        isZooming               = TRUE;
        isActive                        = TRUE;
        isAnimating     = FALSE;
        vFrameNumber    = 0;

        time_GetDelta();

        while(isActive & (GetKB()!=2) & (nXzoom>=1))
        {
                VBlank();
                time_GetDelta();

                if(nXzoom<=120)
                {
                        ILBM_Palette(pGraphics+file_offsets[gSEQ2_LOGO],64,0,-(uint32)vFadeValue);
                        vFadeValue += ((float)sTimer.vDelta)/60;
                        if(vFadeValue>=64)
                                isActive = FALSE;
                }

                if(isZooming)
                {
                        memset(sGraphics.pScreen,0,320*256);

                        if(nXzoom>=320)
                        {
                                sZoom.nXpos     = ((uint32)(nXzoom) - 320)>>1;
                                sZoom.nSXpos    = 0;
                        }
                        else
                        {
                                sZoom.nXpos     = 0;
                                sZoom.nSXpos  = 160 - ((uint32)(nXzoom)>>1);
                        }

                        if(nYzoom>=256)
                        {
                                sZoom.nYpos     = ((uint32)(nYzoom) - 256)>>1;
                                sZoom.nSYpos    = 0;
                        }
                        else
                        {
                                sZoom.nYpos     = 0;
                                sZoom.nSYpos  = 128 - ((uint32)(nYzoom)>>1);
                        }

                        sZoom.nScaleX           = (uint32)nXzoom;
                        sZoom.nScaleY           = (uint32)nYzoom;
                        sZoom.pGraphics = pLayer3;

                        if(isAnimating)
                                BitmapZoom((Zoom_t*)&sZoom);
                        sZoom.pGraphics = pLayer1;
                        BitmapZoom((Zoom_t*)&sZoom);

                        if(nXzoom>=2560)
                        {
                                nXzoom -= ((float)sTimer.vDelta)*6;
                                nYzoom -= ((float)sTimer.vDelta)*6;
                        }

                        if(nXzoom>=1280 & nXzoom<=2559)
                        {
                                nXzoom -= ((float)sTimer.vDelta)*3;
                                nYzoom -= ((float)sTimer.vDelta)*3;
                        }

                        if(nXzoom<=1279 & nXzoom>=320)
                        {
                                nXzoom -= ((float)sTimer.vDelta)*2;
                                nYzoom -= ((float)sTimer.vDelta)*2;
                        }

                        if(nXzoom<=319)
                        {
                                nXzoom -= ((float)sTimer.vDelta)/20;
                                nYzoom -= ((float)sTimer.vDelta)/20;
                                isAnimating = TRUE;
                        }

                        if(isAnimating)
                        {
                                DepackILBM(pGraphics+file_offsets[aSEQ2_LOGO+(uint32)vFrameNumber],pLayer3,20);

                                vFrameNumber += ((float)sTimer.vDelta)/125;

                                if(vFrameNumber>=8)
                                        vFrameNumber = 7;
                        }

                        if(nXzoom<=1)
                                isZooming = FALSE;
                        if(nYzoom<=1)
                                isZooming = FALSE;
                }

                //MotionBlurr(pLayer2);

                memcpy(pLayer2,sGraphics.pScreen,320*256);

                ShowScreen();
        }
}
