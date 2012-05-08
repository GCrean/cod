// eyes.c - draws an eyeball that follows the mouse cursor, among
// other things
#include <stdio.h>
#include <stdlib.h>

#define COD_LINK
#include "cod.c"

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

// Location of the circle
static int cx = 200, cy = 208;

static void render() {
  int dx = cx, dy = cy, diff = 0;

  cod_fill(cod_screen, white);
  cod_draw_image(puppy, 0, 0, 0, 0, cod_screen,
                 center(puppy->width, cod_screen->width),
                 center(puppy->height, cod_screen->height));

  cod_draw_image_tinted(cat, red, 0, 0, 0, 0, cod_screen, 400, 0);

  cod_draw_image(cat, 0, 0, 0, 0, cod_screen, 250, 250);

  snprintf(buffer, COD_BUFFER_SIZE, "Mouse: (%d, %d)", mouse_x, mouse_y);

  cod_draw_text(proggy, buffer, black, cod_screen, 5, 5);

  // Draw eyeball
  cod_fill_circle(cod_screen, white, cx, cy, 10);
  cod_draw_circle(cod_screen, black, cx, cy, 10);

  // Draw pupil
  diff = cx - mouse_x;
  dx += (diff > 0 ? -COD_MIN(4, diff) : COD_MIN(4, -diff));
  diff = cy - mouse_y;
  dy += (diff > 0 ? -COD_MIN(3, diff) : COD_MIN(3, -diff));

  cod_fill_circle(cod_screen, black, dx, dy, 4);

  cod_swap();
}

typedef struct test_pixel { unsigned char r,g,b,a; };

int main(void) {
  cod_event e;
  int running = 1, update=0;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("eyeballs");

  cod_swap();

  puppy = cod_load_image("examples/puppy.png");
  proggy = cod_load_font("examples/proggy/ProggyCleanTTSZ-12px.fnt",
                                   "examples/proggy/ProggyCleanTTSZ-12px_0.png");
  cat = cod_load_image("examples/cat.png");

  if(!puppy || !proggy || !cat) {
    fail();
  }

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

