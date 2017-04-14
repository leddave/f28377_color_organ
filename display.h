#ifndef DISPLAY_H
#define DISPLAY_H


void display_init(void);
void clear_display(void);
void do_gap_display(void);
void RGB_test(uint32_t count);
void chase(uint32_t loops, uint32_t rate, uint32_t color, uint16_t overwrite);
void line_segments(uint16_t peak_flag, uint16_t *chan_val);
void two_by_two(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val);
void color_bars(uint16_t side, uint16_t *chan_val);
void tetris(uint16_t side, uint16_t blocks, uint16_t peak_flag, uint16_t *chan_val);

void color_organ_prep(float *fft_bin, uint16_t *chan_val);
void initial_display(void);
void pixel_up(void);

void show_text(int16_t dispmode, int16_t panel, uint32_t color, int16_t delay, char *str);

#endif
