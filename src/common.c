// common.c - misc. code

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cod.h"

// Check COD_PLATFORM
#ifndef COD_PLATFORM
# error "COD_PLATFORM must be defined when compiling Cod"
#endif

#if COD_PLATFORM == COD_X11
#elif COD_PLATFORM == COD_WIN32
#elif COD_PLATFORM == COD_COCOA
#else
# error "COD_PLATFORM unrecognized"
#endif

// Global variables
int cod_window_width = 0, cod_window_height = 0;
cod_image* cod_screen;

// Error reporting
char cod_error_buffer[COD_BUFFER_SIZE];

const char* cod_get_error() {
  return (const char*) cod_error_buffer;
}

void cod_clear_error() {
  memset(cod_error_buffer, 0, COD_BUFFER_SIZE);
}

int cod_open(int width, int height) {
  assert(sizeof(cod_pixel) == sizeof(int));
  assert(sizeof(int) == 4);

  cod_window_width = width;
  cod_window_height = height;

  cod_screen = cod_make_image(cod_window_width, cod_window_height);

  return _cod_open();
}

void cod_close() {
  _cod_close();

  if(cod_screen) {
    cod_free_image(cod_screen);
    cod_screen = NULL;
  }
}

// Clear screen to black
void cod_clear(void) {
  memset(cod_screen->data, 0, cod_screen->width * cod_screen->height * COD_BYTES_PER_PIXEL);
}

///// EVENT HANDLING
static const char* key_names[] = {
#define COD_KEY_DECL(X) #X,
  COD_DECLARE_KEYS(COD_KEY_DECL)
  "KEY_UNKNOWN",
#undef COD_KEY_DECL
};

const char* cod_key_name(cod_key key) {
  return key_names[key];
}
