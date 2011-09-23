// image.c - load an image of a cat and blit it to the screen
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(void) {
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("image");

  cod_image* cat = cod_load_image("examples/cat.png");
  // if this is run from the examples/ directory instead of the
  // toplevel cod directory, we'll need to load it from the current directory
  if(!cat) {
    cat = cod_load_image("cat.png");
    if(!cat) {
      // something really went wrong
      printf("%s\n", cod_get_error());
      return EXIT_FAILURE;
    }
  }

  cod_event e;
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
      case COD_QUIT:
	running = 0;
	break;
      }
    }

    cod_clear();

    cod_simple_draw_image(cat, 50, 50);

    cod_swap();
  }

  cod_free_image(cat);
  
  cod_close();
  return EXIT_SUCCESS;
}
