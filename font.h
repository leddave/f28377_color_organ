/* This file defines an 8 row by 6 column font


 */
#ifndef FONT_H
#define FONT_H

#include "globals.h"

#define FONT_HEIGHT                    8
#define FONT_WIDTH                     6
#define NUM_FONT_CHAR                  74
#define FONT_MAX_CHAR                  50

typedef struct
{
  uint16_t asc:8;  //ASCII code for the character
  uint16_t wid:8;  //number of columns with 1 pixels.
  uint16_t pix[3]; //byte 0 contains column zero, top down.
} font_t;


extern const font_t fontchar[NUM_FONT_CHAR];


uint16_t font_str_len(char *str);
uint16_t font_pix_len(char *str, uint16_t *str_idx);
uint16_t font_get_idx(uint16_t code);

#endif
