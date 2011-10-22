// events.c - reads and prints events
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(int argc, char** argv) {
  int running = 1;

  // If given an argument, we'll print all mouse motion out, otherwise
  // not (it's very annoying)
  int show_mouse_motion = argc > 1;

  cod_event e;

  if(argc > 1)
    printf("%s\n", argv[1]);

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("events");

  cod_swap();
  
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
          printf("key up key=%s x=%d y=%d\n", cod_key_name(e.data.key_up.key), e.data.key_up.x, e.data.key_up.y);
          break;
        case COD_MOUSE_MOTION:
          if(show_mouse_motion)
            printf("mouse motion x=%d y=%d\n", e.data.mouse_motion.x, e.data.mouse_motion.y);
          break;
        default:
          break;
      }
    }

    cod_sleep(1);
    cod_swap();
  }
  
  cod_close();
  return EXIT_SUCCESS;
}
