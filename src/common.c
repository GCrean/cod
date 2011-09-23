// common.c -- misc. code

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cod.h"

// Check COD_PLATFORM
#ifndef COD_PLATFORM
# error "COD_PLATFORM must be defined when compiling Cod"
#endif

#if COD_PLATFORM == COD_X11
#elif COD_PLATFORM == COD_WIN32
#else
# error "COD_PLATFORM unrecognized"
#endif

// Global variables
int cod_window_width = 0, cod_window_height = 0;
cod_image* cod_pixels;

// Error reporting
char cod_error_buffer[COD_BUFFER_SIZE];

const char* cod_get_error() {
  return (const char*) cod_error_buffer;
}

void cod_clear_error() {
  memset(cod_error_buffer, 0, COD_BUFFER_SIZE);
}

// The following functions are common platform-independent operations
// that are called by the platform-specific functions of the same name
void _cod_open(int width, int height) {
  assert(sizeof(cod_pixel) == sizeof(int));

  cod_window_width = width;
  cod_window_height = height;
}

void _cod_close() {
  if(cod_pixels) {
    cod_free_image(cod_pixels);
  }
}

// Clear screen to black
void cod_clear(void) {
  memset(cod_pixels->data, 0, cod_pixels->width * cod_pixels->height * 4);
}
