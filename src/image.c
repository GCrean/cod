// image.c -- cod image support

#include <stdlib.h>

#include "cod.h"

// Include stb_image.c, but suppress lots of warnings and don't
// include HDR image support
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wall"
# pragma clang diagnostic ignored "-Wextra"
# pragma clang diagnostic ignored "-Wunused-function"
#endif
#define STBI_NO_HDR
#include "stb-png.c"
#if defined(__clang__)
# pragma clang diagnostic pop
#endif

cod_image* cod_make_image(int width, int height) {
  cod_image* image = COD_ALLOCATE(sizeof(cod_image));
  image->width = width;
  image->height = height;
  image->data = COD_ALLOCATE(width * height * 4);
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

  image = COD_ALLOCATE(sizeof(cod_image));

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

  if(!width) width = src->width;
  if(!height) height = src->height;

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
      
      int alpha = srcp->a;
      int inverse_alpha = 256 - alpha;

      dstp->r = ((srcp->r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = ((srcp->g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = ((srcp->b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      dstp->a = srcp->a;

      ++src_offset;
      ++dst_offset;
    }
  }
}


void cod_draw_over_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {

  if(!width) width = src->width;
  if(!height) height = src->height;

  // Here we truncate the dimensions of the image if it extends over
  // the borders of the source or destination
  width = COD_MIN(dst->width - dst_x, COD_MIN(src->width - src_x, width));
  height = COD_MIN(dst->height - dst_y, COD_MIN(src->height - src_y, height));

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    memcpy(dst->data + dst_offset, src->data + src_offset, width * sizeof(cod_pixel));
  }
}

void cod_simple_draw_image(cod_image* image, int dst_x, int dst_y) {
  cod_draw_image(image, 0, 0, 0, 0, cod_pixels, dst_x, dst_y);
}

