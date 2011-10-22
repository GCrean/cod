// primitives.c -- test out drawing primitives
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

static cod_pixel white = {255, 255, 255, 255};
static cod_pixel orange = {237, 149, 100, 255};

int main(void) {
  cod_event e;
  cod_pixel orange;
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("primitives");

  cod_draw_rect(cod_screen, 5, 5, 50, 50, orange);
  cod_fill_rect(cod_screen, 60, 5, 50, 50, white);
  cod_fill_bordered_rect(cod_screen, 60+55, 5, 50, 50, white, orange);
  
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
