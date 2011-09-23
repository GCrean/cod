// lab.c -- file for experimenting with cod
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

void fail() {
  printf("%s\n", cod_get_error());
  exit(EXIT_FAILURE);
}

void cod_draw_text_at(cod_font* font, const char* text, cod_pixel fg,
                            cod_image *target, int dstx, int dsty);

int main(void) {
  int running = 1;

  if(!cod_open(640, 480))
    fail();

  cod_set_title("cod sample");

  cod_image* cat = cod_load_image("examples/cat.png");
  cod_image* wee = cod_load_image("AmpharosStantlerSpriteTransparent.png");
  cod_font* font = cod_load_font("DroidSansMono.fnt", "DroidSansMono_0.png");

  if(!cat || !wee || !font) {
    fail();
  }

  cod_pixel white = { 255, 255, 255, 255 };

  cod_event e;

  const char* text_to_render = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod te";

  int update = 0;
  cod_draw_image(cat, 0, 0, 0, 0, cod_pixels, 0, 0);
  cod_draw_image(wee, 0, 0, 0, 0, cod_pixels, 50, 50);
  cod_draw_text_at(font, text_to_render, white, cod_pixels, 150, 120);
  cod_draw_text_at(font, text_to_render, white, cod_pixels, 250, 250); 

  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
      case COD_QUIT:
        running = 0;
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
  cod_free_image(cat);
  cod_free_image(wee);

  cod_close();
  return EXIT_SUCCESS;
}
