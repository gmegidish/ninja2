
static unsigned
ILBM_GetXSize (p)
       unsigned char *p;
{
       if (!p) {
               return 0;
       }

       return ((p[0x14] << 8) | p[0x15]);
}

static unsigned
ILBM_GetYSize (p)
       unsigned char *p;
{
       if (!p) {
               return 0;
       }

       return ((p[0x16] << 8) | p[0x17]);
}

static unsigned char *
chunk_search (ilbm, name, size)
       unsigned char *ilbm;
       unsigned long name;
       unsigned *size;
{
       unsigned char *p;
       unsigned offset;

       if (!ilbm || !name) {
               return 0;
       }

       /* fixme: crappy algorithm */
       p = ilbm;
       while (1) {
               if (*(int*)p == name) {
                       break;
               }

               p++;
       }

       if (size) {
               *size = ((p[6] << 8) | (p[7]));
       }

       return (p + 8);
}

static void
ILBM_Palette (ilbm, numcols, coloff, nfade)
       unsigned char *ilbm;
       unsigned numcols;
       unsigned char coloff;
       int nfade;
{
       unsigned p;
       int r, g, b;
       unsigned char *cmap;
       SDL_Color color[256];

       if (!ilbm || numcols > 256) {
               printf ("ILBM_Palette invalid args\n");
               return;
       }

       cmap = chunk_search (ilbm, 'PAMC', NULL);
       if (!cmap) {
               printf ("ILBM_Palette failed!\n");
               return;
       }

       nfade = nfade << 2;

       #define LIMIT(min,cur,max) \
       if ((cur) < (min)) { \
               (cur) = (min); \
       } else if ((cur) > (max)) { \
               (cur) = (max); \
       }

       /* XXX fade_palette not implemented */
       for (p=0; p<numcols; p++) {
                r = (int)(*cmap++) + nfade;
                g = (int)(*cmap++) + nfade;
                b = (int)(*cmap++) + nfade;

                LIMIT(0, r, 255);
                LIMIT(0, g, 255);
                LIMIT(0, b, 255);

                color[p].r = r;
                color[p].g = g;
                color[p].b = b;
       }

       SDL_SetColors (screen, color, coloff, numcols);
}

static void
DepackILBM (src, dst, colstart)
       unsigned char *src;
       unsigned char *dst;
       int colstart;
{
       register unsigned char *body;
       register int op, col;
       int width, height;
       int frame_size;
       int chunk_size;

       width = ILBM_GetXSize (src);
       height = ILBM_GetYSize (src);
       frame_size = width * height;

       body = chunk_search (src, 'YDOB', &chunk_size);
       if (!body) {
               printf ("no body\n");
               return;
       }

       while (frame_size > 0) {
               op = (*body++) & 0xff;
               if (op > 128) {
                       /* replicated */
                       op = (256 - op) + 1;
                       frame_size = frame_size - op;
                       if (frame_size < 0) {
                               break;
                       }

                       col = *body++;
                       if (col != 0) {
                               col = col + colstart;
                       }

                       memset ((void*)dst, col, op);
                       dst = dst + op;
               } else if (op != 128) {
                       /* opaque */
                       op++;
                       frame_size = frame_size - op;
                       if (frame_size < 0) {
                               break;
                       }

                       while (op > 0) {
                               col = *body++;
                               if (col) {
                                       *dst++ = col + colstart;
                               } else {
                                       dst++;
                               }

                               op--;
                       }
               }
       }
}

static void
DrawLayerVert (uint8 *pLayer, uint32 nYpos)
{
       register uint8 *src, *dst;
       register uint8 col;
       register int x, y;

       if (!pLayer) {
               return;
       }

       src = pLayer + (nYpos*320);
       dst = (uint8*)screen->pixels;
       for (y=0; y<256; y++) {
               for (x=0; x<320; x++) {
                       col = *src++;
                       if (col) *dst = col;
                       dst++;
               }
       }
}

static void
DrawLayerHor (uint8 *pLayer, uint32 nXpos, uint32 nXsize, uint32 nYpos)
{
       register uint8 col;
       register int x, y;
       register uint8 *src, *dst;

       if (!pLayer) {
               return;
       }

       for (y=0; y<256; y++) {

               src = pLayer + (y*nXsize) + nXpos;
               dst = (uint8*)screen->pixels + ((y+nYpos)*320); // + nXpos;

               for (x=0; x<80; x++) {
                       int x2;

                       if (*(int*)src == 0) {
                           src+=4;
                           dst+=4;
                           continue;
                       }

                       col = src[0];
                       if (col) dst[0] = col;

                       col = src[1];
                       if (col) dst[1] = col;

                       col = src[2];
                       if (col) dst[2] = col;

                       col = src[3];
                       if (col) dst[3] = col;

                       src += 4;
                       dst += 4;
               }
       }
}

static void
DrawFog (uint8 *pLayer, uint32 nXpos, uint32 nXsize, uint32 nYpos)
{
       register uint8 col;
       register int x, y;
       register uint8 *src, *dst;

       if (!pLayer) {
               return;
       }

       for (y=0; y<256; y++) {

               src = pLayer + (y*nXsize) + nXpos;
               dst = (uint8*)screen->pixels + ((y+nYpos)*320); // + nXpos;

               for (x=0; x<320; x++) {
                       col = *src++;
                       if (col) {
                               *dst++ = col + 16;
                       } else {
                               dst++;
                       }
               }
       }
}

static void
Sprite_Draw (Sprite_t *sprite)
{
       register uint8 *src, *dst;
       register int x, y;
       int x_to_copy, y_to_copy;
       int x_start, y_start;

       if (!sprite) {
               return;
       }

       if (sprite->xscreen1 || sprite->yscreen1 ||
           sprite->xflip || sprite->yflip ||
           sprite->modulo != 320) {
           printf ("<< SPRITE ERROR! >>\n");
           return;
       }

       //printf ("sprite->xpos %d sprite->ypos %d\n", sprite->xpos, sprite->ypos);
       //printf ("sprite->xsize %d sprite->ysize %d\n", sprite->xsize, sprite->ysize);

       if (!sprite->screen || !sprite->vsprite) {
               return;
       }

       if (sprite->xpos + sprite->xsize > sprite->xscreen2) {
               x_to_copy = sprite->xscreen2 - sprite->xpos;
               if (x_to_copy <= 0) {
                       return;
               }
       } else {
               x_to_copy = sprite->xsize;
       }

       if (sprite->ypos + sprite->ysize > sprite->yscreen2) {
               y_to_copy = sprite->yscreen2 - sprite->ypos;
               if (y_to_copy <= 0) {
                       return;
               }
       } else {
               y_to_copy = sprite->ysize;
       }

       //printf ("sprite->ypos %d\n", sprite->ypos);
       if (sprite->ypos < 0 || sprite->xpos < 0) return;

       x_start = 0;
       y_start = 0;

       for (y=0; y<y_to_copy; y++) {
               src = sprite->vsprite + ((y+y_start)*sprite->xsize) + x_start;
               dst = sprite->screen + ((y+sprite->ypos)*sprite->modulo) + sprite->xpos;

               for (x=0; x<x_to_copy; x++) {
                       if (*src) {
                               *dst++ = *src++;
                       } else {
                               src++;
                               dst++;
                       }
               }
       }
}

