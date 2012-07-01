// primitives.c -- test out drawing primitives
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

#include "extra/drawing.c"

static cod_pixel white = COD_MAKE_PIXEL(255, 255, 255, 255);
static cod_pixel orange = COD_MAKE_PIXEL(237, 149, 100, 255);

int main(void) {
  cod_event e;
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("primitives");

  cod_draw_rect(cod_screen, orange, 5, 5, 50, 50);
  cod_fill_rect(cod_screen, white, 60, 5, 50, 50);
  cod_fill_bordered_rect(cod_screen, white, orange, 60+55, 5, 50, 50);
  
  cod_swap();
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
        case COD_QUIT:
          running = 0;
          break;
        default:
          break;
      }
    }

    cod_swap();
  }
  
  cod_close();
  return EXIT_SUCCESS;
}
