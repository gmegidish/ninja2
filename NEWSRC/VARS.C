
float   nLayer1Xpos;                                                                                                    /* Layer 1 X position */
float   nLayer2Xpos;                          /* Layer 2 X position */
float   nLayer3Xpos;                          /* Layer 3 X position */
float   nLayer1Ypos;                          /* Layer 1 Y position */
float   nLayer2Ypos;                          /* Layer 2 Y position */
float   nLayer3Ypos;                          /* Layer 3 Y position */

uint8 *pLayer1;                               /* Pointer to depack layer area 1 */
uint8 *pLayer2;                               /* Pointer to depack layer area 2 */
uint8 *pLayer3;                               /* Pointer to depack layer area 3 */

SDL_Surface *pLayer[3];

uint8 *pIntroLayer1;
uint8 *pIntroLayer2;
uint8 *pIntroLayer3;
uint8 *pCityLayer1;
uint8 *pCityLayer2;
uint8 *pCityLayer3;

uint8 *pGraphics;                                                                                                                       /* All graphics */
uint8 *pVspace;                                                                                                                         /* Virtual depack space */

Graphics_t sGraphics;
Timer_t          sTimer;
