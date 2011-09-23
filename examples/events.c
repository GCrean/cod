// events.c - reads and prints events (except mouse motion which is pretty frequent)
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(void) {
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("events");

  cod_swap();

  cod_event e;
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
        case COD_QUIT:
          running = 0;
          break;
        case COD_KEY_DOWN:
          printf("key down key=%s x=%d y=%d\n", cod_key_name(e.data.key_down.key), e.data.key_down.x, e.data.key_down.y);
          break;
        case COD_KEY_UP:
          printf("key up key=%s x=%d y=%d\n", cod_key_name(e.data.key_down.key), e.data.key_down.x, e.data.key_down.y);
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
