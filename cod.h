#ifndef COD_H
#define COD_H

#include <stddef.h>

typedef enum {
  COD_QUIT
} cod_event_type;

typedef struct {
  cod_event_type type;
} cod_event;

typedef struct {
  unsigned char r, g, b, a;
} cod_pixel;

typedef struct {
  int width, height;
  cod_pixel* data;
} cod_image;

typedef struct {
  cod_image* image;
  // Font dimensions
  int width, height;
} cod_font;

// Window dimensions
extern int cod_window_width, cod_window_height;

// Pixel buffer - you can write directly to this if you
// like. cod_draw() draws this to the screen
cod_image* cod_pixels;

// If cod generates an error message, this will return it
const char* cod_get_error();

// Open a window and initializes cod. Returns 0 on failure.
int cod_open(int width, int height);
// Set the window's title
void cod_set_title(const char* title);
// Fills EVENT with the next event to be processed. Returns 0 if there
// are no new events.
int cod_get_event(cod_event* event);
// Shuts down cod
void cod_close();

// Sleeps for MICROSECONDS
void cod_sleep(int microseconds);

void cod_swap();
void cod_clear();

cod_image* cod_load_image(const char* path);
void cod_free_image(cod_image*);
void cod_draw_image_ext(cod_image* src, int src_x, int src_y, int width, 
			int height, cod_image* dst, int dst_x, int dst_y);
void cod_draw_image(cod_image* image, int dst_x, int dst_y);

// Bitmap fonts
cod_font* cod_load_font(const char* path, int width, int height);

#endif
