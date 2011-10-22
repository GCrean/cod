// image.c -- cod image support

// TODO: Remove this #if and switch to premultiplied alpha permanently
// Leaving this in ATM because of difficulty with alpha blending so far
#define COD_PREMULTIPLIED_ALPHA 0

#include <stdlib.h>

#ifndef COD_PRIVATE
# define COD_PRIVATE
#endif

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
  cod_image* image = (cod_image*) COD_ALLOCATE(sizeof(cod_image));
  image->width = width;
  image->height = height;
  image->data = (cod_pixel*) COD_ALLOCATE(width * height * COD_BYTES_PER_PIXEL);
  return image;
}

#if COD_PREMULTIPLIED_ALPHA
static void premultiply_alpha(cod_image* image) {
  cod_pixel* newdata = (cod_pixel*) COD_ALLOCATE(image->width * image->height * COD_BYTES_PER_PIXEL);
  cod_pixel* olddata = image->data;
  for(int y = 0; y < image->height; y++) {
    int offset = COD_IMAGE_OFFSET(0, y, image->width);
    for(int x = 0; x < image->width; x++) {

#define MULTIPLY(c) newdata[offset].c = (olddata[offset].c * olddata[offset].a) >> 8
      MULTIPLY(a);
      MULTIPLY(r);
      MULTIPLY(g);
      MULTIPLY(b);
#undef MULTIPLY

      offset++;
    }
  }
  free(olddata);
  image->data = newdata;
}
#endif // COD_PREMULTIPLIED_ALPHA

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
#if COD_PREMULTIPLIED_ALPHA
  premultiply_alpha(image);
#endif

  return image;
}

void cod_free_image(cod_image* image) {
  free(image->data);
  free(image);
}

void cod_draw_image(cod_image* src, int src_x, int src_y, int width,
                    int height, cod_image* dst, int dst_x, int dst_y) {

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

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(int x = 0; x < width; x++) {
      cod_pixel* dstp = dst->data + dst_offset;
      cod_pixel* srcp = src->data + src_offset;

#if COD_PREMULTIPLIED_ALPHA

      dstp->a = ((srcp->a * srcp->a) >> 8) + ((dstp->a * (255 - srcp->a)) >> 8);
#define BLEND(c) dstp->c = srcp->c + ((dstp->c * (255 - srcp->a)) >> 8)
      BLEND(r);
      BLEND(g);
      BLEND(b);
#undef BLEND

#else
      int alpha = srcp->a;
      int inverse_alpha = 255 - alpha;

      dstp->r = ((srcp->r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = ((srcp->g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = ((srcp->b * alpha) + (dstp->b * inverse_alpha)) >> 8;
      dstp->a = srcp->a;
#endif

      ++src_offset;
      ++dst_offset;
    }
  }
}

void cod_draw_image_tinted(cod_image* src, int src_x, int src_y, int width,
                           int height, cod_image* dst, int dst_x, int dst_y,
                           cod_pixel fg) {

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

  for(int y = 0; y < height; y++) {
    int src_offset = COD_IMAGE_OFFSET(src_x, src_y + y, src->width);
    int dst_offset = COD_IMAGE_OFFSET(dst_x, dst_y + y, dst->width);
    for(int x = 0; x < width; x++) {
      cod_pixel* dstp = dst->data + dst_offset;
      cod_pixel* srcp = src->data + src_offset;

      int alpha = (srcp->r + srcp->g + srcp->b) / 3;

      int tint_r = (fg.r * srcp->r) / 255;
      int tint_g = (fg.g * srcp->g) / 255;
      int tint_b = (fg.b * srcp->b) / 255;

#if COD_PREMULTIPLIED_ALPHA
      tint_r = (tint_r * alpha) >> 8;
      tint_g = (tint_g * alpha) >> 8;
      tint_b = (tint_b * alpha) >> 8;
      alpha = (alpha * alpha) >> 8;

      dstp->a = ((alpha * alpha) >> 8) + ((dstp->a * (255 - alpha)) >> 8);
      dstp->r = tint_r + ((dstp->r * (255 - alpha)) >> 8);
      dstp->g = tint_g + ((dstp->g * (255 - alpha)) >> 8);
      dstp->b = tint_b + ((dstp->b * (255 - alpha)) >> 8);
#else
      int inverse_alpha = 255 - alpha;

      dstp->r = (((int)tint_r * alpha) + (dstp->r * inverse_alpha)) >> 8;
      dstp->g = (((int)tint_g * alpha) + (dstp->g * inverse_alpha)) >> 8;
      dstp->b = (((int)tint_b * alpha) + (dstp->b * inverse_alpha)) >> 8;
#endif

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
