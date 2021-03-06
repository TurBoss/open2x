#ifndef __DRAW
#define __DRAW 1

#ifdef BUILD_X11

char
__imlib_CreatePixmapsForImage(Display *d, Drawable w, Visual *v, int depth,
			      Colormap cm, ImlibImage *im, Pixmap *p, Mask *m,
			      int sx, int sy, int sw, int sh,
			      int dw, int dh,
			      char anitalias, char hiq, char dither_mask,
			      ImlibColorModifier *cmod);

#endif

#endif
