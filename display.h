#ifndef DISPLAY_H
#define DISPLAY_H


void display_init(void);
void RGB_test(uint32_t count);
void chase(uint32_t loops, uint32_t rate, uint32_t color, uint16_t overwrite);
void two_by_two(uint16_t side, uint16_t peak_flag, uint16_t *chan_val);
void color_bars(uint16_t side, uint16_t *chan_val);

void color_organ_prep(float *fft_bin, uint16_t *chan_val);
void initial_display(void);
void pixel_up(void);

#endif
