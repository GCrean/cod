// font.c -- cod font support; uses bmfont from angelcode.com

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef COD_PRIVATE
# define COD_PRIVATE
#endif

#include "cod.h"

#define COD_GET_CHAR(font, c) font->chars[c-32]

cod_font* _cod_load_font(FILE* file, cod_font* font, const char* fnt_path, const char* png_path) {
  char buffer[COD_BUFFER_SIZE];

  int lineHeight = 0, base = 0, scaleW = 0, scaleH = 0, pages = 0, packed = 0, alphaChnl = 0, redChnl = 0, greenChnl = 0, blueChnl = 0, chars = 0;

  // For looping
  int i=0, id=0, x=0, y=0, width=0, height=0, xoffset=0, yoffset=0, xadvance=0, page=0, chnl=0;
  cod_char* chr = NULL;

#define skip_line() \
  if(fgets(buffer, COD_BUFFER_SIZE, file) != buffer) {                 \
    COD_ERROR("cod_load_font: fgets failed in file \"%s\"", fnt_path); \
    return NULL; \
  }

  skip_line();

  if(fscanf(file, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d alphaChnl=%d redChnl=%d greenChnl=%d blueChnl=%d\n",
            &lineHeight, &base, &scaleW, &scaleH, &pages, &packed, &alphaChnl, &redChnl, &greenChnl, &blueChnl) != 10) {
    COD_ERROR("cod_load_font: file \"%s\": fscanf failed to scan row 'common'", fnt_path);
    return NULL;
  }

  if(pages != 1) {
    COD_ERROR("cod_load_font: file \"%s\": cod only supports one font page", fnt_path);
    return NULL;
  }

  skip_line();

#undef skip_line

  if(fscanf(file, "chars count=%d\n", &chars) != 1) {
    COD_ERROR("cod_load_font: file \"%s\": fscanf failed to scan row 'chars'", fnt_path);
    return NULL;
  }

  if(chars > 255-32) {
    COD_ERROR("cod_load_font: file \"%s\": cod only supports font characters from 32-255", fnt_path);
    return NULL;
  }

  for(i = 0; i != chars; i++) {
    if(fscanf(file, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n",
              &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance, &page, &chnl) != 10) {
      COD_ERROR("cod_load_font: file \"%s\": fscanf failed to scan char in file", fnt_path);
      return NULL;
    }

    // wrapped in if(ferror(file)) because EOF is acceptable while reading chars
    if(ferror(file)) {
      COD_ERROR("cod_load_font: file \"%s\" encountered ferror while reading 'char' row", fnt_path);
      return NULL;
    }

    //printf("char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n",
    //       id, x, y, width, height, xoffset, yoffset, xadvance, page, chnl);

    if(id > 255) {
      COD_ERROR("cod_load_font: encountered unsupported character %d\n", id);
      return NULL;
    } else if(id < 32) {
      continue;
    }

    chr = &COD_GET_CHAR(font, id);
    chr->initialized = 1;
    chr->x = x;
    chr->y = y;
    chr->width = width;
    chr->height = height;
    chr->xoffset = xoffset;
    chr->yoffset = yoffset;
    chr->xadvance = xadvance;

    if(feof(file))
      break;
  }

  font->image = cod_load_image(png_path);
  if(!font->image) {
    return NULL;
  }

  return font;
}


cod_font* cod_load_font(const char* fnt_path, const char* png_path) {
  // Open the file
  cod_font* font = NULL;
  FILE* file = fopen(fnt_path, "r");

  if(!file) {
    COD_ERROR("cod_load_font: fopen failed to open \"%s\"", fnt_path);
    return NULL;
  }

  // Allocate the font
  font = (cod_font*) COD_ALLOCATE(sizeof(cod_font) + (sizeof(cod_char) * 223));

  // Hand them off to the real loading routine, then clean up after it
  if(_cod_load_font(file, font, fnt_path, png_path) == NULL) {
    free(font);
    fclose(file);
    return NULL;
  } else {
    fclose(file);
    return font;
  }

}

void cod_size_text(cod_font* font, int* width, int* height, const char* text) {
  char c=0;
  int w=0,h=0;
  while((c = *text++)) {
    cod_char* ch = &COD_GET_CHAR(font, c);
    w += ch->xadvance;
    h = COD_MAX(ch->height + ch->yoffset, h);
  }
  (*width) = w;
  (*height) = h;
}

void cod_draw_text(cod_font* font, const char* text, cod_pixel fg,
                            cod_image *target, int dstx, int dsty) {
  int x = 0, c = 0;

  while((c = *text++)) {
    cod_char* ch = &COD_GET_CHAR(font, c);

    assert(ch->initialized);

    if(c != 32) {
      cod_draw_image_tinted(font->image, ch->x, ch->y, ch->width, ch->height, target,
                    dstx + x + ch->xoffset,
                    dsty + ch->yoffset, fg);
    }

    x += ch->xadvance;
  }
}

void cod_free_font(cod_font* font) {
  cod_free_image(font->image);
  free(font);
}
