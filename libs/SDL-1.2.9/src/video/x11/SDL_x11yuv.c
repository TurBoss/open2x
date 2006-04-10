/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_x11yuv.c,v 1.16 2004/08/25 05:39:03 slouken Exp $";
#endif

/* This is the XFree86 Xv extension implementation of YUV video overlays */

#ifdef XFREE86_XV

#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#ifndef NO_SHARED_MEMORY
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#include <XFree86/extensions/Xvlib.h>

#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_x11yuv_c.h"
#include "SDL_yuvfuncs.h"

#define XFREE86_REFRESH_HACK
#ifdef XFREE86_REFRESH_HACK
#include "SDL_x11image_c.h"
#endif

/* Workaround when pitch != width */
#define PITCH_WORKAROUND

/* Fix for the NVidia GeForce 2 - use the last available adaptor */
/*#define USE_LAST_ADAPTOR*/  /* Apparently the NVidia drivers are fixed */

/* The functions used to manipulate software video overlays */
static struct private_yuvhwfuncs x11_yuvfuncs = {
	X11_LockYUVOverlay,
	X11_UnlockYUVOverlay,
	X11_DisplayYUVOverlay,
	X11_FreeYUVOverlay
};

struct private_yuvhwdata {
	int port;
#ifndef NO_SHARED_MEMORY
	int yuv_use_mitshm;
	XShmSegmentInfo yuvshm;
#endif
	SDL_NAME(XvImage) *image;
};


static int (*X_handler)(Display *, XErrorEvent *) = NULL;

#ifndef NO_SHARED_MEMORY
/* Shared memory error handler routine */
static int shm_error;
static int shm_errhandler(Display *d, XErrorEvent *e)
{
        if ( e->error_code == BadAccess ) {
        	shm_error = True;
        	return(0);
        } else
		return(X_handler(d,e));
}
#endif /* !NO_SHARED_MEMORY */

static int xv_error;
static int xv_errhandler(Display *d, XErrorEvent *e)
{
        if ( e->error_code == BadMatch ) {
        	xv_error = True;
        	return(0);
        } else
		return(X_handler(d,e));
}

