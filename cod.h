#ifndef COD_H
#define COD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Platform enumeration
#define COD_X11 1
#define COD_WINDOWS 2

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
  // Char's position in the font texture
  int x, y;
  // Width of character
  int width, height;
  // How much to offset the image from the texture to the screen
  int xoffset, yoffset;
  // How much x to advance by
  int xadvance;
} cod_char;

typedef struct {
  cod_image* image;
  cod_char chars[1];
} cod_font;

// Window dimensions
extern int cod_window_width, cod_window_height;

// Pixel buffer - you can write directly to this if you
// like. cod_draw() draws this to the screen
cod_image* cod_pixels;

// If cod generates an error message, this will return it
const char* cod_get_error();
void cod_clear_error();

// Open a window and initializes cod. Returns 0 on failure.
int cod_open(int width, int height);

// Draw the buffer to the screen
void cod_swap();

// Clear the screen
void cod_clear();

// Set the window's title
void cod_set_title(const char* title);

// Fills EVENT with the next event to be processed. Returns 0 if there
// are no new events.
int cod_get_event(cod_event* event);

// Shuts down cod
void cod_close();

// Sleeps for MICROSECONDS
void cod_sleep(int microseconds);

cod_image* cod_load_image(const char* path);
void cod_free_image(cod_image*);
void cod_draw_image(cod_image* src, int src_x, int src_y, int width, 
			int height, cod_image* dst, int dst_x, int dst_y);
// Faster draw with no transparency
void cod_draw_over_image(cod_image* src, int src_x, int src_y, int width, 
                         int height, cod_image* dst, int dst_x, int dst_y);
void cod_simple_draw_image(cod_image* src, int dst_x, int dst_y);

// Bitmap fonts
cod_font* cod_load_font(const char* fnt_path, const char* png_path);
cod_image* cod_draw_text(cod_font* font, const char* text, cod_pixel fg);
void cod_size_text(cod_font* font, int* width, int* height, const char* text);
void cod_free_font(cod_font* font);

#ifdef __cplusplus
extern "C" {
#endif

#endif
