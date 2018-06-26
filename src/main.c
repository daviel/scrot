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

#define _GNU_SOURCE

#include <math.h>
#include <stdbool.h>
#include <zlib.h>

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "ws2811.h"
#include "imlib.h"

#include "scrot.h"
#include "bcm_host.h"


void calculate_colors(void);
void calculate_top(void);
void calculate_left(void);
void calculate_right(void);
void calculate_bottom(void);
void init(void);

int BLOCKSIZE_DIVISION_OF_RESOLUTION = 10; // means resolution divided by 10

int RESOLUTION_X = 1920;
int RESOLUTION_Y = 1080;
int TARGET_FPS = 30;

int BOTTOM_START_LED = 204;
int BOTTOM_STOP_LED = 255;

int LEFT_START_LED = 174;
int LEFT_STOP_LED = 204;

int RIGHT_START_LED = 95;
int RIGHT_STOP_LED = 123;

int TOP_START_LED = 123;
int TOP_STOP_LED = 174;


int RIGHT_LENGTH;
int TOP_LENGTH;
int LEFT_LENGTH;
int BOTTOM_LENGTH;

int TOP_BLOCKSIZE_X;
int TOP_BLOCKSIZE_Y;

int BOTTOM_BLOCKSIZE_X;
int BOTTOM_BLOCKSIZE_Y;

int LEFT_BLOCKSIZE_X;
int LEFT_BLOCKSIZE_Y;

int RIGHT_BLOCKSIZE_X;
int RIGHT_BLOCKSIZE_Y;


// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
//#define STRIP_TYPE            WS2811_STRIP_RGB                // WS2812/SK6812RGB integrated chip+leds
#define STRIP_TYPE              WS2811_STRIP_GBR                // WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW               // SK6812RGBW (NOT SK6812RGB)
#define LED_COUNT               300


ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};


Imlib_Image image;
Imlib_Image thumbnail;

Imlib_Color *colors_right;
Imlib_Color *colors_top;
Imlib_Color *colors_left;
Imlib_Color *colors_bottom;

const char* program = "Test";


int
main(int argc,
     char **argv)
{
  printf("Started!\n");

  int result = 0;
  VC_IMAGE_TYPE_T imageType = VC_IMAGE_RGBA32;

  init();
  bcm_host_init();

  DISPMANX_DISPLAY_HANDLE_T displayHandle = vc_dispmanx_display_open(displayNumber);

  if (displayHandle == 0)
  {
      fprintf(stderr,
              "%s: unable to open display %d\n",
              program,
              displayNumber);

      exit(EXIT_FAILURE);
  }

  uint32_t vcImagePtr = 0;
  DISPMANX_RESOURCE_HANDLE_T resourceHandle;
  resourceHandle = vc_dispmanx_resource_create(imageType,
                                               RESOLUTION_X,
                                               RESOLUTION_Y,
                                               &vcImagePtr);

  result = vc_dispmanx_snapshot(displayHandle,
                                 resourceHandle,
                                 DISPMANX_NO_ROTATE);

  VC_RECT_T rect;
  result = vc_dispmanx_rect_set(&rect, 0, 0, 1920, 1080);
  void *dmxImagePtr = malloc(1920 * 1080);

  result = vc_dispmanx_resource_read_data(resourceHandle,
                                          &rect,
                                          dmxImagePtr,
                                          0);


  return 0;

  ws2811_return_t ret;

  if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
  {
    fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
    return ret;
  }

  init_x_and_imlib(NULL, 0);
  imlib_set_cache_size(0);

  struct timespec ts;
  float frame_time = 1/(float) TARGET_FPS;
  ts.tv_sec  = 0;
  ts.tv_nsec = 500000000L;
  double render_time = 0;

  for(int led_num = 0; led_num < 300; led_num++)
    ledstring.channel[0].leds[led_num] = 0;

  while(1){
    clock_t start = clock();
    calculate_colors();
    clock_t end = clock();
    render_time = (double)(end-start) / CLOCKS_PER_SEC;

    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
      fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
      break;
    }

    //printf("Elapsed time: %.6f seconds\n", render_time);

    if(render_time >= frame_time){

    }else{
      ts.tv_nsec = (frame_time - render_time) * 1000000000;
      nanosleep(&ts, NULL);
    }
  }
  return 0;
}


Imlib_Image
scrot_grab_part_shot(int x, int y, int width, int height)
{
  Imlib_Image im;

  XBell(disp, 0);
  im = gib_imlib_create_image_from_drawable(root, 0, x, y, width, height, 1);
  return im;
}


void
calculate_colors(void){
  calculate_top();
  calculate_left();
  calculate_right();
  calculate_bottom();
}

