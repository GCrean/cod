#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "cod.h"

// Constants
#define COD_BUFFER_SIZE 512
#define COD_BYTES_PER_PIXEL sizeof(cod_pixel)

// Global variables
int cod_window_width = 0, cod_window_height = 0;
cod_image* cod_pixels;

// Utility macros
#define COD_ALLOCATE(x) (calloc(1, (x)))

// Get the offset of an (x,y) coordinate pair in a one-dimensional
// array: (y * width) + x
#define COD_IMAGE_OFFSET(x, y, width) (((y) * (width)) + x)

// Return the smaller of two numbers
#define COD_MIN(a,b) (((a) > (b)) ? (b) : (a))
#define COD_MAX(a,b) (((a) < (b)) ? (b) : (a))

// Include stb_image.c, but suppress lots of warnings and don't
// include HDR image support
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wall"
# pragma clang diagnostic ignored "-Wextra"
# pragma clang diagnostic ignored "-Wunused-function"
#endif
#define STBI_NO_HDR
#include "stb-png.c"
#if defined(__clang__)
# pragma clang diagnostic pop
#endif

// Error reporting

#define COD_ERROR(fmt, ...) snprintf(error_buffer, COD_BUFFER_SIZE, "cod: "fmt, __VA_ARGS__)
#define COD_ERROR0(fmt) snprintf(error_buffer, COD_BUFFER_SIZE, "cod: "fmt)

static char error_buffer[COD_BUFFER_SIZE];

const char* cod_get_error() {
  return (const char*) error_buffer;
}

void cod_clear_error() {
  memset(error_buffer, 0, COD_BUFFER_SIZE);
}

// The following functions are common platform-independent operations
// that are called by the platform-specific functions of the same name

static void _cod_open(int width, int height) {
  assert(sizeof(cod_pixel) == sizeof(int));

  cod_window_width = width;
  cod_window_height = height;
}

static void _cod_close() {
  if(cod_pixels) {
    cod_free_image(cod_pixels);
  }
}

// Clear screen to black

void cod_clear(void) {
  memset(cod_pixels->data, 0, cod_pixels->width * cod_pixels->height * 4);
}

// Image support

cod_image* cod_make_image(int width, int height) {
  cod_image* image = COD_ALLOCATE(sizeof(cod_image));
  image->width = width;
  image->height = height;
  image->data = COD_ALLOCATE(width * height * 4);
  return image;
}

cod_image* cod_load_image(const char* path) {
  cod_image* image = 0;
  int width = 0, height = 0, components = 0;
  unsigned char* data = stbi_load(path, &width, &height, &components, 4);

  if(!data) {
    COD_ERROR("cod_load_image: stbi_load failed to load \"%s\": %s ", path, stbi_failure_reason());
    return NULL;
  }

  image = COD_ALLOCATE(sizeof(cod_image));

  image->width = width;
  image->height = height;
  image->data = (cod_pixel*) data;

  return image;
}

void cod_free_image(cod_image* image) {
  free(image->data);
  free(image);
}

