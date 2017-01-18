/*****************************************************************************
 * This is the rgb_f28377 project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "display.h"
#include "led_driver.h"
#include "utils.h"
#include "font.h"


//Global variables
Led       led[MAX_LEDS];
//Led       tled[MAX_LEDS]; //temp work buff
extern volatile uint16_t frame_sync;


//Define a long (wide) array of LED panels (the current design is for each panel
//to be a separate string of LEDs, driven by a separate Launchpad GPIO).
#define NUM_PANEL    NUM_STRINGS
#ifdef TEST_ARRAY
#define PANEL_ROW    8
#define PANEL_COL    7
#else
#define PANEL_ROW    10
#define PANEL_COL    12
#endif
#define MAX_ROW      PANEL_ROW
#define MAX_COL     (PANEL_COL * NUM_PANEL)


// The following tables are built to convert a 2D x:y position into an index
// in the 1D array of LEDs.

#pragma DATA_SECTION(col, "ramConsts")

#ifdef TEST_ARRAY
//Define an array that matches the fire lamp array:
const uint16_t col[MAX_ROW][MAX_COL] = //y rows, up to x each
 {{48, 47, 32, 31, 16, 15, 0,  56, 56, 56, 56, 56, 56, 56,  160, 159, 144, 143, 128, 127, 112,  56, 56, 56, 56, 56, 56, 56},
  {49, 46, 33, 30, 17, 14, 1,  56, 56, 56, 56, 56, 56, 56,  161, 158, 145, 142, 129, 126, 113,  56, 56, 56, 56, 56, 56, 56},
  {50, 45, 34, 29, 18, 13, 2,  56, 56, 56, 56, 56, 56, 56,  162, 157, 146, 141, 130, 125, 114,  56, 56, 56, 56, 56, 56, 56},
  {51, 44, 35, 28, 19, 12, 3,  56, 56, 56, 56, 56, 56, 56,  163, 156, 147, 140, 131, 124, 115,  56, 56, 56, 56, 56, 56, 56},
  {52, 43, 36, 27, 20, 11, 4,  56, 56, 56, 56, 56, 56, 56,  164, 155, 148, 139, 132, 123, 116,  56, 56, 56, 56, 56, 56, 56},
  {53, 42, 37, 26, 21, 10, 5,  56, 56, 56, 56, 56, 56, 56,  165, 154, 149, 138, 133, 122, 117,  56, 56, 56, 56, 56, 56, 56},
  {54, 41, 38, 25, 22,  9, 6,  56, 56, 56, 56, 56, 56, 56,  166, 153, 150, 137, 134, 121, 118,  56, 56, 56, 56, 56, 56, 56},
  {55, 40, 39, 24, 23,  8, 7,  56, 56, 56, 56, 56, 56, 56,  167, 152, 151, 136, 135, 120, 119,  56, 56, 56, 56, 56, 56, 56}};
#else
//Define a "wide" 1x4 panel arrangement:
const uint16_t col[MAX_ROW][MAX_COL] = // y rows by x cols
 //panel 0                                                     panel 1                                                      panel 2                                                      panel 3
{{ 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120,  251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240,  371, 370, 369, 368, 367, 366, 365, 364, 363, 362, 361, 360},
 { 12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,  372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383},
 { 35,  34,  33,  32,  31,  30,  29,  28,  27,  26,  25,  24,  155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,  275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,  395, 394, 393, 392, 391, 390, 389, 388, 387, 386, 385, 384},
 { 36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,  276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287,  396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407},
 { 59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,  179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168,  299, 298, 297, 296, 295, 294, 293, 292, 291, 290, 289, 288,  419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408},
 { 60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311,  420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431},
 { 83,  82,  81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192,  323, 322, 321, 320, 319, 318, 317, 316, 315, 314, 313, 312,  443, 442, 441, 440, 439, 438, 437, 436, 435, 434, 433, 432},
 { 84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,  324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335,  444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455},
 {107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96,  227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216,  347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336,  467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 457, 456},
 {108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,  228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359,  468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479}};
#endif


//This determines the panel's position in the logical [x, y] array.
const uint16_t panel_off[NUM_PANEL][2] =
{{        0,   0},  //panel 0: x=0,  y=0
 {PANEL_COL,   0},  //panel 1: x=12, y=0
 {PANEL_COL*2, 0},  //panel 2: x=24, y=0
 {PANEL_COL*3, 0}}; //panel 3: x=36, y=0


#pragma DATA_SECTION(rainbow, "ramConsts")
#define MAX_RGB_VAL     0xe0

//The function get_rgb() can be used to break these colors into component values.
const uint32_t rainbow[12] = {0xe00000,  //red
                              0xe02000,  //orange
                              0xe0e000,  //yellow
                              0x60e000,  //lime
                              0x00e000,  //green
                              0x00e030,  //l green
                              0x00e090,  //cyan
                              0x0070b0,  //marine
                              0x0000d0,  //blue
                              0x7000e0,  //l blue
                              0x800070,  //purple
                              0xe00020}; //fushia


#pragma DATA_SECTION(chan_clr, "ramConsts")
#pragma DATA_SECTION(chan_bin, "ramConsts")

/*  Nine channel, one octave per channel color organ
    Frequency ranges: (44Khz sample rate, 86Hz per bin)

    Octave 1:      0.. 85Hz    bin: 0         color: Red
    Octave 2:     86..171Hz    bin: 1         color: Fushia
    Octave 3:    172..343Hz    bin: 2..3      color: Purple
    Octave 4:    344..687Hz    bin: 4..7      color: Blue
    Octave 5:    688..1375Hz   bin: 8..15     color: Aqua
    Octave 6:   1376..2751Hz   bin: 16..31    color: Cyan
    Octave 7:   2752..5503Hz   bin: 32..63    color: Green
    Octave 8:   5504..11007Hz  bin: 64..127   color: Lime
    Octave 9+: 11008..20000Hz  bin: 128..232  color: Yellow

    Or, three channel color organ frequency ranges:

    Low:          0..257Hz    bin: 0..3      color: Red
    Mid:        258..3439Hz   bin: 4..39     color: Blue
    High:      3440..20000Hz  bin: 40..232   color: Green
*/
#ifdef NINE_OCTAVE_BIN
const uint16_t chan_bin[COLOR_CHANNELS+1] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 233};

