// drawing.c -- drawing primitives, lines, rectangles, etc
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

void cod_fill(cod_image* image, cod_pixel fg) {
  int x,y;

  for(y = 0; y < image->height; y++) {
    int offset = COD_IMAGE_OFFSET(0, y, image->width);
    for(x = 0; x < image->width; x++) {
      cod_pixel* dst = &image->data[offset];
      (*dst) = fg;
      ++offset;
    }
  }
}

void cod_draw_horizontal_line(cod_image* image, int x, int y, int width, cod_pixel fg) {
  int offset = COD_IMAGE_OFFSET(x, y, image->width);
  int clip = COD_MIN(x+width, image->width);
  int ix;

  for(ix = x; ix <= clip; ix++) {
    image->data[offset++] = fg;
  }
}

void cod_draw_vertical_line(cod_image* image, int x, int y, int height, cod_pixel fg) {
  int clip = COD_MIN(y+height, image->height);
  int offset = COD_IMAGE_OFFSET(x, y, image->width);
  int iy;
  for(iy = y; iy <= clip; iy++, offset += image->width) {
    image->data[offset] = fg;
  }
}

void cod_draw_line(cod_image* image, int x0, int y0, int x1, int y1, cod_pixel fg) {
  // TODO: Probably should optimize for horizontal/vertical lines
  int dx, dy, sx, sy, err, e2;

  dx = abs(x1-x0);
  dy = abs(y1-y0);

  sx = x0 < x1 ? 1 : -1;
  sy = y0 < y1 ? 1 : -1;

  err = (dx>dy ? dx : -dy)/2;
 
  for(;;) {
    COD_SET_PIXEL(image, x0, y0, fg);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}


static void plot4points(cod_image* image, int cx, int cy, int x, int y, cod_pixel fg) {
  COD_SET_PIXEL(image, cx + x, cy + y, fg);
  if(x != 0) COD_SET_PIXEL(image, cx - x, cy + y, fg);
  if(y != 0) COD_SET_PIXEL(image, cx + x, cy - y, fg);
  if(x != 0 && y != 0) COD_SET_PIXEL(image, cx - x, cy - y, fg);
}

static void plot8points(cod_image* image, int cx, int cy, int x, int y, cod_pixel fg) {
  plot4points(image, cx, cy, x, y, fg);
  if(x != y)
    plot4points(image, cx, cy, y, x, fg);
}

void cod_draw_circle(cod_image* image, int cx, int cy, int radius, cod_pixel fg) {
  int error = -radius;
  int x = radius;
  int y = 0;
  while(x >= y) {
    plot8points(image, cx, cy, x, y, fg);
    error += y;
    ++y;
    error += y;
    if(error >= 0) {
      --x;
      error -= x;
      error -= x;
    }
  }
}

void cod_fill_circle(cod_image* image, int cx, int cy, int radius, cod_pixel fg) {
  (void) image;
  (void) cx;
  (void) cy;
  (void) radius;
  (void) fg;
}

// Rectangle

void cod_draw_rect(cod_image* image, int x, int y, int w, int h, cod_pixel fg) {
  cod_draw_horizontal_line(image, x, y, w, fg);
  cod_draw_horizontal_line(image, x, y+h, w, fg);
  cod_draw_vertical_line(image, x, y, h, fg);
  cod_draw_vertical_line(image, x+w, y, h, fg);
}

void cod_fill_rect(cod_image* image, int x, int y, int w, int h, cod_pixel fg) {
  int yi;
  for(yi = y; yi < y+h+1; yi++) {
    cod_draw_horizontal_line(image, x, yi, w, fg);
  }
}

void cod_fill_bordered_rect(cod_image* image, int x, int y, int w, int h, cod_pixel border, cod_pixel fill) {
  cod_fill_rect(image, x, y, w, h, fill);
  cod_draw_rect(image, x+1, y+1, w-1, h-1, border);
}
