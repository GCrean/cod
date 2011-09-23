// font.c -- cod font support; uses bmfont from angelcode.com

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

// Short macro that checks for I/O errors and exits gracefully
#define check_file() \
  if(ferror(file) || feof(file)) {              \
    if(ferror(file)) { COD_ERROR("cod_load_font: unexpected error in file \"%s\"", fnt_path); } \
    if(feof(file)) { COD_ERROR("cod_load_font: unexpected EOF in file \"%s\"", fnt_path); } \
    fclose(file);                                                       \
    if(font) { free(font); }                                            \
    return NULL; \
  }

// TODO: Port to binary
cod_font* cod_load_font(const char* fnt_path, const char* png_path) {
  // Gross, I hate C's I/O
  FILE* file = fopen(fnt_path, "r");

  if(!file) {
    COD_ERROR("cod_load_font: failed to open %s", fnt_path);
    return NULL;
  }

  char buffer[COD_BUFFER_SIZE];

  cod_font* font = NULL;

  // Skip the first three lines
  for(int newlines = 0; newlines != 3; newlines++) {
    fgets(buffer, COD_BUFFER_SIZE, file);
    check_file();
  }

  int chars = 0;

  fscanf(file, "chars count=%d\n", &chars);

  font = (cod_font*) COD_ALLOCATE(sizeof(cod_font) + (sizeof(cod_char) * (255-32)));

  //printf("%d\n", count);

  for(int i = 0; i != chars; i++) {
    int id=0, x=0, y=0, width=0, height=0, xoffset=0, yoffset=0, xadvance=0, page=0, chnl=0;

    fscanf(file, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n",
           &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance, &page, &chnl);

    if(ferror(file)) {
      check_file();
    }

    //printf("char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n", id, x, y, width, height, xoffset, yoffset, xadvance, page, chnl);

    cod_char* chr = &font->chars[id-32];
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

  fclose(file);

  font->image = cod_load_image(png_path);
  if(!font->image) {
    free(font);
    return NULL;
  }

  return font;
}

#define COD_GET_CHAR(font, c) font->chars[((int)(c))-32]

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

static void cod_draw_char(cod_image* src, int src_x, int src_y, int width,
                          int height, cod_image* dst, int dst_x, int dst_y,
                          cod_pixel fg) {
  assert(width && height);

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(int x = 0; x < width; x++) {
      cod_pixel* dstp = dst->data + dst_offset;
      cod_pixel* srcp = src->data + src_offset;

      float tint_r = floorf((fg.r / 255.0f) * srcp->r);
      float tint_g = floorf((fg.g / 255.0f) * srcp->g);
      float tint_b = floorf((fg.b / 255.0f) * srcp->b);
      int alpha = COD_MAX(tint_r, COD_MAX(tint_g, tint_b));
      int inverse_alpha = 256 - alpha;

      dstp->r = (((int)tint_r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = (((int)tint_g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = (((int)tint_b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      
      if(srcp->a != 255)
        printf("%d\n", srcp->a);

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_text(cod_font* font, const char* text, cod_pixel fg,
                            cod_image *target, int dstx, int dsty) {
  int x = 0, c = 0;

  while((c = *text++)) {
    cod_char* ch = &COD_GET_CHAR(font, c);

    cod_draw_char(font->image, ch->x, ch->y, ch->width, ch->height, target, dstx + x + ch->xoffset, dsty + ch->yoffset, fg);

    x += ch->xadvance;
  }
}

void cod_free_font(cod_font* font) {
  cod_free_image(font->image);
  free(font);
}
