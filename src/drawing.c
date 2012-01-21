// drawing.c - drawing primitives, lines, rectangles, etc
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

void cod_draw_horizontal_line(cod_image* image, cod_pixel fg, int x, int y, int width) {
  int offset = COD_IMAGE_OFFSET(x, y, image->width);
  int clip = COD_MIN(x+width, image->width);
  int ix;

  for(ix = x; ix <= clip; ix++) {
    image->data[offset++] = fg;
  }
}

void cod_draw_vertical_line(cod_image* image, cod_pixel fg, int x, int y, int height) {
  int clip = COD_MIN(y+height, image->height);
  int offset = COD_IMAGE_OFFSET(x, y, image->width);
  int iy;
  for(iy = y; iy <= clip; iy++, offset += image->width) {
    image->data[offset] = fg;
  }
}

void cod_draw_line(cod_image* image, cod_pixel fg, int x0, int y0, int x1, int y1) {
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

// Circles

void cod_draw_circle(cod_image* image, cod_pixel fg, int cx, int cy, int radius) {
  int error = -radius;
  int x = radius;
  int y = 0;
  while(x >= y) {
    COD_SET_PIXEL(image, cx + x, cy + y, fg);
    COD_SET_PIXEL(image, cx + y, cy + x, fg);

    if (x != 0) {
      COD_SET_PIXEL(image, cx - x, cy + y, fg);
      COD_SET_PIXEL(image, cx + y, cy - x, fg);
    }
       
    if (y != 0) {
      COD_SET_PIXEL(image, cx + x, cy - y, fg);
      COD_SET_PIXEL(image, cx - y, cy + x, fg);
    }
       
    if (x != 0 && y != 0) {
      COD_SET_PIXEL(image, cx - x, cy - y, fg);
      COD_SET_PIXEL(image, cx - y, cy - x, fg);
    }
           
    error += y;
    ++y;
    error += y;

    if (error >= 0) {
      --x;
      error -= x;
      error -= x;
    }
  }
}

void cod_fill_circle(cod_image* image, cod_pixel fg, int cx, int cy, int radius) {
  double r = (double) radius;
  double dy, dx;
  int x;
  int offset_a, offset_b;
  for(dy = 1; dy <= r; dy += 1.0) {
    dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
    x = cx - dx;
    offset_a = COD_IMAGE_OFFSET(x, cy + r - dy, image->width);
    offset_b = COD_IMAGE_OFFSET(x, cy - r + dy, image->width);
    for(; x <= cx + dx; x++) {
      image->data[offset_a] = fg;
      image->data[offset_b] = fg;
      ++offset_a;
      ++offset_b;
    }
  }
}

// Rectangles

void cod_draw_rect(cod_image* image, cod_pixel fg, int x, int y, int w, int h) {
  cod_draw_horizontal_line(image, fg, x, y, w);
  cod_draw_horizontal_line(image, fg, x, y+h, w);
  cod_draw_vertical_line(image, fg, x, y, h);
  cod_draw_vertical_line(image, fg, x+w, y, h);
}

void cod_fill_rect(cod_image* image, cod_pixel fg, int x, int y, int w, int h) {
  int yi;
  for(yi = y; yi < y+h+1; yi++) {
    cod_draw_horizontal_line(image, fg, x, yi, w);
  }
}

void cod_fill_bordered_rect(cod_image* image, cod_pixel border, cod_pixel fill, int x, int y, int w, int h) {
  cod_fill_rect(image, fill, x+1, y+1, w-1, h-1);
  cod_draw_rect(image, border, x, y, w, h);
}
