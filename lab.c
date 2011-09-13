#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

void fail() {
  printf("%s\n", cod_get_error());
  exit(EXIT_FAILURE);
}

int main(void) {
  int running = 1;

  if(!cod_open(640, 480))
    fail();

  cod_set_title("cod sample");

  cod_image* cat = cod_load_image("examples/cat.png");
  // if this is run from the examples/ directory instead of the
  // toplevel cod directory, we'll need to load it from the current directory
  if(!cat) {
    cod_clear_error();
    cat = cod_load_image("cat.png");
    if(!cat)
      fail();
  }

  cod_pixel white = { 255, 255, 255, 255 };

  cod_event e;

  int update = 0;
  cod_draw_image(cat, 0, 0, 0, 0, cod_pixels, 0, 0);
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
      case COD_QUIT:
        running = 0;
        break;
      }
    }

    // Yield to CPU
    cod_sleep(50000);

    if(update) {
      cod_clear();

      update = 0;
    }

    cod_swap();
  }

  cod_free_image(cat);

  cod_close();
  return EXIT_SUCCESS;
}
