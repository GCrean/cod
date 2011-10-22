// font.c - draw some fonts
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

#define center(src, dst) ((dst / 2) - (src / 2))

#define fail() \
  printf("%s\n", cod_get_error()); \
  exit(EXIT_FAILURE);

static int mouse_x = 0, mouse_y = 0;
static cod_image* puppy = NULL, *cat = NULL;
static cod_font* proggy = NULL;

static cod_pixel white = COD_MAKE_PIXEL(255, 255, 255, 255);
static cod_pixel black = COD_MAKE_PIXEL(0, 0, 0, 255);
static cod_pixel red = COD_MAKE_PIXEL(255, 0, 0, 255);
static char buffer[COD_BUFFER_SIZE];
 

static void render() {
  cod_fill(cod_screen, white);
  cod_draw_image(puppy, 0, 0, 0, 0, cod_screen,
                 center(puppy->width, cod_screen->width),
                 center(puppy->height, cod_screen->height));

  cod_draw_image(cat, 0, 0, 0, 0, cod_screen,
                 center(cat->width, cod_screen->width),
                 center(puppy->height, cod_screen->height));

  snprintf(buffer, COD_BUFFER_SIZE, "Mouse: (%d, %d)", mouse_x, mouse_y);

  cod_draw_text(proggy, buffer, black, cod_screen, 5, 5);

  cod_draw_circle(cod_screen, 200, 208, 5, black);

  cod_swap();
}

int main(void) {
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("eyeballs");

  cod_swap();

  cod_event e;

  puppy = cod_load_image("examples/puppy.png");
  proggy = cod_load_font("examples/proggy/ProggyCleanTTSZ-12px.fnt",
                                   "examples/proggy/ProggyCleanTTSZ-12px_0.png");
  cat = cod_load_image("examples/cat.png");

  if(!puppy || !proggy || !cat) {
    fail();
  }

  int update = 0;

  render();
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
        case COD_QUIT:
          running = 0;
          break;
        case COD_MOUSE_MOTION:
          update = 1;
          mouse_x = e.data.mouse_motion.x;
          mouse_y = e.data.mouse_motion.y;
          break;
        default:
          break;
      }
    }

    if(update) {
      render();

      update = 0;
    }

    cod_sleep(1);
  }
  
  cod_free_image(puppy);
  cod_free_font(proggy);
  cod_free_image(cat);

  cod_close();
  return EXIT_SUCCESS;
}