//This array maps into the rainbow[] array to assign colors to each color organ channel.
const uint16_t chan_clr[COLOR_CHANNELS] = {0, 11, 10, 8, 7, 6, 4, 3, 2};
#endif

/*  Ten channel compressed frequency range color organ
    Frequency ranges: (44Khz sample rate, 86Hz per bin)

    Channel 1:      0.. 85Hz    bin: 0         color: Red
    Channel 2:     86..171Hz    bin: 1         color: Orange
    Channel 3:    172..429Hz    bin: 2..4      color: Purple
    Channel 4:    430..1203Hz   bin: 5..13     color: Blue
    Channel 5:   1204..1719Hz   bin: 14..19    color: Aqua
    Channel 6:   1720..2321Hz   bin: 20..26    color: Cyan
    Channel 7:   2322..3009Hz   bin: 27..34    color: L Green
    Channel 8:   3010..6019Hz   bin: 35..69    color: Green
    Channel 9:   6020..9029Hz   bin: 70..104   color: Lime
    Channel 10:  9030..15000Hz  bin: 105..174  color: Yellow
*/
const uint16_t chan_bin[COLOR_CHANNELS+1] = {0, 1, 2, 5, 14, 20, 27, 35, 70, 105, 175};

//This array maps into the rainbow[] array to assign colors to each color organ channel.
const uint16_t chan_clr[COLOR_CHANNELS] = {0, 1, 11, 8, 7, 6, 5, 4, 3, 2};


void display_init(void)
{

  memset(led, 0, sizeof(led));
}


//This function blocks for the specified number of frame syncs to
//expire.
void wait_for_sync(uint32_t num_syncs)
{
  uint16_t idx;

  for (idx = 0; idx < num_syncs; idx ++)
  {
    while (frame_sync == 0)
    {  }

    frame_sync = 0;
  }
}


#pragma CODE_SECTION(get_rgb, "ramCode")

