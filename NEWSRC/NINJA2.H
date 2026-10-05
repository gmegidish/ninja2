
#ifndef __ninja2__
#define __ninja2__

#define TRUE 1
#define FALSE 0

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef signed long int32;
typedef signed short int16;
typedef signed char int8;
typedef int bool;

typedef struct Sprite_s
{
        uint8*  screen;                                                         /* Pointer to Screen_t */
        uint8*  sprite;                                                         /* Pointer to SPRITE */
        int             xscreen1;                                                       /* X clip 1 */
        int                     yscreen1;                                                       /* X clip 2 */
        int           xscreen2;                                                       /* X size of screen */
        int                     yscreen2;                                                       /* Y size of screen */
        int                     modulo;                                                         /* Modulo of real screen */
        int32  xpos;                                                                   /* X position of sprite */
        int32  ypos;                                                                   /* Y position of sprite */
        uint8*  vsprite;                                                        /* Virtual Depack Area Of Sprite */
        int                     xflip;                                                          /* Flag if X flipped */
        int                     yflip;                                                          /* Flag if Y flipped */
        int                     xsize;                                                          /* X size of sprite */
        int                     ysize;                                                          /* Y size of sprite */
} Sprite_t;

typedef struct Graphics_s
{
        uint8 *pScreen;
}Graphics_t;

/* Timer Object */
typedef struct Timer_s
{
        uint32  vPrevious;
        uint32  vCurrent;
        uint32  vDelta;
        uint32  vPreviousDelta;
}Timer_t;

typedef struct Zoom_s
{
        uint8   *pGraphics;                                             /* Pointer to graphics to zoom... */
        uint32  nXpos;                                                          /* X position inside bitmap */
        uint32  nYpos;                                                          /* Y position inside bitmap */
        uint32  nScaleX;                                                        /* X Scale value according to xsize, which
                                                                                                                                         should be 320 */
        uint32  nScaleY;                                                        /* Y Scale value according to ysize, which
                                                                 should be 256 */
        uint32  nSXpos;                                                         /* X position on screen */
        uint32  nSYpos;                                                         /* Y position on screen */
}Zoom_t;


#endif