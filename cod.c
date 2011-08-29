#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cod.h"

// Include stb_image.c, but suppress lots of warnings and don't
// include HDR image support
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wall"
# pragma clang diagnostic ignored "-Wextra"
# pragma clang diagnostic ignored "-Wunused-function"
#endif
#define STBI_NO_HDR
#include "stb_image.c"
#if defined(__clang__)
# pragma clang diagnostic pop
#endif

#define COD_BUFFER_SIZE 512

// Internal stuff
int cod_window_width = 0, cod_window_height = 0, cod_bytes_per_pixel;
static char error_buffer[COD_BUFFER_SIZE];

static void* allocate(size_t size) {
  void* memory = calloc(1, size);
  assert(memory);
  return memory;
}

cod_image* cod_pixels;

// Get the offset of an (x,y) coordinate pair in a one-dimensional
// array: (y * width) + x
#define COD_ARRAY_OFFSET(x, y, width) (((y) * (width)) + x)
#define COD_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define COD_MIN(a,b) (((a) > (b)) ? (b) : (a))

#define COD_ERROR(fmt, ...) snprintf(error_buffer, COD_BUFFER_SIZE, "cod: "fmt, __VA_ARGS__)
#define COD_ERROR0(fmt) snprintf(error_buffer, COD_BUFFER_SIZE, "cod: "fmt)

const char* cod_get_error() { return (const char*) error_buffer; }

cod_image* cod_make_image(int width, int height) {
  cod_image* image = allocate(sizeof(cod_image));
  image->width = width;
  image->height = height;
  image->data = allocate(width * height * 4);
  return image;
}

cod_image* cod_load_image(const char* path) {
  cod_image* image = 0;
  int width = 0, height = 0, components = 0;
  unsigned char* data = stbi_load(path, &width, &height, &components, 4);

  if(!data) {
    COD_ERROR("stbi_load failed to load \"%s\": %s ", path, stbi_failure_reason());
    return NULL;
  }

  image = allocate(sizeof(cod_image));

  image->width = width;
  image->height = height;
  image->data = (cod_pixel*) data;

  return image;
}

void cod_free_image(cod_image* image) {
  free(image->data);
  free(image);
}

void cod_draw_image_ext(cod_image* src, int src_x, int src_y, int width, int height, 
		    cod_image* dst, int dst_x, int dst_y) {

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));
  /*
  if(src_x + width > src->width) {
    width = src->width - src_x;
  }

  if(src_y + height > src->height) {
    height = src->height - src_y;
  }

  if(dst_x + width > dst->width) {
    width = dst->width - dst_x;
  }

  if(dst_y + height > dst->height) {
    height = dst->height - dst_y;
  }
  */

  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      int src_offset = COD_ARRAY_OFFSET(src_x + x, src_y + y, src->width);
      int dst_offset = COD_ARRAY_OFFSET(dst_x + x, dst_y + y, dst->width);
      dst->data[dst_offset] = src->data[src_offset];
    }
  }
}

void cod_draw_image(cod_image* image, int src_x, int src_y) {
  for(int x = 0; x < image->width; x++) {
    for(int y = 0; y < image->height; y++) {
      int screen_offset = COD_ARRAY_OFFSET(x + src_x, y + src_y, cod_window_width);
      int image_offset = COD_ARRAY_OFFSET(x, y, image->width);
      cod_pixels->data[screen_offset] = image->data[image_offset];
    }
  }
}

void cod_clear(void) {
  memset(cod_pixels->data, 0, cod_pixels->width * cod_pixels->height * 4);
}

cod_font* cod_load_font(const char* path, int width, int height) {
  cod_image* image = cod_load_image(path);

  if(!image) {
    return NULL;
  }

  cod_font* font = (cod_font*) allocate(sizeof(cod_font));
  
  font->image = image;
  font->width = width;
  font->height = height;

  return font;
}

// The following functions are common platform-independent operations
// that are called by the platform-specific functions of the same name
static void _cod_open(int width, int height) {
  cod_window_width = width;
  cod_window_height = height;
}

static void _cod_close() {
  if(cod_pixels) {
    cod_free_image(cod_pixels);
  }
}

// Platform-specific code
#define COD_X11 1

#ifndef COD_PLATFORM
# error "COD_PLATFORM must be defined when compiling Cod"
#endif

#if COD_PLATFORM == COD_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

static Display* display = 0;
static Window window = 0;
static GC gc;
static XImage* image;
static char window_title[COD_BUFFER_SIZE];
static unsigned char* x_pixels;

#else

# error "COD_PLATFORM must be COD_X11"

#endif

#if COD_PLATFORM == COD_X11
int cod_open(int width, int height) {
  _cod_open(width, height);

  // Acquire display information
  if((display = XOpenDisplay(0)) == NULL) {
    COD_ERROR0("x11: XOpenDisplay failed");
    return 0;
  }

  int screen = DefaultScreen(display);
  Visual* visual = DefaultVisual(display, screen);
  if(!visual) {
    COD_ERROR0("x11: DefaultVisual failed");
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
    COD_ERROR0("x11: XCreateWindow failed");
    return 0;
  }

  // Allow the window manager to delete this window
  Atom wm_delete_window;
  wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", True);

  if(!wm_delete_window) {
    COD_ERROR0("x11: XInternAtom failed");
    return 0;
  }

  if(!XSetWMProtocols(display, window, &wm_delete_window, 1)) {
    COD_ERROR0("x11: XSetWMProtocols failed");
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
  cod_bytes_per_pixel = sizeof(cod_pixel);
  gc = DefaultGC(display, screen);
  image = XCreateImage(display, CopyFromParent, display_depth, ZPixmap, 0, NULL,
		       cod_window_width, cod_window_height, 32, cod_window_width * cod_bytes_per_pixel);

  if(!image) {
    COD_ERROR0("x11: XCreateImage failed");
    return 0;
  }

  cod_pixels = cod_make_image(cod_window_width, cod_window_height);
  x_pixels = allocate(cod_window_width * cod_window_height * 4);  

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

int cod_get_event(cod_event* event) {
  XEvent xevent;
  if(XCheckWindowEvent(display, window, -1, &xevent) || XCheckTypedWindowEvent(display, window, ClientMessage, &xevent)) {
    switch(xevent.type) {
    case ClientMessage:
      event->type = COD_QUIT;
      return 1;
    default:
      break;
    }
  }
  return 0;
}

void cod_swap() {
  // Convert RGBA to BGRA for x, then blit to screen
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
#endif // COD_X11