//This function returns the R G B components of a 32-bit color value.
void get_rgb(uint32_t color, uint16_t *r, uint16_t *g, uint16_t *b)
{
  *r = (color >> 16) & 0xff;
  *g = (color >>  8) & 0xff;
  *b =  color        & 0xff;
}


#pragma CODE_SECTION(get_rgb_scaled, "ramCode")

//This function returns the R G B components of a scaled 32-bit color value.
void get_rgb_scaled(uint32_t color, uint16_t scale, uint16_t *r, uint16_t *g, uint16_t *b)
{
  float temp = (float)scale / (float)MAX_RGB_VAL;

  get_rgb(color, r, g, b);

  *r = (uint16_t) ((float)*r * temp);
  *g = (uint16_t) ((float)*g * temp);
  *b = (uint16_t) ((float)*b * temp);
}


#pragma CODE_SECTION(checkerboard, "ramCode")

void checkerboard(uint32_t color1, uint32_t color2)
{
  uint16_t x, y, idx, color;
  uint16_t r, g, b;

  for (y = 0; y < MAX_ROW; y += 2)
  {
    color = (y / 2) & 0x01;

    for (x = 0; x < MAX_COL; x += 2)
    {
      if (color)
      {
        get_rgb(color1, &r, &g, &b);
        color = 0;
      }
      else
      {
        get_rgb(color2, &r, &g, &b);
        color = 1;
      }

      idx = col[y][x];
      led[idx].r = r;
      led[idx].g = g;
      led[idx].b = b;

      idx = col[y][x+1];
      led[idx].r = r;
      led[idx].g = g;
      led[idx].b = b;

      idx = col[y+1][x];
      led[idx].r = r;
      led[idx].g = g;
      led[idx].b = b;

      idx = col[y+1][x+1];
      led[idx].r = r;
      led[idx].g = g;
      led[idx].b = b;
    }
  }
}


#pragma CODE_SECTION(RGB_test, "ramCode")

//This function creates a test pattern to verify operation of the LEDs and
//the driver software.
void RGB_test(uint32_t count)
{
  uint16_t idx1;

  for (idx1 = 0; idx1 < count; idx1 ++)
  {
    //All Red:
    checkerboard(rainbow[0], 0);
    wait_for_sync(15);
    checkerboard(0, rainbow[0]);
    wait_for_sync(15);

    //All Green:
    checkerboard(rainbow[4], 0);
    wait_for_sync(15);
    checkerboard(0, rainbow[4]);
    wait_for_sync(15);

    //All Blue:
    checkerboard(rainbow[8], 0);
    wait_for_sync(15);
    checkerboard(0, rainbow[8]);
    wait_for_sync(15);

    //All White:
    checkerboard(BAL_WHITE, 0);
    wait_for_sync(15);
    checkerboard(0, BAL_WHITE);
    wait_for_sync(15);
  }
}


#pragma CODE_SECTION(chase, "ramCode")

//This function displays a chasing pixel with a fading 3 LED tail.  If "overwrite"
//is NULL, it will not change pixels that are currently black (off).
void chase(uint32_t loops, uint32_t rate, uint32_t color, uint16_t overwrite)
{
  uint16_t idx1, curr, tmp;
  uint16_t r, g, b;
  int16_t  x, y, idx3;
  uint32_t row[MAX_COL];

  get_rgb(color, &r, &g, &b);

  for (idx1 = 0; idx1 < loops; idx1 ++)
  {
    for (y = 0; y < MAX_ROW; y ++)
    {
      curr = col[y][x];
      tmp  = curr;

      //save the state of the row.
      for (x = 0; x < MAX_COL; x ++)
      {
        tmp = col[y][x];
        row[x] = led[tmp].r + led[tmp].g + led[tmp].b;
      }

      for (x = 0; x < (MAX_COL+4); x ++)
      {
        //Give it a fading tail
        for (idx3 = x-3; idx3 <= x; idx3 ++)
        {
          if ((idx3 >= 0) && (idx3 < MAX_COL))
          {
            if ((overwrite) || (row[idx3] == 0))
            {
              tmp = col[y][idx3];
              led[tmp].r = r >> ((x-idx3)*2);
              led[tmp].g = g >> ((x-idx3)*2);
              led[tmp].b = b >> ((x-idx3)*2);
            }
          }
        }

        if (((x-4) >= 0) && (x < MAX_COL))
        {
          if ((overwrite) || (row[x-4] == 0))
          {
            tmp = col[y][x-4];
            led[tmp].r = 0;
            led[tmp].g = 0;
            led[tmp].b = 0;
          }
        }

        wait_for_sync(rate);
      }
    }
  }
}