SDL_Overlay *X11_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display)
{
	SDL_Overlay *overlay;
	struct private_yuvhwdata *hwdata;
	int xv_port;
	int i, j, k;
	int adaptors;
	SDL_NAME(XvAdaptorInfo) *ainfo;
	int bpp;
#ifndef NO_SHARED_MEMORY
	XShmSegmentInfo *yuvshm;
#endif

	/* Look for the XVideo extension with a valid port for this format */
	xv_port = -1;
	if ( (Success == SDL_NAME(XvQueryExtension)(GFX_Display, &j, &j, &j, &j, &j)) &&
	     (Success == SDL_NAME(XvQueryAdaptors)(GFX_Display,
	                                 RootWindow(GFX_Display, SDL_Screen),
	                                 &adaptors, &ainfo)) ) {
#ifdef USE_LAST_ADAPTOR
		for ( i=0; i < adaptors; ++i )
#else
		for ( i=0; (i < adaptors) && (xv_port == -1); ++i )
#endif /* USE_LAST_ADAPTOR */
		{
			/* Check to see if the visual can be used */
			if ( BUGGY_XFREE86(<=, 4001) ) {
				int visual_ok = 0;
				for ( j=0; j<ainfo[i].num_formats; ++j ) {
					if ( ainfo[i].formats[j].visual_id ==
							SDL_Visual->visualid ) {
						visual_ok = 1;
						break;
					}
				}
				if ( ! visual_ok ) {
					continue;
				}
			}
			if ( (ainfo[i].type & XvInputMask) &&
			     (ainfo[i].type & XvImageMask) ) {
				int num_formats;
				SDL_NAME(XvImageFormatValues) *formats;
				formats = SDL_NAME(XvListImageFormats)(GFX_Display,
				              ainfo[i].base_id, &num_formats);
#ifdef USE_LAST_ADAPTOR
				for ( j=0; j < num_formats; ++j )
#else
				for ( j=0; (j < num_formats) && (xv_port == -1); ++j )
#endif /* USE_LAST_ADAPTOR */
				{
					if ( (Uint32)formats[j].id == format ) {
						for ( k=0; k < ainfo[i].num_ports; ++k ) {
							if ( Success == SDL_NAME(XvGrabPort)(GFX_Display, ainfo[i].base_id+k, CurrentTime) ) {
								xv_port = ainfo[i].base_id+k;
								break;
							}
						}
					}
				}
				if ( formats ) {
					XFree(formats);
				}
			}
		}
		SDL_NAME(XvFreeAdaptorInfo)(ainfo);
	}

	/* Precalculate the bpp for the pitch workaround below */
	switch (format) {
	    /* Add any other cases we need to support... */
	    case SDL_YUY2_OVERLAY:
	    case SDL_UYVY_OVERLAY:
	    case SDL_YVYU_OVERLAY:
		bpp = 2;
		break;
	    default:
		bpp = 1;
		break;
	}

#if 0
    /*
     * !!! FIXME:
     * "Here are some diffs for X11 and yuv.  Note that the last part 2nd
     *  diff should probably be a new call to XvQueryAdaptorFree with ainfo
     *  and the number of adaptors, instead of the loop through like I did."
     *
     *  ACHTUNG: This is broken! It looks like XvFreeAdaptorInfo does this
     *  for you, so we end up with a double-free. I need to look at this
     *  more closely...  --ryan.
     */
 	for ( i=0; i < adaptors; ++i ) {
 	  if (ainfo[i].name != NULL) Xfree(ainfo[i].name);
 	  if (ainfo[i].formats != NULL) Xfree(ainfo[i].formats);
   	}
 	Xfree(ainfo);
#endif

	if ( xv_port == -1 ) {
		SDL_SetError("No available video ports for requested format");
		return(NULL);
	}

	/* Enable auto-painting of the overlay colorkey */
	{
		static const char *attr[] = { "XV_AUTOPAINT_COLORKEY", "XV_AUTOPAINT_COLOURKEY" };
		unsigned int i;

		SDL_NAME(XvSelectPortNotify)(GFX_Display, xv_port, True);
		X_handler = XSetErrorHandler(xv_errhandler);
		for ( i=0; i < sizeof(attr)/(sizeof attr[0]); ++i ) {
			Atom a;

			xv_error = False;
			a = XInternAtom(GFX_Display, attr[i], True);
			if ( a != None ) {
     				SDL_NAME(XvSetPortAttribute)(GFX_Display, xv_port, a, 1);
				XSync(GFX_Display, True);
				if ( ! xv_error ) {
					break;
				}
			}
		}
		XSetErrorHandler(X_handler);
		SDL_NAME(XvSelectPortNotify)(GFX_Display, xv_port, False);
	}

	/* Create the overlay structure */
	overlay = (SDL_Overlay *)malloc(sizeof *overlay);
	if ( overlay == NULL ) {
		SDL_NAME(XvUngrabPort)(GFX_Display, xv_port, CurrentTime);
		SDL_OutOfMemory();
		return(NULL);
	}
	memset(overlay, 0, (sizeof *overlay));

	/* Fill in the basic members */
	overlay->format = format;
	overlay->w = width;
	overlay->h = height;

	/* Set up the YUV surface function structure */
	overlay->hwfuncs = &x11_yuvfuncs;
	overlay->hw_overlay = 1;

	/* Create the pixel data and lookup tables */
	hwdata = (struct private_yuvhwdata *)malloc(sizeof *hwdata);
	overlay->hwdata = hwdata;
	if ( hwdata == NULL ) {
		SDL_NAME(XvUngrabPort)(GFX_Display, xv_port, CurrentTime);
		SDL_OutOfMemory();
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}
	hwdata->port = xv_port;
#ifndef NO_SHARED_MEMORY
	yuvshm = &hwdata->yuvshm;
	memset(yuvshm, 0, sizeof(*yuvshm));
	hwdata->image = SDL_NAME(XvShmCreateImage)(GFX_Display, xv_port, format,
						   0, width, height, yuvshm);
#ifdef PITCH_WORKAROUND
	if ( hwdata->image != NULL && hwdata->image->pitches[0] != (width*bpp) ) {
		/* Ajust overlay width according to pitch */ 
		XFree(hwdata->image);
		width = hwdata->image->pitches[0] / bpp;
		hwdata->image = SDL_NAME(XvShmCreateImage)(GFX_Display, xv_port, format,
							   0, width, height, yuvshm);
	}
#endif /* PITCH_WORKAROUND */
	hwdata->yuv_use_mitshm = (hwdata->image != NULL);
	if ( hwdata->yuv_use_mitshm ) {
		yuvshm->shmid = shmget(IPC_PRIVATE, hwdata->image->data_size,
				       IPC_CREAT | 0777);
		if ( yuvshm->shmid >= 0 ) {
			yuvshm->shmaddr = (char *)shmat(yuvshm->shmid, 0, 0);
			yuvshm->readOnly = False;
			if ( yuvshm->shmaddr != (char *)-1 ) {
				shm_error = False;
				X_handler = XSetErrorHandler(shm_errhandler);
				XShmAttach(GFX_Display, yuvshm);
				XSync(GFX_Display, True);
				XSetErrorHandler(X_handler);
				if ( shm_error )
					shmdt(yuvshm->shmaddr);
			} else {
				shm_error = True;
			}
			shmctl(yuvshm->shmid, IPC_RMID, NULL);
		} else {
			shm_error = True;
		}
		if ( shm_error ) {
			XFree(hwdata->image);
			hwdata->yuv_use_mitshm = 0;
		} else {
			hwdata->image->data = yuvshm->shmaddr;
		}
	}
	if ( !hwdata->yuv_use_mitshm )
#endif /* NO_SHARED_MEMORY */
	{
		hwdata->image = SDL_NAME(XvCreateImage)(GFX_Display, xv_port, format,
							0, width, height);

#ifdef PITCH_WORKAROUND
		if ( hwdata->image != NULL && hwdata->image->pitches[0] != (width*bpp) ) {
			/* Ajust overlay width according to pitch */ 
			XFree(hwdata->image);
			width = hwdata->image->pitches[0] / bpp;
			hwdata->image = SDL_NAME(XvCreateImage)(GFX_Display, xv_port, format,
								0, width, height);
		}
#endif /* PITCH_WORKAROUND */
		if ( hwdata->image == NULL ) {
			SDL_SetError("Couldn't create XVideo image");
			SDL_FreeYUVOverlay(overlay);
			return(NULL);
		}
		hwdata->image->data = malloc(hwdata->image->data_size);
		if ( hwdata->image->data == NULL ) {
			SDL_OutOfMemory();
			SDL_FreeYUVOverlay(overlay);
			return(NULL);
		}
	}

	/* Find the pitch and offset values for the overlay */
	overlay->planes = hwdata->image->num_planes;
	overlay->pitches = (Uint16 *)malloc(overlay->planes * sizeof(Uint16));
	overlay->pixels = (Uint8 **)malloc(overlay->planes * sizeof(Uint8 *));
	if ( !overlay->pitches || !overlay->pixels ) {
		SDL_OutOfMemory();
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}
	for ( i=0; i<overlay->planes; ++i ) {
		overlay->pitches[i] = hwdata->image->pitches[i];
		overlay->pixels[i] = (Uint8 *)hwdata->image->data +
		                              hwdata->image->offsets[i];
	}

#ifdef XFREE86_REFRESH_HACK
	/* Work around an XFree86 X server bug (?)
	   We can't perform normal updates in windows that have video
	   being output to them.  See SDL_x11image.c for more details.
	 */
	X11_DisableAutoRefresh(this);
#endif

	/* We're all done.. */
	return(overlay);
}

