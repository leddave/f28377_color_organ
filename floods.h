#ifndef FLOODS_H
#define FLOODS_H

//#include "globals.h"

#define HYST_SIZE      3

#ifdef  FIRE_FLOODS
#define FLOOD_ROW      8
#define FLOOD_COL      7
#else
#define FLOOD_ROW      16
#define FLOOD_COL      16
#endif

void init_floods(void);
void reset_hyst(void);
void update_hyst(uint16_t side, uint16_t channels);

void wash(void);
void flood_gap(uint32_t color1, uint32_t color2, uint16_t onoff);
void floods(uint16_t *left_val, uint16_t *right_val);

#endif //FLOODS_H
