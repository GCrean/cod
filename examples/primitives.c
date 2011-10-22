// primitives.c -- test out drawing primitives
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(void) {
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("primitives");

  cod_pixel white = COD_MAKE_PIXEL(255,255,255);
  cod_pixel orange = COD_MAKE_PIXEL(237, 149, 100);
  cod_pixel red = COD_MAKE_PIXEL(255, 0, 0);
  cod_pixel blue = COD_MAKE_PIXEL(0, 0, 255);

  cod_draw_rect(cod_screen, 5, 5, 50, 50, orange);
  cod_fill_rect(cod_screen, 60, 5, 50, 50, white);
  cod_fill_bordered_rect(cod_screen, 60+55, 5, 50, 50, white, orange);
  
  cod_swap();

  cod_event e;
  
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
