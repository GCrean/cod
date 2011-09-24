// lab.c -- file for experimenting

#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

void fail() {
  printf("%s\n", cod_get_error());
  exit(EXIT_FAILURE);
}

void cod_draw_text2(cod_font* font, const char* text, cod_pixel fg,
                            cod_image *target, int dstx, int dsty);

int main(void) {
  int running = 1;

  if(!cod_open(640, 480))
    fail();

  cod_set_title("cod sample");

  cod_font* font = cod_load_font("examples/droid/DroidSansMono-16px.fnt", "examples/droid/DroidSansMono-16px_0.png");

  if(!font) {
    fail();
  }

  cod_pixel white = { 255, 255, 255, 255 };

  cod_event e;

  const char* text_to_render = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod te";

  int update = 0;
  cod_simple_draw_image_path("examples/cat.png", 0, 0);
  cod_simple_draw_image_path("sprite.png", 50, 50);
  
  /*
  cod_draw_text(font, text_to_render, white, cod_pixels, 150, 120);
  cod_draw_text(font, text_to_render, white, cod_pixels, 250, 250); 
  cod_draw_text2(font, text_to_render, white, cod_pixels, 150, 120+15);
  cod_draw_text2(font, text_to_render, white, cod_pixels, 250, 250+15); 
  cod_simple_draw_image_path("pop.png", 100, 100);

  cod_pixel black = { 0, 0, 0, 255 };

  cod_draw_text(font, text_to_render, black, cod_pixels, 150, 129);
  cod_draw_text2(font, text_to_render, black, cod_pixels, 150, 129+15);
  */
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

    if(update) {
      cod_clear();

      update = 0;
    }

    cod_swap();
  }
  
  cod_free_font(font);

  cod_close();
  return EXIT_SUCCESS;
}
