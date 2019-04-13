/*
  This file contains beat detection algorithms.
*/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "floods.h"



extern void get_rgb(uint32_t color, uint16_t *r, uint16_t *g, uint16_t *b);
extern void get_rgb_scaled(uint32_t color, uint16_t scale, uint16_t *r, uint16_t *g, uint16_t *b);
extern void wait_for_sync(uint32_t num_syncs);

extern const uint32_t rainbow[12];
extern const uint16_t chan_clr[MAX_CHANNELS+1][MAX_CHANNELS];
extern uint16_t chan_max_left[MAX_CHANNELS];
extern uint16_t chan_max_right[MAX_CHANNELS];

//Locally defined globals:
#ifdef FLOODS
#pragma DATA_SECTION(fled_ping, "leddata")
//#pragma DATA_SECTION(fled_pong, "leddata")
LED_FLOOD  fled_ping[FLOOD_LEDS];
//LED_FLOOD  fled_pong[FLOOD_LEDS];
uint16_t  top_chan_l[MAX_FLOOD_CHAN];  //Color channels for the frame sorted by power
uint16_t  top_chan_l_prev[MAX_FLOOD_CHAN];
uint16_t  top_chan_r[MAX_FLOOD_CHAN];
uint16_t  top_chan_r_prev[MAX_FLOOD_CHAN];
uint16_t  top_chan_l_lvl[MAX_FLOOD_CHAN];
uint16_t  top_chan_r_lvl[MAX_FLOOD_CHAN];
uint16_t  hyst_l[MAX_CHANNELS][HYST_SIZE];
uint16_t  hyst_r[MAX_CHANNELS][HYST_SIZE];
uint16_t  hyst_sum_l[MAX_CHANNELS];
uint16_t  hyst_sum_r[MAX_CHANNELS];
uint16_t  chan_count_l;
uint16_t  chan_count_r;
uint16_t  hyst_idx;
uint16_t  wash_row;
uint16_t  wash_col;
uint16_t  wash_clr_idx;

extern uint16_t  color_channels;

//These globals are defined in the CPU -> CLA message RAM
extern uint16_t  ppong_fill;



#pragma DATA_SECTION(fcol, "ramConsts")

//NOTE: FIRE_FLOODS must only be defined when FLOODS is also defined.

#ifndef FIRE_FLOODS
//Define a square arrangement:
const uint16_t fcol[FLOOD_ROW][FLOOD_COL] = // y rows by x cols
{{ 15, 16, 47, 48, 79, 80, 111, 112, 143, 144, 175, 176, 207, 208, 239, 240},
 { 14, 17, 46, 49, 78, 81, 110, 113, 142, 145, 174, 177, 206, 209, 238, 241},
 { 13, 18, 45, 50, 77, 82, 109, 114, 141, 146, 173, 178, 205, 210, 237, 242},
 { 12, 19, 44, 51, 76, 83, 108, 115, 140, 147, 172, 179, 204, 211, 236, 243},
 { 11, 20, 43, 52, 75, 84, 107, 116, 139, 148, 171, 180, 203, 212, 235, 244},
 { 10, 21, 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 202, 213, 234, 245},
 {  9, 22, 41, 54, 73, 86, 105, 118, 137, 150, 169, 182, 201, 214, 233, 246},
 {  8, 23, 40, 55, 72, 87, 104, 119, 136, 151, 168, 183, 200, 215, 232, 247},
 {  7, 24, 39, 56, 71, 88, 103, 120, 135, 152, 167, 184, 199, 216, 231, 248},
 {  6, 25, 38, 57, 70, 89, 102, 121, 134, 153, 166, 185, 198, 217, 230, 249},
 {  5, 26, 37, 58, 69, 90, 101, 122, 133, 154, 165, 186, 197, 218, 229, 250},
 {  4, 27, 36, 59, 68, 91, 100, 123, 132, 155, 164, 187, 196, 219, 228, 251},
 {  3, 28, 35, 60, 67, 92,  99, 124, 131, 156, 163, 188, 195, 220, 227, 252},
 {  2, 29, 34, 61, 66, 93,  98, 125, 130, 157, 162, 189, 194, 221, 226, 253},
 {  1, 30, 33, 62, 65, 94,  97, 126, 129, 158, 161, 190, 193, 222, 225, 254},
 {  0, 31, 32, 63, 64, 95,  96, 127, 128, 159, 160, 191, 192, 223, 224, 255}};
#else
const Uint16 fcol[FLOOD_ROW][FLOOD_COL] = //8 rows x 7 columns for "fire" lamps
 {{48, 47, 32, 31, 16, 15, 0},
  {49, 46, 33, 30, 17, 14, 1},
  {50, 45, 34, 29, 18, 13, 2},
  {51, 44, 35, 28, 19, 12, 3},
  {52, 43, 36, 27, 20, 11, 4},
  {53, 42, 37, 26, 21, 10, 5},
  {54, 41, 38, 25, 22,  9, 6},
  {55, 40, 39, 24, 23,  8, 7}};
