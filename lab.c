// font.c - draw some fonts
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

static int yoffset = 0;

static void font_demo(const char* path, cod_pixel fg) {
  char fnt_buffer[COD_BUFFER_SIZE];
  char png_buffer[COD_BUFFER_SIZE];
  snprintf(fnt_buffer, COD_BUFFER_SIZE, "%s.fnt", path);
  snprintf(png_buffer, COD_BUFFER_SIZE, "%s_0.png", path);

  cod_font* font = cod_load_font(fnt_buffer, png_buffer);
  if(!font) {
    printf("%s\n", cod_get_error());
    exit(EXIT_FAILURE);
  }

  static const char* txt1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ 12345";
  static const char* txt2 = "abcdefghijklmnopqrstuvwxyz 67890";
  static const char* txt3 = "{}[]()<>$*-+=/#_%^@.&|~?'\" !,.;:";
  static const char* txt4 = "The quick brown fox jumps over the lazy dog.";
  static const char* txt5 = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt";

  int rough_height = 0, w = 0;
  cod_size_text(font, &w, &rough_height, txt1);

  rough_height += 2;

  cod_draw_text(font, txt1, fg, cod_pixels, 0, yoffset+0);
  cod_draw_text(font, txt2, fg, cod_pixels, 0, yoffset+rough_height);
  cod_draw_text(font, txt3, fg, cod_pixels, 0, yoffset+rough_height*2);
  cod_draw_text(font, txt4, fg, cod_pixels, 0, yoffset+rough_height*4);
  cod_draw_text(font, txt5, fg, cod_pixels, 0, yoffset+rough_height*5);

  cod_free_font(font);
}

int main(void) {
  int running = 1;

  if(!cod_open(1280, 800)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("font");

  cod_swap();

  static cod_pixel white = {255,255,255,0};
  static cod_pixel blue = {100, 149, 237,0};

  font_demo("examples/proggy/ProggyCleanTTSZ-12px", white);
  yoffset += 80;
  font_demo("examples/droid/DroidSansMono-16px", white);
  yoffset += 120;
  font_demo("examples/kingthings/Kingthings-Petrock-32px", white);

  yoffset += 190;

  font_demo("examples/proggy/ProggyCleanTTSZ-12px", blue);
  yoffset += 80;
  font_demo("examples/droid/DroidSansMono-16px", blue);
  yoffset += 120;
  font_demo("examples/kingthings/Kingthings-Petrock-32px", blue);

  cod_event e;
  
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

    cod_swap();
  }
  
  cod_close();
  return EXIT_SUCCESS;
}