void cod_draw_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(int x = 0; x < width; x++) {
      cod_pixel* dstp = dst->data + dst_offset;
      cod_pixel* srcp = src->data + src_offset;
      
      int alpha = srcp->a;
      int inverse_alpha = 256 - alpha;

      dstp->r = ((srcp->r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = ((srcp->g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = ((srcp->b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      dstp->a = srcp->a;

      ++src_offset;
      ++dst_offset;
    }
  }
}


void cod_draw_over_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    memcpy(dst->data + dst_offset, src->data + src_offset, width * sizeof(cod_pixel));
  }
}

void cod_simple_draw_image(cod_image* image, int dst_x, int dst_y) {
  cod_draw_image(image, 0, 0, 0, 0, cod_pixels, dst_x, dst_y);
}

// Font support
// Uses bmfont by angelcode.com

// Short macro that checks for I/O errors and exits gracefully
#define check_file() \
  if(ferror(file) || feof(file)) {              \
    if(ferror(file)) { COD_ERROR("cod_load_font: unexpected error in file \"%s\"", fnt_path); } \
    if(feof(file)) { COD_ERROR("cod_load_font: unexpected EOF in file \"%s\"", fnt_path); } \
    fclose(file);                                                       \
    if(font) { free(font); }                                            \
    return NULL; \
  }

// TODO: Port to binary 
cod_font* cod_load_font(const char* fnt_path, const char* png_path) {
  // Gross, I hate C's I/O
  FILE* file = fopen(fnt_path, "r");

  if(!file) {
    COD_ERROR("cod_load_font: failed to open %s", fnt_path);
    return NULL;
  }

  char buffer[COD_BUFFER_SIZE];

  cod_font* font = NULL;

  // Skip the first three lines
  for(int newlines = 0; newlines != 3; newlines++) {
    fgets(buffer, COD_BUFFER_SIZE, file);
    check_file();
  }

  int chars = 0;

  fscanf(file, "chars count=%d\n", &chars);

  font = (cod_font*) COD_ALLOCATE(sizeof(cod_font) + (sizeof(cod_char) * (255-32)));

  //printf("%d\n", count);

  for(int i = 0; i != chars; i++) {
    int id=0, x=0, y=0, width=0, height=0, xoffset=0, yoffset=0, xadvance=0, page=0, chnl=0;

    fscanf(file, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n",
           &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance, &page, &chnl);

    if(ferror(file)) {
      check_file();
    }

    //printf("char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n", id, x, y, width, height, xoffset, yoffset, xadvance, page, chnl);

    cod_char* chr = &font->chars[id-32];
    chr->x = x;
    chr->y = y;
    chr->width = width;
    chr->height = height;
    chr->xoffset = xoffset;
    chr->yoffset = yoffset;
    chr->xadvance = xadvance;

    if(feof(file))
      break;
  }

  fclose(file);

  font->image = cod_load_image(png_path);
  if(!font->image) {
    free(font);
    return NULL;
  }

  return font;
}

#define COD_GET_CHAR(font, c) font->chars[((int)(c))-32]

void cod_size_text(cod_font* font, int* width, int* height, const char* text) {
  char c=0;
  int w=0,h=0;
  while((c = *text++)) {
    cod_char* ch = &COD_GET_CHAR(font, c);
    w += ch->xadvance;
    h = COD_MAX(ch->height + ch->yoffset, h);
  }
  (*width) = w;
  (*height) = h;
}

static void cod_draw_char(cod_image* src, int src_x, int src_y, int width,
                          int height, cod_image* dst, int dst_x, int dst_y,
                          cod_pixel fg) {

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(int x = 0; x < width; x++) {
      cod_pixel* dstp = dst->data + dst_offset;
      cod_pixel* srcp = src->data + src_offset;
      
      // Wow, I'm really not sure about any of this.
      int alpha = (srcp->r + srcp->g + srcp->b) / 3;
      int inverse_alpha = 256 - alpha;

      int tint_r = ((fg.r) / 255) * srcp->r;
      int tint_g = ((fg.g) / 255) * srcp->g;
      int tint_b = ((fg.b) / 255) * srcp->b;

      dstp->r = ((tint_r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = ((tint_g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = ((tint_b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      
      if(srcp->a != 255)
        printf("%d\n", srcp->a);

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_text_at(cod_font* font, const char* text, cod_pixel fg,
                            cod_image *target, int dstx, int dsty) {
  int x = 0, c = 0;

  while((c = *text++)) {
    cod_char* ch = &COD_GET_CHAR(font, c);

    cod_draw_char(font->image, ch->x, ch->y, ch->width, ch->height, target, dstx + x + ch->xoffset, dsty + ch->yoffset, fg);

    x += ch->xadvance;
  }
  
  //return image;
}

void cod_simple_draw_text(cod_font* font, int width, int height, const char* text) {
  (void) font; (void) width; (void) height; (void) text;
}

void cod_free_font(cod_font* font) {
  cod_free_image(font->image);
  free(font);
}

// Platform-specific code

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

#endif // COD_X11