#endif

#pragma CODE_SECTION(reset_hyst, "ramCode")

void reset_hyst(void)
{
  memset(hyst_l, 0, sizeof(hyst_l));
  memset(hyst_r, 0, sizeof(hyst_r));
  memset(hyst_sum_l, 0, sizeof(hyst_sum_l));
  memset(hyst_sum_r, 0, sizeof(hyst_sum_r));
  memset(top_chan_l_lvl, 0, sizeof(top_chan_l_lvl));
  memset(top_chan_r_lvl, 0, sizeof(top_chan_r_lvl));
  hyst_idx = 0;
}


#pragma CODE_SECTION(update_hyst, "ramCode")

void update_hyst(uint16_t side, uint16_t channels)
{
  uint16_t idx, idx2, idx3;
  uint16_t imax;
  uint16_t chan_sort[MAX_CHANNELS];

  //Update the hysteresis buffers
  for (idx = 0; idx < channels; idx++)
  {
    if (side == 0) //left
    {
      //subtract the oldest entry from the sum
      hyst_sum_l[idx] -= hyst_l[idx][hyst_idx];

      //add the new entry to the sum and buffer
      hyst_sum_l[idx] += chan_max_left[idx];
      hyst_l[idx][hyst_idx] = chan_max_left[idx];
    }
    else //right
    {
      //subtract the oldest entry from the sum
      hyst_sum_r[idx] -= hyst_r[idx][hyst_idx];

      //add the new entry to the sum and buffer
      hyst_sum_r[idx] += chan_max_right[idx];
      hyst_r[idx][hyst_idx] = chan_max_right[idx];
    }
  }

  if (side == 1) //right, and now done with both sides so update the index
  {
    hyst_idx ++;
    if (hyst_idx == HYST_SIZE)
      hyst_idx = 0;

    chan_count_r = 0;
  }
  else
    chan_count_l = 0;


  //Sort the strength of the color channels using the hysteresis sums (sort the top two)

  for (idx = 0; idx < MAX_FLOOD_CHAN; idx++)
  {
    imax = 0; //will keep the max found
    idx3 = 0; //will keep the index

    for (idx2 = idx; idx2 < channels; idx2++)
    {
      if (idx == 0)
      {
        if (side == 0)
          chan_sort[idx2] = hyst_sum_l[idx2];
        else
          chan_sort[idx2] = hyst_sum_r[idx2];
      }

      if (chan_sort[idx2] > imax)
      {
        imax = chan_sort[idx2];
        idx3 = idx2; //save the index
      }
    }

    //Now save the max's channel index and clear it from the list
    if (imax > 0)
    {
      if (side == 0)
        chan_count_l ++;
      else
        chan_count_r ++;
    }

    if (side == 0)
      top_chan_l[idx] = idx3;
    else
      top_chan_r[idx] = idx3;

    chan_sort[idx3] = 0; //clear the max from the list
  }
}


void init_floods(void)
{

  memset(fled_ping, 0, sizeof(fled_ping));
//  memset(fled_pong, 0, sizeof(fled_pong));
  reset_hyst();
  wash_row = 9999;
  wash_col = 0;
}


#pragma CODE_SECTION(wash_pix, "ramCode")

void wash_pix(LED_FLOOD *fled, uint16_t flood, uint16_t clr_idx, uint16_t column)
{
  uint32_t  tmp;
  uint16_t  l1;
  uint16_t  r, g, b;
  uint16_t  cl = column;

  tmp = rainbow[clr_idx];
  get_rgb(tmp, &r, &g, &b);

  l1 = fcol[wash_row][cl] + (FLOOD_STRING_LEN * flood);
  fled[l1].r = r;
  fled[l1].g = g;
  fled[l1].b = b;

  cl = FLOOD_COL - (1 + column);

  l1 = fcol[wash_row][cl] + (FLOOD_STRING_LEN * flood);
  fled[l1].r = r;
  fled[l1].g = g;
  fled[l1].b = b;
}


//This function creates a downward flowing color wash.
#pragma CODE_SECTION(wash, "ramCode")

void wash(void)
{
  uint16_t tmp_idx;
  LED_FLOOD *fled;

//  if (ppong_fill == 0)
    fled = &fled_ping[0];
//  else
//    fled = &fled_pong[0];

  //Test for the starting condition
  if (wash_row > FLOOD_ROW)
  {
    wash_row = FLOOD_ROW - 1;
    wash_col = 0;
    wash_clr_idx = rnd(12);
  }

  wash_pix(fled, 0, wash_clr_idx, wash_col); //flood 0

  tmp_idx = wash_clr_idx + 2;
  if (tmp_idx >= 12) tmp_idx -= 12;

  wash_pix(fled, 1, tmp_idx, wash_col); //Flood 2


  wash_col ++;
  if (wash_col == (FLOOD_COL / 2)) //finished the row
  {
    wash_col = 0;
    if (wash_row == 0)
      wash_row = 9999;
    else
      wash_row --;
  }

  wait_for_sync(1);
}