int X11_LockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	return(0);
}

void X11_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	return;
}

int X11_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *dstrect)
{
	struct private_yuvhwdata *hwdata;

	hwdata = overlay->hwdata;
#ifndef NO_SHARED_MEMORY
	if ( hwdata->yuv_use_mitshm ) {
		SDL_NAME(XvShmPutImage)(GFX_Display, hwdata->port, SDL_Window, SDL_GC,
	              hwdata->image, 0, 0, overlay->w, overlay->h,
	              dstrect->x, dstrect->y, dstrect->w, dstrect->h, False);
	}
	else
#endif
	{
		SDL_NAME(XvPutImage)(GFX_Display, hwdata->port, SDL_Window, SDL_GC,
				     hwdata->image, 0, 0, overlay->w, overlay->h,
				     dstrect->x, dstrect->y, dstrect->w, dstrect->h);
	}
	XSync(GFX_Display, False);
	return(0);
}

void X11_FreeYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata;

	hwdata = overlay->hwdata;
	if ( hwdata ) {
		SDL_NAME(XvUngrabPort)(GFX_Display, hwdata->port, CurrentTime);
#ifndef NO_SHARED_MEMORY
		if ( hwdata->yuv_use_mitshm ) {
			XShmDetach(GFX_Display, &hwdata->yuvshm);
			shmdt(hwdata->yuvshm.shmaddr);
		}
#endif
		if ( hwdata->image ) {
			XFree(hwdata->image);
		}
		free(hwdata);
	}
	if ( overlay->pitches ) {
		free(overlay->pitches);
		overlay->pitches = NULL;
	}
	if ( overlay->pixels ) {
		free(overlay->pixels);
		overlay->pixels = NULL;
	}
#ifdef XFREE86_REFRESH_HACK
	X11_EnableAutoRefresh(this);
#endif
}

#endif /* XFREE86_XV */
