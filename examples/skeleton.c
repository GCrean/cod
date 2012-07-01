// skeleton.c - This is a skeleton application that doesn't do anything except exit
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

static void print_modifiers(cod_event* e) {
  if(e->key_down.modifiers & COD_MOD_ALT) printf("alt ");
  if(e->key_down.modifiers & COD_MOD_CONTROL) printf("ctrl ");
  if(e->key_down.modifiers & COD_MOD_SHIFT) printf("shift ");

}

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
        case COD_QUIT:
          running = 0;
          break;
        case COD_KEY_DOWN:
          printf("key down key=%s x=%d y=%d ", cod_key_name(e.key_down.key), e.key_down.x, e.key_down.y);
          print_modifiers(&e);
          putchar('\n');
          break;
        case COD_KEY_UP:
          printf("key up key=%s x=%d y=%d ", cod_key_name(e.key_up.key), e.key_up.x, e.key_up.y);
          print_modifiers(&e);
          putchar('\n');
          break;
        case COD_REDRAW:
          printf("redraw\n");
          break;
        case COD_FOCUS:
          printf("focus\n");
          break;
        case COD_UNFOCUS:
          printf("unfocus\n");
          break;
        case COD_MOUSE_MOTION:
          // Don't print anything because it's kind of annoying
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