#pragma CODE_SECTION(copy_pixel, "ramCode")
void copy_pixel(uint16_t dest, uint16_t src)
{
  led[dest].r = led[src].r;
  led[dest].g = led[src].g;
  led[dest].b = led[src].b;
}


//This function creates a downward flowing pixel pattern.
#pragma CODE_SECTION(pixel_up, "ramCode")

void pixel_up(void)
{
  uint32_t  tmp;
  uint16_t  l1, l2;
  uint16_t  r, g, b;
  int16_t   rw, cl;

  while (1)
  {
//    if (button_pushed)
//      return;

    //Move all rows down by 1.
    for (rw = 0; rw < MAX_ROW-1; rw ++)
    {
      for (cl = 0; cl < MAX_COL; cl ++)
      {
        l1 = col[rw+1][cl];
        if ((cl+1) < MAX_COL)
          l2 = col[rw][cl+1];
        else
          l2 = col[rw][(cl+1)-MAX_COL];

        copy_pixel(l2, l1);
      }
    }

    //Generate a new row on the top
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      l1 = col[MAX_ROW-1][cl];

      tmp = rnd(22);
      if (tmp < 12)
      {
        if (rnd(100) > 35)
          tmp = rainbow[tmp & 0xc]; //only R, G or B
        else
          tmp = rainbow[tmp];
      }
      else
        tmp = 0; //off

      get_rgb(tmp, &r, &g, &b);
      led[l1].r = r;
      led[l1].g = g;
      led[l1].b = b;
    }

    wait_for_sync(2);
  }
}


#pragma CODE_SECTION(color_bars, "ramCode")

//This function creates a simple color organ display with one row of LEDs
//given to each channel.  Channel 0 (lowest frequencies) will go to Row 0.
void color_bars(uint16_t side, uint16_t *chan_val)
{
  uint32_t color;
  uint16_t idx, idx2, idx3;
  uint16_t r, g, b, cv;
  uint16_t side_offset;

  if (side == LEFT)
    side_offset = 0;
  else
    side_offset = PANEL_COL * 2;


  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    if (idx < MAX_ROW)
    {
      //Get the base color
      color = rainbow[chan_clr[idx]];
      get_rgb(color, &r, &g, &b);

      //Scale the brightness to the current channel intensity
      cv = chan_val[idx];
      r = (r * cv) >> 8;
      g = (g * cv) >> 8;
      b = (b * cv) >> 8;

      //Paint the row with it
      for (idx2 = 0; idx2 < PANEL_COL*2; idx2 ++)
      {
        idx3 = col[idx][idx2+side_offset];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;
      }
    }
  }

//  wait_for_sync(1);
}


#define HIGH_PASS     0x10
#pragma CODE_SECTION(color_organ_prep, "ramCode")