void
init(void){
  RIGHT_LENGTH = RIGHT_STOP_LED - RIGHT_START_LED;
  TOP_LENGTH = TOP_STOP_LED - TOP_START_LED;
  LEFT_LENGTH = LEFT_STOP_LED - LEFT_START_LED;
  BOTTOM_LENGTH = BOTTOM_STOP_LED - BOTTOM_START_LED;

  TOP_BLOCKSIZE_X = RESOLUTION_X / TOP_LENGTH;
  TOP_BLOCKSIZE_Y = RESOLUTION_Y / BLOCKSIZE_DIVISION_OF_RESOLUTION;

  BOTTOM_BLOCKSIZE_X = RESOLUTION_X / BOTTOM_LENGTH;
  BOTTOM_BLOCKSIZE_Y = RESOLUTION_Y / BLOCKSIZE_DIVISION_OF_RESOLUTION;

  LEFT_BLOCKSIZE_X = RESOLUTION_X / BLOCKSIZE_DIVISION_OF_RESOLUTION;
  LEFT_BLOCKSIZE_Y = RESOLUTION_Y / LEFT_LENGTH;

  RIGHT_BLOCKSIZE_X = RESOLUTION_X / BLOCKSIZE_DIVISION_OF_RESOLUTION;
  RIGHT_BLOCKSIZE_Y = RESOLUTION_Y / RIGHT_LENGTH;

  colors_right = (Imlib_Color *) malloc(TOP_LENGTH * sizeof(Imlib_Color));
  colors_top = (Imlib_Color *) malloc(TOP_LENGTH * sizeof(Imlib_Color));
  colors_left = (Imlib_Color *) malloc(TOP_LENGTH * sizeof(Imlib_Color));
  colors_bottom = (Imlib_Color *) malloc(TOP_LENGTH * sizeof(Imlib_Color));
}

void calculate_top(){
  image = scrot_grab_part_shot(0, 0, RESOLUTION_X, TOP_BLOCKSIZE_Y);

  for(int x = 0, l = TOP_LENGTH; x < l; x++){
    thumbnail = gib_imlib_create_cropped_scaled_image(image, x * TOP_BLOCKSIZE_X, 0, TOP_BLOCKSIZE_X, TOP_BLOCKSIZE_Y, 1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_top[x]);

    ledstring.channel[0].leds[TOP_STOP_LED - x] = (int) ((colors_top[x].blue << 16) + (colors_top[x].green << 8) + colors_top[x].red);

    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_top[x].red, colors_top[x].green, colors_top[x].blue);
    gib_imlib_free_image(thumbnail);
  }
  gib_imlib_free_image(image);
}

void calculate_left(){
  image = scrot_grab_part_shot(0, 0, LEFT_BLOCKSIZE_X, RESOLUTION_Y);

  for(int x = 0, l = LEFT_LENGTH; x < l; x++){
    thumbnail = gib_imlib_create_cropped_scaled_image(image, 0, x * LEFT_BLOCKSIZE_Y, LEFT_BLOCKSIZE_X, LEFT_BLOCKSIZE_Y, 1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_left[x]);

    ledstring.channel[0].leds[LEFT_START_LED + x] = (colors_left[x].blue << 16) + (colors_left[x].green << 8) + colors_left[x].red;

    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_left[x].red, colors_left[x].green, colors_left[x].blue);
    gib_imlib_free_image(thumbnail);
  }
  gib_imlib_free_image(image);
}

void calculate_bottom(){
  image = scrot_grab_part_shot(0, RESOLUTION_Y - BOTTOM_BLOCKSIZE_Y, RESOLUTION_X, BOTTOM_BLOCKSIZE_Y);

  for(int x = 0, l = BOTTOM_LENGTH; x < l; x++){
    thumbnail =gib_imlib_create_cropped_scaled_image(image, x * BOTTOM_BLOCKSIZE_X, 0, BOTTOM_BLOCKSIZE_X, BOTTOM_BLOCKSIZE_Y, 1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_bottom[x]);

    ledstring.channel[0].leds[BOTTOM_START_LED + x] = (colors_bottom[x].blue << 16) + (colors_bottom[x].green << 8) + colors_bottom[x].red;

    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_bottom[x].red, colors_bottom[x].green, colors_bottom[x].blue);
    gib_imlib_free_image(thumbnail);
  }
  gib_imlib_free_image(image);
}

void calculate_right(){
  image = scrot_grab_part_shot(RESOLUTION_X - RIGHT_BLOCKSIZE_X, 0, RIGHT_BLOCKSIZE_X, RESOLUTION_Y);

  for(int x = 0, l = RIGHT_LENGTH; x < l; x++){
    thumbnail = gib_imlib_create_cropped_scaled_image(image, 0, x * RIGHT_BLOCKSIZE_Y, RIGHT_BLOCKSIZE_X, RIGHT_BLOCKSIZE_Y, 1, 1, 1);
    imlib_context_set_image(thumbnail);
    imlib_image_query_pixel(0, 0, &colors_right[x]);

    ledstring.channel[0].leds[RIGHT_STOP_LED - x] = (colors_right[x].blue << 16) + (colors_right[x].green << 8) + colors_right[x].red;

    //printf("Pixel %d: rgb: %d %d %d\n", x, colors_right[x].red, colors_right[x].green, colors_right[x].blue);
    gib_imlib_free_image(thumbnail);
  }
  gib_imlib_free_image(image);
}
