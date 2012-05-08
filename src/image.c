// image.c - cod image support
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Should be a macro, later on
void COD_DRAW_IMAGE() {

}

void cod_draw_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {
  // For looping
  int y, src_offset, dst_offset, x, alpha, inverse_alpha;
  cod_pixel srcp, dstp, result;

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
      dstp = *(dst->data + dst_offset);
      srcp = *(src->data + src_offset);

      alpha = COD_PIXEL_A(srcp);
      inverse_alpha = 255 - alpha;

      // TODO: FIXME after turning into int
      result = COD_MAKE_PIXEL(                                     \
        ((COD_PIXEL_R(srcp) * alpha) + (COD_PIXEL_R(dstp) * inverse_alpha)) >> 8, \
        ((COD_PIXEL_G(srcp) * alpha) + (COD_PIXEL_G(dstp) * inverse_alpha)) >> 8, \
        ((COD_PIXEL_B(srcp) * alpha) + (COD_PIXEL_B(dstp) * inverse_alpha)) >> 8, \
        alpha \
      );

      *(dst->data + dst_offset) = result;

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_image_tinted(cod_image* src, cod_pixel fg, int src_x, int src_y, int width,
                           int height, cod_image* dst, int dst_x, int dst_y) {
  int y, src_offset, dst_offset, x, alpha, inverse_alpha;
  cod_pixel srcp, dstp, result;
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
      dstp = *(dst->data + dst_offset);
      srcp = *(src->data + src_offset);

      alpha = COD_PIXEL_A(srcp);

      tint_r = (COD_PIXEL_R(fg) + COD_PIXEL_R(srcp)) / 2.0f;
      tint_g = (COD_PIXEL_G(fg) + COD_PIXEL_G(srcp)) / 2.0f;
      tint_b = (COD_PIXEL_B(fg) + COD_PIXEL_B(srcp)) / 2.0f;

      inverse_alpha = 255 - alpha;

      result = COD_MAKE_PIXEL(
        (((int)tint_r * alpha) + (COD_PIXEL_R(dstp) * inverse_alpha)) >> 8,
        (((int)tint_g * alpha) + (COD_PIXEL_G(dstp) * inverse_alpha)) >> 8,
        (((int)tint_b * alpha) + (COD_PIXEL_B(dstp) * inverse_alpha)) >> 8,
        COD_PIXEL_A(dstp));

      *(dst->data + dst_offset) = result;

      ++src_offset;
      ++dst_offset;
    }
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
