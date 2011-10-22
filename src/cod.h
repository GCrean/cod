#ifndef COD_H
#define COD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

///// CONSTANTS

#ifndef COD_BUFFER_SIZE
# define COD_BUFFER_SIZE 512
#endif

// Platform enumeration
#define COD_X11 1
#define COD_WIN32 2
#define COD_COCOA 3

#ifdef _MSC_VER
# define COD_PLATFORM COD_WIN32
#endif

///// MAIN

typedef struct cod_image cod_image;
typedef struct cod_event cod_event;

// Window dimensions
extern int cod_window_width, cod_window_height;

// Pixel buffer - you can write directly to this if you
// like. cod_draw() draws this to the window
extern cod_image* cod_screen;

// Return the smaller of two numbers
#define COD_MIN(a,b) (((a) > (b)) ? (b) : (a))
#define COD_MAX(a,b) (((a) < (b)) ? (b) : (a))

// Utility macros
#define COD_ALLOCATE(x) (calloc(1, (x)))

// Errors
#define COD_ERROR(fmt, ...) snprintf(cod_error_buffer, COD_BUFFER_SIZE, "cod: "fmt, __VA_ARGS__)
#define COD_ERROR0(fmt) snprintf(cod_error_buffer, COD_BUFFER_SIZE, "cod: "fmt)

// If cod generates an error message, this will return it
const char* cod_get_error();
void cod_clear_error();

extern char cod_error_buffer[COD_BUFFER_SIZE];

// Open a window and initializes cod. Returns 0 on failure.
int cod_open(int width, int height);
// Internal, platform-specific open
int _cod_open();

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
// Internal, platform-specific close
void _cod_close();

// Sleeps for MILLISECONDS
void cod_sleep(int milliseconds);

///// EVENTS

typedef enum {
  COD_EVENT_UNKNOWN = 0,
  COD_QUIT = 1,
  COD_KEY_DOWN = 2,
  COD_KEY_UP = 3,
  COD_MOUSE_MOTION
} cod_event_type;

// Keys are declared in a macro like this so we can convert them to strings in common.c
#define COD_DECLARE_KEYS(V) \
  V(MOUSE_LEFT)             \
  V(MOUSE_MIDDLE)           \
  V(MOUSE_RIGHT)            \
  V(MOUSE_WHEELUP)          \
  V(MOUSE_WHEELDOWN)        \
  V(KEY_A)                  \
  V(KEY_B)                  \
  V(KEY_C)                  \
  V(KEY_D)                  \
  V(KEY_E)                  \
  V(KEY_F)                  \
  V(KEY_G)                  \
  V(KEY_H)                  \
  V(KEY_I)                  \
  V(KEY_J)                  \
  V(KEY_K)                  \
  V(KEY_L)                  \
  V(KEY_M)                  \
  V(KEY_N)                  \
  V(KEY_O)                  \
  V(KEY_P)                  \
  V(KEY_Q)                  \
  V(KEY_R)                  \
  V(KEY_S)                  \
  V(KEY_T)                  \
  V(KEY_U)                  \
  V(KEY_V)                  \
  V(KEY_W)                  \
  V(KEY_X)                  \
  V(KEY_Y)                  \
  V(KEY_Z)                  \
  V(KEY_0)                  \
  V(KEY_1)                  \
  V(KEY_2)                  \
  V(KEY_3)                  \
  V(KEY_4)                  \
  V(KEY_5)                  \
  V(KEY_6)                  \
  V(KEY_7)                  \
  V(KEY_8)                  \
  V(KEY_9)                  \
  V(KEY_LEFT_ARROW)         \
  V(KEY_DOWN_ARROW)         \
  V(KEY_RIGHT_ARROW)        \
  V(KEY_UP_ARROW)           \
  V(KEY_LEFT_CONTROL)       \
  V(KEY_RIGHT_CONTROL)      \
  V(KEY_LEFT_SHIFT)         \
  V(KEY_RIGHT_SHIFT)        \
  V(KEY_LEFT_ALT)           \
  V(KEY_RIGHT_ALT)          \
  V(KEY_SUPER_L)            \
  V(KEY_CAPS_LOCK)          \
  V(KEY_TAB)                \
  V(KEY_GRAVE)              \
  V(KEY_ENTER)              \
  V(KEY_COMMA)              \
  V(KEY_PERIOD)             \
  V(KEY_SEMICOLON)          \
  V(KEY_BACKSPACE)          \
  V(KEY_SLASH)              \
  V(KEY_BACKSLASH)          \
  V(KEY_APOSTROPHE)         \
  V(KEY_LEFT_BRACKET)       \
  V(KEY_RIGHT_BRACKET)      \
  V(KEY_MINUS)              \
  V(KEY_EQUAL)              \
  V(KEY_SPACE)              \
  V(KEY_ESCAPE)             \
  V(KEY_NUMPAD_0)           \
  V(KEY_NUMPAD_1)           \
  V(KEY_NUMPAD_2)           \
  V(KEY_NUMPAD_3)           \
  V(KEY_NUMPAD_4)           \
  V(KEY_NUMPAD_5)           \
  V(KEY_NUMPAD_6)           \
  V(KEY_NUMPAD_7)           \
  V(KEY_NUMPAD_8)           \
  V(KEY_NUMPAD_9)           

