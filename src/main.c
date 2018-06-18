/* vim: set expandtab ts=2 sw=2: */
/* main.c

Copyright (C) 1999,2000 Tom Gilbert.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "scrot.h"


int
main(int argc,
     char **argv)
{
  int BLOCKSIZE_X = 16;
  int BLOCKSIZE_Y = 16;
  int RESOLUTION_X = 1280;
  int RESOLUTION_Y = 800;

  Imlib_Image image;
  Imlib_Image thumbnail;
  //Imlib_Load_Error err;

  init_x_and_imlib(NULL, 0);

  image = scrot_grab_part_shot(0, 0, RESOLUTION_X, BLOCKSIZE_Y);
  Imlib_Color colors_top[RESOLUTION_X/BLOCKSIZE_X];
  for(int x = 0, l = RESOLUTION_X/BLOCKSIZE_X; x < l; x++){
    thumbnail =
      gib_imlib_create_cropped_scaled_image(image, x*BLOCKSIZE_X, 0, BLOCKSIZE_X, BLOCKSIZE_Y,
                                            1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_top[x]);
    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_top[x].red, colors_top[x].green, colors_top[x].blue);
  }

  image = scrot_grab_part_shot(0, 0, BLOCKSIZE_X, RESOLUTION_Y);
  Imlib_Color colors_left[RESOLUTION_Y/BLOCKSIZE_X];
  for(int x = 1, l = RESOLUTION_Y/BLOCKSIZE_X-1; x < l; x++){
    thumbnail =
      gib_imlib_create_cropped_scaled_image(image, 0, x*BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_X,
                                            1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_left[x]);
    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_left[x].red, colors_left[x].green, colors_left[x].blue);
  }

  image = scrot_grab_part_shot(RESOLUTION_X-BLOCKSIZE_X, 0, BLOCKSIZE_X, RESOLUTION_Y);
  Imlib_Color colors_right[RESOLUTION_Y/BLOCKSIZE_X];
  for(int x = 1, l = RESOLUTION_Y/BLOCKSIZE_X-1; x < l; x++){
    thumbnail =
      gib_imlib_create_cropped_scaled_image(image, 0, x*BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_X,
                                            1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_right[x]);
    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_right[x].red, colors_right[x].green, colors_right[x].blue);
  }

  image = scrot_grab_part_shot(0, RESOLUTION_Y-BLOCKSIZE_X, RESOLUTION_X, BLOCKSIZE_Y);
  Imlib_Color colors_bottom[RESOLUTION_X/BLOCKSIZE_X];
  for(int x = 0, l = RESOLUTION_X/BLOCKSIZE_X; x < l; x++){
    thumbnail =
     gib_imlib_create_cropped_scaled_image(image, x*BLOCKSIZE_X, 0, BLOCKSIZE_X, BLOCKSIZE_Y,
                                            1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_bottom[x]);
    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_bottom[x].red, colors_bottom[x].green, colors_bottom[x].blue);
  }

  gib_imlib_free_image_and_decache(image);
  return 0;
}

Imlib_Image
scrot_grab_part_shot(int x, int y, int width, int height)
{
  Imlib_Image im;

  XBell(disp, 0);
  im =
    gib_imlib_create_image_from_drawable(root, 0, x, y, width,
                                         height, 1);
  return im;
}