//This function prepares the FFT output bins for the display routines.
void color_organ_prep(float *fft_bin, uint16_t *chan_val)
{
  uint16_t idx, idx2, idx3, end;
  float  temp, max;
  float  maxlo, maxmid, maxhi;

  idx3  = 1;
  maxhi = 0;
  maxmid= 0;
  maxlo = 0;

  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    max = 0;
    end = chan_bin[idx+1]; //this depends on one extra entry in the array

    for (idx2 = chan_bin[idx]; idx2 < end; idx2 ++)
    {
      //Square the data to make more difference between large and small values
      temp = (fft_bin[idx3] * fft_bin[idx3]);

      if (max < temp)
        max = temp;
      idx3 += 1;
    }

    chan_val[idx] = (uint16_t) max;

    if (idx < 3) //low freq channels
    {
      if (maxlo < max)
        maxlo = max;
    }
    else
    if (idx < 6) //mid freq channels
    {
      if (maxmid < max)
        maxmid = max;
    }
    else //high freq channels
    {
      if (maxhi < max)
        maxhi = max;
    }

    //Don't pass very small values through.
    if (chan_val[idx] < HIGH_PASS)
      chan_val[idx] = 0;
  }


  //Scale the data to the maximum component RGB value.
  if (maxlo > (float)MAX_RGB_VAL)
  {
    temp = maxlo / (float)MAX_RGB_VAL;

    for (idx = 0; idx < 3; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxmid > (float)MAX_RGB_VAL)
  {
    temp = maxmid / (float)MAX_RGB_VAL;

    for (idx = 3; idx < 6; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxhi > (float)MAX_RGB_VAL)
  {
    temp = maxhi / (float)MAX_RGB_VAL;

    for (idx = 6; idx < COLOR_CHANNELS; idx++)
    {
      chan_val[idx] /= temp;
    }
  }
}


#pragma CODE_SECTION(draw_char, "ramCode")

//Draw a character starting at lower-left x,y, and clip to the array.
void draw_char(int16_t x, int16_t y, int16_t idx, uint32_t color)
{
  int16_t   ix, iy, clm, tmp;
  uint16_t  bits;
  uint16_t r, g, b;
  font_t *fchr = (font_t *) &fontchar[idx];

  get_rgb(color, &r, &g, &b);
  clm = 0;

  for (ix = x; ix < x + fchr->wid; ix++)
  {
    bits = fchr->pix[clm / 2]; //grab the right 16 bits
    if (clm & 0x01)
      bits &= 0xff; //keep 2nd byte
    else
      bits >>= 8;   //keep 1st byte

    for (iy = y; iy < y + FONT_HEIGHT; iy++)
    {
      if ((ix < MAX_COL) && (iy < MAX_ROW))
      {
        if (bits & 0x01)
        {
          tmp = col[iy][ix];
          led[tmp].r = r;
          led[tmp].g = g;
          led[tmp].b = b;
        }

        bits >>= 1;
      }
    }

    clm ++;
  }
}


#pragma CODE_SECTION(show_text, "ramCode")

// panel >= 0 means show just in that panel
//dispmode = 0 = centered in panel
//         = 1 = scroll from right
void show_text(int16_t panel, int16_t dispmode, uint32_t color, char *str)
{
  int16_t  x, y;
  uint16_t idx, s_len, p_len;
  uint16_t str_idx[FONT_MAX_CHAR];

  s_len = font_str_len(str);
  p_len = font_pix_len(str, str_idx);

  if (dispmode == 0)
  {
    if (panel >= 0)
    {
      if (p_len > PANEL_COL) //won't fit
        return;

      x = panel_off[panel][0] + (PANEL_COL/2) - (p_len/2);
      y = panel_off[panel][1] + (PANEL_ROW/2) - (FONT_HEIGHT/2);
    }
    else //center in the overall x,y array
    {
      x = MAX_COL - 1;
      y =(MAX_ROW / 2) + panel_off[panel][1] + 1;
    }

    for (idx = 0; idx < s_len; idx++)
    {
      draw_char(x, y, str_idx[idx], color);
      x += fontchar[str_idx[idx]].wid + 1;
    }

    wait_for_sync(1);
  }
}


//This function performs power on (or reset) displays to verify that the LED
//panels are correctly operating prior to the color organ running.
void initial_display(void)
{
  char str[2];

  RGB_test(1);
  memset(led, 0, sizeof(led));
  wait_for_sync(1);

  //Display the panel number in each panel to identify correct hookup.
  str[0] = '1'; str[1] = 0;
  show_text(0, 0, 0x0000a0, str);
  str[0] = '2';
  show_text(1, 0, 0x0000a0, str);
  str[0] = '3';
  show_text(2, 0, 0x0000a0, str);
  str[0] = '4';
  show_text(3, 0, 0x0000a0, str);
  wait_for_sync(15);

  chase(1, 1, 0xe00000, 0);
}
