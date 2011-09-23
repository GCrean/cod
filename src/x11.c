// x11.c -- cod x11 implementation

#if COD_PLATFORM == COD_X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "cod.h"

static Display* display = 0;
static Window window = 0;
static GC gc;
static XImage* image;
static char window_title[COD_BUFFER_SIZE];
static unsigned char* x_pixels;

int cod_open(int width, int height) {
  _cod_open(width, height);

  // Acquire display information
  if((display = XOpenDisplay(0)) == NULL) {
    COD_ERROR0("cod_open: x11: XOpenDisplay failed");
    return 0;
  }

  int screen = DefaultScreen(display);
  Visual* visual = DefaultVisual(display, screen);
  if(!visual) {
    COD_ERROR0("cod_open: x11: DefaultVisual failed");
  }

  Window root = DefaultRootWindow(display);

  // Get display depth
  int display_depth = DefaultDepth(display, screen);
  


  // Create the window 

  // Put our window in the middle

  XSetWindowAttributes attributes;
  attributes.border_pixel = attributes.background_pixel = BlackPixel(display, screen);

  window = XCreateWindow(display, root, 0, 0, cod_window_width, cod_window_height, 0, CopyFromParent,
			 InputOutput, CopyFromParent, CWBackPixel | CWBorderPixel, 
			 &attributes);

  if(!window) {
    COD_ERROR0("cod_open: x11: XCreateWindow failed");
    return 0;
  }

  // Allow the window manager to delete this window
  Atom wm_delete_window;
  wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", True);

  if(!wm_delete_window) {
    COD_ERROR0("cod_open: x11: XInternAtom failed");
    return 0;
  }

  if(!XSetWMProtocols(display, window, &wm_delete_window, 1)) {
    COD_ERROR0("cod_open: x11: XSetWMProtocols failed");
    return 0;
  }

  // Force X11 to obey our size specifications
  XSizeHints* sizehints = XAllocSizeHints();
  sizehints->min_width = sizehints->max_width = cod_window_width;
  sizehints->min_height = sizehints->max_height = cod_window_height;
  sizehints->flags = PMaxSize | PMinSize | USPosition;
  XSetWMNormalHints(display, window, sizehints);
  XFree(sizehints);  

  // Create image that we will draw to
  gc = DefaultGC(display, screen);
  image = XCreateImage(display, CopyFromParent, display_depth, ZPixmap, 0, NULL,
		       cod_window_width, cod_window_height, 32, cod_window_width * COD_BYTES_PER_PIXEL);

  if(!image) {
    COD_ERROR0("cod_open: x11: XCreateImage failed");
    return 0;
  }

  cod_pixels = cod_make_image(cod_window_width, cod_window_height);
  x_pixels = COD_ALLOCATE(cod_window_width * cod_window_height * 4);  

  // Tell X11 what events we care about
  XSelectInput(display, window, 
	       KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask);

  // Map window and return
  XMapRaised(display, window);

  XFlush(display);

  return 1;
}

void cod_set_title(const char* title) {
  strncpy(window_title, title, COD_BUFFER_SIZE);
  XTextProperty title_property;
  // Clang generates a warning because window_title is char[512] and
  // not char*, this suppresses that
  char* title_ptr = (char*) window_title;
  XStringListToTextProperty(&title_ptr, 1, &title_property);
  XSetWMName(display, window, &title_property);
  XFree(title_property.value);
  XFlush(display);
}

void cod_close() {
  _cod_close();

  if(x_pixels) {
    free(x_pixels);
  }

  if(image) {
    XDestroyImage(image);
    image = 0;
  }

  if(display) {
    if(window) {
      XDestroyWindow(display, window);
    }
    XCloseDisplay(display);
    display = 0;
  }
}

int cod_get_event(cod_event* cevent) {
  XEvent xevent;

  if(XPending(display)) {
    XNextEvent(display, &xevent);
    switch(xevent.type) {
    case ClientMessage:
      cevent->type = COD_QUIT;
      return 1;
    default:
      break;
    }
  }

  return 0;
}

void cod_swap() {
  // Convert RGBA to BGRA for x (cause apparently that's what it wants?), then write to screen
  for(int x = 0; x < cod_window_width; x++) {
    for(int y = 0; y < cod_window_height; y++) {
      int cod_offset = (y * cod_window_width) + x;
      int x_offset = cod_offset * 4;
      x_pixels[x_offset] = cod_pixels->data[cod_offset].b;
      x_pixels[x_offset+1] = cod_pixels->data[cod_offset].g;
      x_pixels[x_offset+2] = cod_pixels->data[cod_offset].r;
      x_pixels[x_offset+3] = cod_pixels->data[cod_offset].a;
    }
  }

  image->data = (char*) x_pixels;

  XPutImage(display, window, gc, image, 0, 0, 0, 0, cod_window_width, cod_window_height);
  XFlush(display);

  image->data = NULL;
}

extern int usleep(int);

void cod_sleep(int seconds) {
  usleep(seconds);
}

#endif // COD_PLATFORM == COD_X11
