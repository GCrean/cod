// image.c -- cod image support

// TODO: Remove this #if and switch to premultiplied alpha permanently
// Leaving this in ATM because of difficulty with alpha blending so far
#define COD_PREMULTIPLIED_ALPHA 0

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ifndef is used in case all files are amalgamated into cod.c
#ifndef COD_PRIVATE
# define COD_PRIVATE
#endif

#include "cod.h"

cod_image* cod_make_image(int width, int height) {
  cod_image* image = (cod_image*) COD_ALLOCATE(sizeof(cod_image));
  image->width = width;
  image->height = height;
  image->data = (cod_pixel*) COD_ALLOCATE(width * height * COD_BYTES_PER_PIXEL);
  return image;
}

cod_image* cod_load_image(const char* path) {
  cod_image* image = 0;
  int width = 0, height = 0, components = 0;
  unsigned char* data = stbi_load(path, &width, &height, &components, 4);

  if(!data) {
    COD_ERROR("cod_load_image: stbi_load failed to load \"%s\": %s ", path, stbi_failure_reason());
    return NULL;
  }

  image = (cod_image*) COD_ALLOCATE(sizeof(cod_image));

  image->width = width;
  image->height = height;
  image->data = (cod_pixel*) data;

  return image;
}

void cod_free_image(cod_image* image) {
  free(image->data);
  free(image);
}

void cod_draw_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {
  // For looping
  int y, src_offset, dst_offset, x, alpha, inverse_alpha;
  cod_pixel *srcp, *dstp;

  assert(src_x >= 0);
  assert(src_y >= 0);
  assert(dst_x >= 0);
  assert(dst_y >= 0);

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(y = 0; y < height; y++) {
    src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(x = 0; x < width; x++) {
      dstp = dst->data + dst_offset;
      srcp = src->data + src_offset;

      alpha = srcp->a;
      inverse_alpha = 255 - alpha;

      dstp->r = ((srcp->r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = ((srcp->g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = ((srcp->b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      dstp->a = srcp->a;

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_image_tinted(cod_image* src, cod_pixel fg, int src_x, int src_y, int width,
                           int height, cod_image* dst, int dst_x, int dst_y) {
  int y, src_offset, dst_offset, x, alpha, inverse_alpha;
  cod_pixel *srcp, *dstp;
  float tint_r = 0, tint_g = 0, tint_b = 0;

  assert(src_x >= 0);
  assert(src_y >= 0);
  assert(dst_x >= 0);
  assert(dst_y >= 0);


  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(y = 0; y < height; y++) {
    src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(x = 0; x < width; x++) {
      dstp = dst->data + dst_offset;
      srcp = src->data + src_offset;

      alpha = srcp->a;

      tint_r = (fg.r + srcp->r) / 2.0f;
      tint_g = (fg.g + srcp->g) / 2.0f;
      tint_b = (fg.b + srcp->b) / 2.0f;

      inverse_alpha = 255 - alpha;
      
      dstp->r = (((int)tint_r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = (((int)tint_g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = (((int)tint_b * alpha) + (dstp->b * inverse_alpha)) >> 8;

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_over_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {
  int y;

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    memcpy(dst->data + dst_offset, src->data + src_offset, width * COD_BYTES_PER_PIXEL);
  }
}

void cod_simple_draw_image(cod_image* image, int dst_x, int dst_y) {
  cod_draw_image(image, 0, 0, 0, 0, cod_screen, dst_x, dst_y);
}

void cod_simple_draw_image_path(const char* image, int dst_x, int dst_y) {
  cod_image* img = cod_load_image(image);
  if(!img) return;
  cod_simple_draw_image(img, dst_x, dst_y);
  cod_free_image(img);
}