typedef enum {
#define COD_KEY_DECL(X) COD_##X,
  COD_DECLARE_KEYS(COD_KEY_DECL)
  COD_KEY_UNKNOWN
#undef COD_KEY_DECL
} cod_key;

struct cod_event {
  cod_event_type type;
  union {
    struct { 
      // For mouse button presses
      int x, y;
      // Actual key
      cod_key key;
    } key_down;

    struct {
      // For mouse button releases
      int x, y;
      // Actual key
      cod_key key;
    } key_up;

    struct {
      int x, y;
    } mouse_motion;
  } data;
};

const char* cod_key_name(cod_key key);

///// IMAGES

// Currently this is treated as a struct of unsigned chars -- but it may become
// an int later for speed
typedef struct {
  unsigned char r, g, b, a;
} cod_pixel;

// Bytes per pixel
#define COD_BYTES_PER_PIXEL (sizeof(cod_pixel))

struct cod_image {
  int width, height;
  cod_pixel* data;
};

// Use this to construct pixels as the structure behind pixels may change later
#define COD_MAKE_PIXEL(r,g,b) {(r), (g), (b), 255 }

// Get the offset of an (x,y) coordinate pair in a one-dimensional
// array: (y * width) + x
#define COD_IMAGE_OFFSET(x, y, width) (((y) * (width)) + x)

// Set a pixel. Unsafe.
#define COD_SET_PIXEL(image, x, y, px) ((image)->data[((y) * (image)->width) + (x)]) = (px)

// Allocate a blank image
cod_image* cod_make_image(int width, int height);
// Load an image from a file
cod_image* cod_load_image(const char* path);
// Free an image
void cod_free_image(cod_image*);

// Draw an image
void cod_draw_image(cod_image* src, int src_x, int src_y, int width, 
			int height, cod_image* dst, int dst_x, int dst_y);

// Tint and then draw an image
void cod_draw_image_tinted(cod_image* src, int src_x, int src_y, int width,
                           int height, cod_image* dst, int dst_x, int dst_y,
                           cod_pixel fg);

// Faster draw with no transparency
void cod_draw_over_image(cod_image* src, int src_x, int src_y, int width, 
                         int height, cod_image* dst, int dst_x, int dst_y);

// Draw an image directly to the screen
void cod_simple_draw_image(cod_image* src, int dst_x, int dst_y);

// Draw an image file directly to the screen (obviously should not be used
// during a loop or something because it loads and then frees the image)
void cod_simple_draw_image_path(const char* image, int dst_x, int dst_y);

///// DRAWING

void cod_fill(cod_image* image, cod_pixel fg);
void cod_draw_horizontal_line(cod_image* image, int x, int y, int width, cod_pixel fg);
void cod_draw_vertical_line(cod_image* image, int x, int y, int height, cod_pixel fg);
void cod_draw_line(cod_image* image, int x1, int y1, int x2, int y2, cod_pixel fg);
void cod_draw_circle(cod_image* image, int cx, int cy, int radius, cod_pixel fg);
void cod_fill_circle(cod_image* image, int cx, int cy, int radius, cod_pixel fg);
void cod_draw_rect(cod_image* image, int x, int y, int w, int h, cod_pixel fg);
void cod_fill_rect(cod_image* image, int x, int y, int w, int h, cod_pixel fg);
void cod_fill_bordered_rect(cod_image* image, int x, int y, int w, int h, cod_pixel border, cod_pixel fill);

///// FONTS

typedef struct {
  int initialized;
  // Char's position in the font texture
  int x, y;
  // Width of character
  int width, height;
  // How much to offset the position when copying from the texture to the screen
  int xoffset, yoffset;
  // How much x to advance by
  int xadvance;
} cod_char;

typedef struct {
  cod_image* image;
  cod_char chars[1];
} cod_font;

cod_font* cod_load_font(const char* fnt_path, const char* png_path);
void cod_size_text(cod_font* font, int* width, int* height, const char* text);
void cod_draw_text(cod_font* font, const char* text, cod_pixel fg,
                   cod_image *target, int dstx, int dsty);
void cod_free_font(cod_font* font);

#ifdef COD_PRIVATE
# ifdef _MSC_VER
#  define snprintf(buffer, size, ...) _snprintf_s((buffer), (size), _countof((buffer)), __VA_ARGS__)
# endif
#endif

#ifdef __cplusplus
}
#endif

#endif // COD_H
