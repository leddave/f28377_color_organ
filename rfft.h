#ifndef RFFT_H
#define RFFT_H

#include "globals.h"

#define RFFT_STAGES      9
#define RFFT_SIZE       (1 << RFFT_STAGES)


void fft_init(void);
void fft_calc(void);

#endif
