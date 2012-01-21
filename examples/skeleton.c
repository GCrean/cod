// skeleton.c - This is a skeleton application that doesn't do anything except exit
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(void) {
  int running = 1;
  cod_event e;


  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("skeleton");

  cod_swap();
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
        case COD_KEY_DOWN:
          printf("key down key=%s x=%d y=%d\n", cod_key_name(e.data.key_down.key), e.data.key_down.x, e.data.key_down.y);
          break;
        case COD_KEY_UP:
          printf("key up key=%s x=%d y=%d\n", cod_key_name(e.data.key_up.key), e.data.key_up.x, e.data.key_up.y);
          break;
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