#pragma CODE_SECTION(flood_gap, "ramCode")

void flood_gap(uint32_t color1, uint32_t color2, uint16_t onoff)
{
  int16_t   idx, idx2;
  uint16_t  l1;
  uint16_t  r1, g1, b1;
  uint16_t  r2, g2, b2;
  LED_FLOOD *fled;

//  if (ppong_fill == 0)
    fled = &fled_ping[0];
//  else
//    fled = &fled_pong[0];

  idx = onoff * MAX_RGB_VAL;

  idx = MAX_RGB_VAL - onoff;
  if (idx < MIN_RGB_VAL) idx = MIN_RGB_VAL;

  get_rgb_scaled(color1, idx, &r1, &g1, &b1);
  get_rgb_scaled(color2, idx, &r2, &g2, &b2);

  for (idx = 0; idx < FLOOD_ROW; idx ++)
  {
    for (idx2 = 0; idx2 < FLOOD_COL/2; idx2 ++)
    {
      l1 = fcol[idx][idx2]; //1st flood
      fled[l1].r = r1;
      fled[l1].g = g1;
      fled[l1].b = b1;

      l1 = fcol[idx][idx2] + FLOOD_STRING_LEN; //2nd flood
      fled[l1].r = r1;
      fled[l1].g = g1;
      fled[l1].b = b1;
    }
    for (idx2 = FLOOD_COL/2; idx2 < FLOOD_COL; idx2 ++)
    {
      l1 = fcol[idx][idx2]; //1st flood
      fled[l1].r = r2;
      fled[l1].g = g2;
      fled[l1].b = b2;

      l1 = fcol[idx][idx2] + FLOOD_STRING_LEN; //2nd flood
      fled[l1].r = r2;
      fled[l1].g = g2;
      fled[l1].b = b2;
    }
  }

//  wait_for_sync(1);
}


#pragma CODE_SECTION(floods, "ramCode")

//This function is the main display routine for the flood lights. It takes
//histagrammed and averaged channel data from the prep routine and displays
//the top two channels.
void floods(uint16_t *left_val, uint16_t *right_val)
{
  uint32_t color1, color2;
  uint32_t color3, color4;
  uint16_t idx;
  uint16_t lidx1, lidx2;
  uint16_t ridx1, ridx2;
  uint16_t rw, cl;
  uint16_t r1, g1, b1, cv1;
  uint16_t r2, g2, b2, cv2;
  LED_FLOOD *fled;

//  if (ppong_fill == 0)
    fled = &fled_ping[0];
//  else
//    fled = &fled_pong[0];

//Future updates: ?
//if largest is > 4x any other, then use for both halves.
//update every frame?

  //To reduce flicker, see if the top 2 simply swapped sides
  if ((top_chan_l[0] == top_chan_l_prev[1]) ||
      (top_chan_l[1] == top_chan_l_prev[0]))
  {
    idx = top_chan_l[0];
    top_chan_l[0] = top_chan_l_prev[1];
    top_chan_l[1] = idx;
  }
  if ((top_chan_r[0] == top_chan_r_prev[1]) ||
      (top_chan_r[1] == top_chan_r_prev[0]))
  {
    idx = top_chan_r[0];
    top_chan_r[0] = top_chan_r_prev[1];
    top_chan_r[1] = idx;
  }

  switch (chan_count_l)
  {
    case 0: //no channels this frame - keep the previous one(s)
      lidx1  = top_chan_l_prev[0];
      lidx2  = top_chan_l_prev[1];
      color1 = rainbow[chan_clr[color_channels][lidx1]];
      color2 = rainbow[chan_clr[color_channels][lidx2]];
      idx = top_chan_l_lvl[0];
      top_chan_l_lvl[0] = (idx >> 1) + (idx >> 2);
      idx = top_chan_l_lvl[1];
      top_chan_l_lvl[1] = (idx >> 1) + (idx >> 2);
      break;

    case 1: //just one channel this frame
      lidx1  = top_chan_l[0];
      lidx2  = top_chan_l[1];
      color1 = rainbow[chan_clr[color_channels][lidx1]];
      color2 = color1;

      for (idx = 0; idx < MAX_FLOOD_CHAN; idx++)
        top_chan_l_prev[idx] = top_chan_l[idx];
      break;

    case 2: //two or more channels this frame
      lidx1  = top_chan_l[0];
      lidx2  = top_chan_l[1];
      color1 = rainbow[chan_clr[color_channels][lidx1]];
      color2 = rainbow[chan_clr[color_channels][lidx2]];

      for (idx = 0; idx < MAX_FLOOD_CHAN; idx++)
        top_chan_l_prev[idx] = top_chan_l[idx];
      break;
  }

  switch (chan_count_r)
  {
    case 0: //no channels this frame - keep the previous one(s)
      ridx1  = top_chan_r_prev[0];
      ridx2  = top_chan_r_prev[1];
      color3 = rainbow[chan_clr[color_channels][ridx1]];
      color4 = rainbow[chan_clr[color_channels][ridx2]];
      idx = top_chan_r_lvl[0];
      top_chan_r_lvl[0] = (idx >> 1) + (idx >> 2);
      idx = top_chan_r_lvl[1];
      top_chan_r_lvl[1] = (idx >> 1) + (idx >> 2);
      break;

    case 1: //just one channel this frame
      ridx1  = top_chan_r[0];
      ridx2  = top_chan_r[1];
      color3 = rainbow[chan_clr[color_channels][ridx1]];
      color4 = color3;

      for (idx = 0; idx < MAX_FLOOD_CHAN; idx++)
        top_chan_r_prev[idx] = top_chan_r[idx];
      break;

    case 2: //two or more channels this frame
      ridx1  = top_chan_r[0];
      ridx2  = top_chan_r[1];
      color3 = rainbow[chan_clr[color_channels][ridx1]];
      color4 = rainbow[chan_clr[color_channels][ridx2]];

      for (idx = 0; idx < MAX_FLOOD_CHAN; idx++)
        top_chan_r_prev[idx] = top_chan_r[idx];
      break;
  }

  for (cl = 0; cl < FLOOD_COL; cl ++)
  {
    if (cl == 0)
    {
      //Scale the brightness to the current channel intensity
      //Get the base color for this side
      get_rgb(color1, &r1, &g1, &b1);
      get_rgb(color3, &r2, &g2, &b2);

      cv1 = left_val[lidx1];
      if (cv1 < MIN_RGB_VAL)
        cv1 = MIN_RGB_VAL;

      cv2 = right_val[ridx1];
      if (cv2 < MIN_RGB_VAL)
        cv2 = MIN_RGB_VAL;

      //If this side of the panel drops > 25% keep it at 25%
      if (cv1 < top_chan_l_lvl[0])
        cv1 = top_chan_l_lvl[0];

      if (cv2 < top_chan_r_lvl[0])
        cv2 = top_chan_r_lvl[0];

      top_chan_l_lvl[0] = (cv1 >> 1) + (cv1 >> 2);
      top_chan_r_lvl[0] = (cv2 >> 1) + (cv2 >> 2);

      r1 = (r1 * cv1) >> 8;
      g1 = (g1 * cv1) >> 8;
      b1 = (b1 * cv1) >> 8;

      r2 = (r2 * cv2) >> 8;
      g2 = (g2 * cv2) >> 8;
      b2 = (b2 * cv2) >> 8;
    }
    else if (cl == (FLOOD_COL / 2))
    {
      //Scale the brightness to the current channel intensity
      //Get the base color for this side
      get_rgb(color2, &r1, &g1, &b1);
      get_rgb(color4, &r2, &g2, &b2);

      cv1 = left_val[lidx2];
      if (cv1 < MIN_RGB_VAL)
        cv1 = MIN_RGB_VAL;

      cv2 = right_val[ridx2];
      if (cv2 < MIN_RGB_VAL)
        cv2 = MIN_RGB_VAL;

      //If this side of the panel drops > 25% keep it at 25%
      if (cv1 < top_chan_l_lvl[1])
        cv1 = top_chan_l_lvl[1];

      if (cv2 < top_chan_r_lvl[1])
        cv2 = top_chan_r_lvl[1];

      top_chan_l_lvl[1] = (cv1 >> 1) + (cv1 >> 2);
      top_chan_r_lvl[1] = (cv2 >> 1) + (cv2 >> 2);

      r1 = (r1 * cv1) >> 8;
      g1 = (g1 * cv1) >> 8;
      b1 = (b1 * cv1) >> 8;

      r2 = (r2 * cv2) >> 8;
      g2 = (g2 * cv2) >> 8;
      b2 = (b2 * cv2) >> 8;
    }

    for (rw = 0; rw < FLOOD_ROW; rw ++)
    {
      idx = fcol[rw][cl];
      fled[idx].r = r1; //left channel, flood 1
      fled[idx].g = g1;
      fled[idx].b = b1;

      idx += FLOOD_STRING_LEN;
      fled[idx].r = r2; //right channel, flood 1
      fled[idx].g = g2;
      fled[idx].b = b2;
    }
  }
}
#endif
