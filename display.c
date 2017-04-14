/*****************************************************************************
 * This is the f28377_color_organ project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "display.h"
#include "led_driver.h"
#include "utils.h"
#include "font.h"


//Define the overall array of LED panels. The current design is for each panel
//to be a separate string of LEDs, driven by a separate Launchpad GPIO. For this
//reason, it is important that these definitions match with the global variables
//col[][] and panel_off[][].
#define NUM_PANEL    NUM_STRINGS

#ifdef  LARGE_ARRAY
#define PANEL_ROW    8
#define PANEL_COL    36
#define TBT_STEP     3
#define MAX_ROW     (PANEL_ROW * 2)
#define MAX_COL     (PANEL_COL * 2)

#else
#define PANEL_ROW    10
#define PANEL_COL    12
#define TBT_STEP     2
#define MAX_ROW      PANEL_ROW
#define MAX_COL     (PANEL_COL * NUM_PANEL)
#endif

#define TBT_ROW          (MAX_ROW / TBT_STEP)
#define TBT_COL          (MAX_COL / TBT_STEP)

#define TBT_CHAN_BLOCKS  (TBT_ROW * TBT_COL / COLOR_CHANNELS / 2)


#define OLD_STYLE_BULBS


//Global variables
Led       led[MAX_LEDS];       //the array of color information used by the LED driver
uint32_t  work_buff[MAX_LEDS]; //temporary work buffer (can be used by all displays)
uint32_t  disp_frames;

//Temporary storage for the two-by-two and tetris functions
uint16_t tbt_map[TBT_ROW][TBT_COL];

extern uint16_t pause;
extern volatile uint16_t frame_sync;
extern volatile uint16_t end_of_gap;
extern volatile uint32_t gap_frames;


// The following tables are built to convert a 2D y:x position into an index
// in the 1D arrays of LEDs.

#pragma DATA_SECTION(col, "ramConsts")

#ifdef LARGE_ARRAY
//Define a "wide" 2x2 panel arrangement:
const uint16_t col[MAX_ROW][MAX_COL] = // y rows by x cols
 //panel 0                                                                                                                                                                             panel 2
{{  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,   576,  577,  578,  579,  580,  581,  582,  583,  584,  585,  586,  587,  588,  589,  590,  591,  592,  593,  594,  595,  596,  597,  598,  599,  600,  601,  602,  603,  604,  605,  606,  607,  608,  609,  610,  611},
 { 71,  70,  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,  37,  36,   647,  646,  645,  644,  643,  642,  641,  640,  639,  638,  637,  636,  635,  634,  633,  632,  631,  630,  629,  628,  627,  626,  625,  624,  623,  622,  621,  620,  619,  618,  617,  616,  615,  614,  613,  612},
 { 72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107,   648,  649,  650,  651,  652,  653,  654,  655,  656,  657,  658,  659,  660,  661,  662,  663,  664,  665,  666,  667,  668,  669,  670,  671,  672,  673,  674,  675,  676,  677,  678,  679,  680,  681,  682,  683},
 {143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108,   719,  718,  717,  716,  715,  714,  713,  712,  711,  710,  709,  708,  707,  706,  705,  704,  703,  702,  701,  700,  699,  698,  697,  696,  695,  694,  693,  692,  691,  690,  689,  688,  687,  686,  685,  684},
 {144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,   720,  721,  722,  723,  724,  725,  726,  727,  728,  729,  730,  731,  732,  733,  734,  735,  736,  737,  738,  739,  740,  741,  742,  743,  744,  745,  746,  747,  748,  749,  750,  751,  752,  753,  754,  755},
 {215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180,   791,  790,  789,  788,  787,  786,  785,  784,  783,  782,  781,  780,  779,  778,  777,  776,  775,  774,  773,  772,  771,  770,  769,  768,  767,  766,  765,  764,  763,  762,  761,  760,  759,  758,  757,  756},
 {216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,   792,  793,  794,  795,  796,  797,  798,  799,  800,  801,  802,  803,  804,  805,  806,  807,  808,  809,  810,  811,  812,  813,  814,  815,  816,  817,  818,  819,  820,  821,  822,  823,  824,  825,  826,  827},
 {287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264, 263, 262, 261, 260, 259, 258, 257, 256, 255, 254, 253, 252,   863,  862,  861,  860,  859,  858,  857,  856,  855,  854,  853,  852,  851,  850,  849,  848,  847,  846,  845,  844,  843,  842,  841,  840,  839,  838,  837,  836,  835,  834,  833,  832,  831,  830,  829,  828},

 //panel 1                                                                                                                                                                             panel 3
 {288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323,   864,  865,  866,  867,  868,  869,  870,  871,  872,  873,  874,  875,  876,  877,  878,  879,  880,  881,  882,  883,  884,  885,  886,  887,  888,  889,  890,  891,  892,  893,  894,  895,  896,  897,  898,  899},
 {359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330, 329, 328, 327, 326, 325, 324,   935,  934,  933,  932,  931,  930,  929,  928,  927,  926,  925,  924,  923,  922,  921,  920,  919,  918,  917,  916,  915,  914,  913,  912,  911,  910,  909,  908,  907,  906,  905,  904,  903,  902,  901,  900},
 {360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395,   936,  937,  938,  939,  940,  941,  942,  943,  944,  945,  946,  947,  948,  949,  950,  951,  952,  953,  954,  955,  956,  957,  958,  959,  960,  961,  962,  963,  964,  965,  966,  967,  968,  969,  970,  971},
 {431, 430, 429, 428, 427, 426, 425, 424, 423, 422, 421, 420, 419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 402, 401, 400, 399, 398, 397, 396,  1007, 1006, 1005, 1004, 1003, 1002, 1001, 1000,  999,  998,  997,  996,  995,  994,  993,  992,  991,  990,  989,  988,  987,  986,  985,  984,  983,  982,  981,  980,  979,  978,  977,  976,  975,  974,  973,  972},
 {432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467,  1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043},
 {503, 502, 501, 500, 499, 498, 497, 496, 495, 494, 493, 492, 491, 490, 489, 488, 487, 486, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 474, 473, 472, 471, 470, 469, 468,  1079, 1078, 1077, 1076, 1075, 1074, 1073, 1072, 1071, 1070, 1069, 1068, 1067, 1066, 1065, 1064, 1063, 1062, 1061, 1060, 1059, 1058, 1057, 1056, 1055, 1054, 1053, 1052, 1051, 1050, 1049, 1048, 1047, 1046, 1045, 1044},
 {504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539,  1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115},
 {575, 574, 573, 572, 571, 570, 569, 568, 567, 566, 565, 564, 563, 562, 561, 560, 559, 558, 557, 556, 555, 554, 553, 552, 551, 550, 549, 548, 547, 546, 545, 544, 543, 542, 541, 540,  1151, 1150, 1149, 1148, 1147, 1146, 1145, 1144, 1143, 1142, 1141, 1140, 1139, 1138, 1137, 1136, 1135, 1134, 1133, 1132, 1131, 1130, 1129, 1128, 1127, 1126, 1125, 1124, 1123, 1122, 1121, 1120, 1119, 1118, 1117, 1116}};
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


//This determines the panel's position in the logical [row, col] array.
#ifdef LARGE_ARRAY
const uint16_t panel_off[NUM_PANEL][2] =
{{0,         0},          //panel 0:  y=0, x=0
 {PANEL_ROW, 0},          //panel 1:  y=8, x=0
 {0,         PANEL_COL},  //panel 2:  y=0, x=36
 {PANEL_ROW, PANEL_COL}}; //panel 3:  y=8, x=36
#else
const uint16_t panel_off[NUM_PANEL][2] =
{{0, 0},                  //panel 0:  y=0, x=0
 {0, PANEL_COL},          //panel 1:  y=0, x=12
 {0, PANEL_COL*2},        //panel 2:  y=0, x=24
 {0, PANEL_COL*3}};       //panel 3:  y=0, x=36
#endif


#pragma DATA_SECTION(rainbow, "ramConsts")
#define MAX_RGB_VAL     0xf0

//The function get_rgb() can be used to break these colors into component values.
const uint32_t rainbow[12] = {0xf00000,  //red
                              0xd02200,  //orange
                              0xf0f000,  //yellow
                              0x60e000,  //lime
                              0x00f000,  //green
                              0x00e033,  //l green
                              0x00e090,  //cyan
                              0x0070b0,  //marine
                              0x0000f0,  //blue
                              0x7700e0,  //l blue
                              0x680088,  //purple
                              0x800072}; //fushia


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

    Channel 1:      0.. 85Hz    pitch: c1..e2  16  bin: 0         color: Red
    Channel 2:     86..171Hz    pitch: f2..e3  12  bin: 1         color: Orange
    Channel 3:    172..429Hz    pitch: f3..g#4 16  bin: 2..4      color: Purple
    Channel 4:    430..1117Hz   pitch: a4..c#6 17  bin: 5..12     color: Blue
    Channel 5:   1118..1633Hz   pitch: d6..g#6  7  bin: 13..18    color: Aqua
    Channel 6:   1634..2321Hz   pitch: a6..c#7  5  bin: 19..26    color: Cyan
    Channel 7:   2322..3009Hz   pitch: d7..f#7  5  bin: 27..34    color: L Green
    Channel 8:   3010..6019Hz   pitch: g7..f#8 12  bin: 35..69    color: Green
    Channel 9:   6020..9029Hz   pitch: g8..c#9  7  bin: 70..104   color: Lime
    Channel 10:  9030..16000Hz  pitch: d9..c10 11  bin: 105..185  color: Yellow
*/
const uint16_t chan_bin[COLOR_CHANNELS+1] = {0, 1, 2, 5, 13, 19, 27, 35, 70, 105, 185};

//This array maps into the rainbow[] array to assign colors to each color organ channel.
const uint16_t chan_clr[COLOR_CHANNELS] = {0, 1, 11, 8, 7, 6, 5, 4, 3, 2};


void display_init(void)
{

  memset(led, 0, sizeof(led));
  disp_frames = 9999;
}


//This function blocks for the specified number of frame syncs to
//expire.
void wait_for_sync(uint32_t num_syncs)
{
  uint16_t idx;

  pause = 0;

  for (idx = 0; idx < num_syncs; idx ++)
  {
    while (frame_sync == 0)
    {  }

    led_driver();

    pause = 1;
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


#pragma CODE_SECTION(clear_display, "ramCode")

void clear_display(void)
{
  memset(led, 0, sizeof(led));
  wait_for_sync(1);
  pause = 0;
}


#pragma CODE_SECTION(do_gap_display, "ramCode")

//This function displays things while the audio is silent.  It must be allowed to save its
//state and exit each frame so that the gap detect can run....
void do_gap_display(void)
{

  if (gap_frames % (FRAMES_PER_SEC * 12) == 0)
  {
    work_buff[0] = rnd(99) % 4;

    switch (work_buff[0])
    {
      case 0:
        show_text(1, 0, 0xffffffff, 60, "Hello?");
        break;
      case 1:
        show_text(1, 0, 0xffffffff, 60, "Let's Boogie!");
        break;
      case 2:
        show_text(1, 0, 0xffffffff, 60, "Get Down!");
        break;
      case 3:
        show_text(1, 0, 0xffffffff, 60, "Crank It Up!");
        break;
    }

    clear_display();
  }
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

//    //All White:
//    checkerboard(BAL_WHITE, 0);
//    wait_for_sync(15);
//    checkerboard(0, BAL_WHITE);
//    wait_for_sync(15);
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


#pragma CODE_SECTION(line_segments, "ramCode")

#define MIN_LINE_FRAMES                (FRAMES_PER_SEC * 2)

#ifdef LARGE_ARRAY
#define LINE_SEG_LEN                   rnd(4) + 3
#else
#define LINE_SEG_LEN                   rnd(4) + 2
#endif

//This function creates a display where short horizontal line segments are
//randomly assigned to color channels, without regard for balancing the number
//of LEDs per channel (it should balance over time).
void line_segments(uint16_t peak_flag, uint16_t *chan_val)
{
  uint32_t  color;
  uint16_t  idx, first, next;
  uint16_t  len;
  uint16_t  r, g, b;
  uint16_t  last, cv;
  uint16_t  rw, cl;
//  uint16_t  chan_cnt[COLOR_CHANNELS];
  uint16_t *chan;

  if (peak_flag & (disp_frames > MIN_LINE_FRAMES))
  {
    disp_frames = 0;
    last = COLOR_CHANNELS + 1;

    //Create a new random arrangement of lines.
    chan = (uint16_t *)&work_buff;
    for (rw = 0; rw < MAX_ROW; rw ++)
    {
      for (cl = 0; cl < MAX_COL; cl ++)
      {
        if ((cl == 0) || (cl == next))
        {
          len = LINE_SEG_LEN; //random line length
          if ((len + cl) > MAX_COL)
            len = MAX_COL - cl;
          next = cl + len;

          while (cv == last)
            cv = rnd(COLOR_CHANNELS);

          last = cv;
        }

        *chan++ = cv;
      }
    }
  }

  disp_frames ++;
  chan = (uint16_t *)&work_buff;

  for (rw = 0; rw < MAX_ROW; rw ++)
  {
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      //Get the base color for this line
      first = *chan++;
      color = rainbow[chan_clr[first]];
      get_rgb(color, &r, &g, &b);

      //Scale the brightness to the current channel intensity
      cv = chan_val[first];
      r = (r * cv) >> 8;
      g = (g * cv) >> 8;
      b = (b * cv) >> 8;

      idx = col[rw][cl];
      led[idx].r = r;
      led[idx].g = g;
      led[idx].b = b;
    }
  }
}


#pragma CODE_SECTION(two_by_two, "ramCode")

#define MIN_TBT_FRAMES                (FRAMES_PER_SEC * 5)

//This function creates a display where each 2x2 pixel subarray is assigned a
//random color organ channel.  When the peak flag is set (after a minumum number
//of frames), it will shuffle the 2x2s. For the large array, 3x3s are created.
void two_by_two(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint32_t color;
  uint16_t idx, idx2, idx3;
  uint16_t r, g, b, cv;
  uint16_t rw, cl, mode;
  int16_t  blk_cnt;
  int16_t  shift;
  int16_t  prev_shift;
  uint16_t temp[TBT_ROW];

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_TBT_FRAMES))
  {
    disp_frames = 0;

    idx = rnd(100);
    if (idx < 22)
      mode = 1; //fixed arrow
    else if (idx < 44)
    {
      mode = 2; //fixed fade
      cv   = 0;
      blk_cnt = 0;
    }
    else if (idx < 65)
      mode = 3; //random blocks
    else
      mode = 4; //random wave

    if (mode > 2) //not a fixed pattern
    {
      //Create a new random arrangement of 2x2s or 3x3s.
      //First, create a linear arrangement to guarantee equal channel distribution
      idx = rnd(100);
      if (idx < 50)
        cv = 0; //channel numbers increasing with row number
      else
        cv = COLOR_CHANNELS - 2; //channel numbers decreasing with row number

      for (cl = 0; cl < TBT_COL; cl += 2)
      {
        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          tbt_map[rw][cl]   = cv;
          tbt_map[rw][cl+1] = cv+1;

          if (idx < 50)
          {
            cv += 2;
            if (cv >= COLOR_CHANNELS)
              cv = 0;
          }
          else
          {
            cv -= 2;
            if (cv > COLOR_CHANNELS)
              cv = COLOR_CHANNELS - 2;
          }
        }
      }
    }

    prev_shift = rnd(TBT_ROW); //get an initial shift
    shift = prev_shift;

    for (cl = 0; cl < TBT_COL; cl ++)
    {
      switch (mode)
      {
        case 1: //fixed arrow
          //create a symmetric, fixed row-based "arrow" pattern
          if (cl < (TBT_COL/2))
          {
            for (rw = 0; rw < 2; rw ++) //rows 0 and 4, 1 and 3
            {
              idx3 = (COLOR_CHANNELS + cl - rw) % COLOR_CHANNELS;
              tbt_map[rw][cl] = idx3;
              tbt_map[rw][TBT_COL - 1 - cl] = idx3;
              tbt_map[4-rw][cl] = idx3;
              tbt_map[4-rw][TBT_COL - 1 - cl] = idx3;
            }

            idx3 = (COLOR_CHANNELS + cl - 2) % COLOR_CHANNELS; //row 2
            tbt_map[2][cl] = idx3;
            tbt_map[2][TBT_COL - 1 - cl] = idx3;
          }
          break;

        case 2: //fixed fade
          if (cl >= (TBT_COL/2))
          {
            //Set row zero, because it is always inside the array
            tbt_map[0][cl] = cv;
            tbt_map[0][TBT_COL/2 - 1 - (cl-(TBT_COL/2))] = cv;

            blk_cnt ++;
            if (blk_cnt == TBT_CHAN_BLOCKS)
            {
              cv ++;
              blk_cnt = 0;
            }

            idx3 = cl - 1;
            rw   = 1;

            //move up and left 1 block
            while ((rw < TBT_ROW) && (idx3 >= TBT_COL/2)) //stay inside the array
            {
              tbt_map[rw][idx3] = cv;
              tbt_map[rw][TBT_COL/2 - 1 - (idx3-(TBT_COL/2))] = cv;

              blk_cnt ++;
              if (blk_cnt == TBT_CHAN_BLOCKS)
              {
                cv ++;
                blk_cnt = 0;
              }

              rw   ++;
              idx3 --;
            }

            if (cl == (TBT_COL-1)) //special case for the outside corners
            {
              for (rw = 1; rw < TBT_ROW; rw ++)
              {
                idx2 = rw;
                idx3 = cl;

                while ((idx2 < TBT_ROW) && (idx3 >= TBT_COL/2)) //stay inside the array
                {
                  tbt_map[idx2][idx3] = cv;
                  tbt_map[idx2][TBT_COL/2 - 1 - (idx3-(TBT_COL/2))] = cv;

                  blk_cnt ++;
                  if (blk_cnt == TBT_CHAN_BLOCKS)
                  {
                    cv ++;
                    blk_cnt = 0;
                  }

                  idx2 ++;
                  idx3 --;
                }
              }
            }
          }
          break;

        case 3: //random blocks
          //do random rotate by column
          while (shift == prev_shift)
            shift = rnd(TBT_ROW);

          prev_shift = shift;
          break;

        case 4: //random wave
          //move -1, 0 or +1 rows from the previous row to make a "wave" shape
          if ((cl & 1) == 0)
          {
            if (prev_shift == 2) // in the middle; pick up or down randomly
            {
              if (rnd(100) < 50)
                shift = 1;
              else
                shift = 3;
            }
            else //move towards the other end
            {
              if (shift < 2)
                shift ++;
              else
                shift --;
            }
          }

          prev_shift = shift;
          break;
      }

      if (mode > 2)
      {
        for (rw = 0; rw < TBT_ROW; rw ++) //save the column
          temp[rw] = tbt_map[rw][cl];

        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          idx3 = rw + shift;
          if (idx3 >= TBT_ROW) idx3 -= TBT_ROW;

          tbt_map[idx3][cl] = temp[rw];
        }
      }
    }

    //L/R panel invert the fixed patterns
    if ((mode < 3) && (rnd(100) > 50))
    {
      for (cl = 0; cl < TBT_COL/2; cl ++)
      {
        for (rw = 0; rw < TBT_ROW; rw ++)
        {
          idx3 = tbt_map[rw][cl];
          tbt_map[rw][cl] = tbt_map[rw][cl+TBT_COL/2];
          tbt_map[rw][cl+TBT_COL/2] = idx3;
        }
      }
    }
  }

  disp_frames ++;

  #ifdef LARGE_ARRAY
  for (rw = 1; rw < MAX_ROW; rw += TBT_STEP)
  #else
  for (rw = 0; rw < MAX_ROW; rw += TBT_STEP)
  #endif
  {
    for (cl = 0; cl < MAX_COL; cl += TBT_STEP)
    {
      idx = tbt_map[rw/TBT_STEP][cl/TBT_STEP];

      //Get the base color for this 2x2 or 3x3
      color = rainbow[chan_clr[idx]];
      get_rgb(color, &r, &g, &b);

      //Scale the brightness to the current channel intensity
      if (cl < panel_off[2][1])
        cv = left_val[idx];
      else
        cv = right_val[idx];

      r = (r * cv) >> 8;
      g = (g * cv) >> 8;
      b = (b * cv) >> 8;

      for (idx2 = 0; idx2 < TBT_STEP; idx2 ++)
      {
        for (idx = 0; idx < TBT_STEP; idx ++)
        {
          idx3 = col[rw+idx][cl+idx2];
          led[idx3].r = r;
          led[idx3].g = g;
          led[idx3].b = b;
        }
      }

      #ifdef LARGE_ARRAY
      if (rw == 1)
      {
        for (idx2 = 0; idx2 < TBT_STEP; idx2 ++)
        {
          idx3 = col[0][cl+idx2];
          led[idx3].r = r >> 2;
          led[idx3].g = g >> 2;
          led[idx3].b = b >> 2;
        }
      }
      #endif
    }
  }
}


//This function is similar to two_by_two, except that the 2x2 blocks for each
//channel are joined to create tetris shapes. It reuses the two_by_two globals.
//The "blocks" parameter is the number of 2x2 blocks to use per channel. To assure
//an equal number of LEDs per channel, this should divide evening into the number
//of LEDs per panel.

uint16_t first_clr = 0;

void tetris(uint16_t side, uint16_t blocks, uint16_t peak_flag, uint16_t *chan_val)
{
  uint32_t color;
  uint16_t idx, idx2, idx3;
  uint16_t r, g, b, cv;
  uint16_t rw, cl; //, temp;
  uint16_t tr, tc, nb, lp;
  uint16_t side_offset;
//  uint16_t chan[COLOR_CHANNELS];

  if (side == LEFT)
    side_offset = 0;
  else
    side_offset = PANEL_COL;

  if (peak_flag & (disp_frames > MIN_TBT_FRAMES))
  {
    disp_frames = 0;

    //Create a new random arrangement of pieces (for both LEFT and RIGHT sides).
    if (side == LEFT)
    {
#if 0
      //First, create a sorted list of channel numbers.
      for (idx = 0; idx < COLOR_CHANNELS; idx ++)
        chan[idx] = idx;

      //Now randomly swap pairs of channel indexes
      for (idx = 0; idx < COLOR_CHANNELS; idx ++)
      {
        idx3 = rnd(COLOR_CHANNELS);
        temp = chan[idx];
        chan[idx]  = chan[idx3];
        chan[idx3] = temp;
      }
#endif
      //Initialize the table to -1
      memset(&tbt_map[0][0], 0xff, sizeof(tbt_map));

      //Now, create the tetris pieces
      rw = 0;  cl = 0;  cv = first_clr;
      nb = 0;  lp = 0;
      first_clr ++;
      if (first_clr == COLOR_CHANNELS)
        first_clr = 0;

      while (lp < (blocks * COLOR_CHANNELS * NUM_PANEL))
      {
        //First, advance to the next free location (scan column-first)
        while (tbt_map[rw][cl] != 255)
        {
          rw ++;
          if (rw == TBT_ROW)
          {
            rw = 0;
            cl ++;
          }
        }

        //Mark the first block with the channel
        tr = rw; tc = cl; //use temp r/c vars to keep rw and cl on the start point.
        tbt_map[tr][tc] = cv;
        nb ++;

        while (nb < blocks)
        {
          if (rnd(100) & 1) //1 = +1 row, 0 = +1 col
          {
            if (((tr+1) < TBT_ROW) && (tbt_map[tr+1][tc] == 255))
            {
              tbt_map[++tr][tc] = cv;
              nb ++;
            }
            else
            if (((tc+1) < TBT_COL) && (tbt_map[tr][tc+1] == 255))
            {
              tbt_map[tr][++tc] = cv;
              nb ++;
            }
            else //the block won't fit here
              break;
          }
          else
          {
            if (((tc+1) < TBT_COL) && (tbt_map[tr][tc+1] == 255))
            {
              tbt_map[tr][++tc] = cv;
              nb ++;
            }
            else
            if (((tr+1) < TBT_ROW) && (tbt_map[tr+1][tc] == 255))
            {
              tbt_map[++tr][tc] = cv;
              nb ++;
            }
            else //the block won't fit here
              break;
          }
        }

        if (nb == blocks)
        {
          nb = 0;
          lp += blocks;

          cv ++;
          if (cv == COLOR_CHANNELS)
          {
            cv = 0;
          }
        }
      }
    }
  }

  disp_frames ++;

  for (idx = 0; idx < TBT_ROW; idx ++)
  {
    rw = idx * 2;

    for (idx2 = 0; idx2 < PANEL_COL; idx2 ++)
    {
      cl   = (idx2*2)+side_offset*2;
      idx3 = tbt_map[idx][idx2+side_offset];

      //Get the base color for this 2x2
      color = rainbow[chan_clr[idx3]];
      get_rgb(color, &r, &g, &b);

      //Scale the brightness to the current channel intensity
      cv = chan_val[idx3];
      r = (r * cv) >> 8;
      g = (g * cv) >> 8;
      b = (b * cv) >> 8;

      idx3 = col[rw][cl];
      led[idx3].r = r;
      led[idx3].g = g;
      led[idx3].b = b;

      idx3 = col[rw][cl+1];
      led[idx3].r = r;
      led[idx3].g = g;
      led[idx3].b = b;

      idx3 = col[rw+1][cl];
      led[idx3].r = r;
      led[idx3].g = g;
      led[idx3].b = b;

      idx3 = col[rw+1][cl+1];
      led[idx3].r = r;
      led[idx3].g = g;
      led[idx3].b = b;
    }
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
}


#define HIGH_BIAS     0x28
#define HIGH_PASS     0x10
#pragma CODE_SECTION(color_organ_prep, "ramCode")

//This function prepares the FFT output bins for the display routines.
void color_organ_prep(float *fft_bin, uint16_t *chan_val)
{
  uint16_t idx, idx2, idx3, end;
  float    temp, max;
  float    maxlo, maxmid, maxhi;
  #ifdef OLD_STYLE_BULBS
  uint16_t prev_cv[COLOR_CHANNELS];
  #endif

  idx3  = 1;
  maxhi = 0;
  maxmid= 0;
  maxlo = 0;

  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    #ifdef OLD_STYLE_BULBS
    //Save off the last frame's value
    prev_cv[idx] = chan_val[idx];
    #endif

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

    //Don't pass very small values through for bass channels.
    if ((idx <= 1) && (chan_val[idx] < HIGH_PASS))
      chan_val[idx] = 0;
  }

  //Since the highest channel registers lower with the same signal strength
  //give it a boost if it's not zero.
  if (chan_val[COLOR_CHANNELS-1] > 0)
    chan_val[COLOR_CHANNELS-1] += HIGH_BIAS;

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

  #ifdef OLD_STYLE_BULBS
  //Incandescent bulbs cannot flash at 15 or 30Hz. Apply a fading factor to
  //slow the LEDs down a bit. We don't want to put people into a trance... ;^)
  for (idx = 0; idx < COLOR_CHANNELS; idx++)
  {
    //If the new value is less than half, set it to half instead.
    if (chan_val[idx] < (prev_cv[idx] >> 1))
      chan_val[idx] = prev_cv[idx] >> 1;
  }
  #endif
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

    //remove extra zero bits (if any) at the bottom of each column
    bits >>= FONT_YOFF;

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

// panel   = 0 .. 3
//dispmode = 0 = centered in one panel
//         = 1 = centered in total array
//         = 2 = scroll from right
void show_text(int16_t dispmode, int16_t panel, uint32_t color, int16_t delay, char *str)
{
  int16_t  x, y;
  uint16_t idx, idx2, idx3;
  uint16_t s_len, p_len;
  uint16_t word_idx;
  uint16_t curr_len;
  uint16_t curr_char;
  uint32_t disp_clr;
  uint16_t str_idx[FONT_MAX_CHAR];
  uint16_t char_wid[FONT_MAX_CHAR];
  uint16_t word_clr[5];

  disp_clr = color;

  s_len = font_str_len(str);          //get string length in ASCII characters
  if (s_len > FONT_MAX_CHAR)
  {
    s_len = FONT_MAX_CHAR;
    str[s_len] = 0; //insert a NULL to terminate
  }

  p_len = font_pix_len(str, str_idx); //get string length in pixels
  if (p_len > MAX_COL)
    dispmode = 2;

  if (dispmode < 2)
  {
    if (p_len > PANEL_COL) //won't fit in one panel
      dispmode = 1; //force total array mode

    if (dispmode == 0) //center in one panel
    {
      x = panel_off[panel][1] + (PANEL_COL/2) - (p_len/2);
      y = panel_off[panel][0] + (PANEL_ROW/2) - (FONT_HEIGHT/2);
    }
    else if (dispmode == 1) //center in total array
    {
      x = (MAX_COL / 2) - (p_len / 2);
      y = (MAX_ROW / 2) - (FONT_HEIGHT / 2);
    }

    if (x < 0) x = 0;
    if (color == 0xffffffff)
      disp_clr = rainbow[rnd(12)];

    for (idx = 0; idx < s_len; idx++)
    {
      if ((color == 0xffffffff) && (str_idx[idx] == FONT_SPACE_IDX))
        disp_clr = rainbow[rnd(12)];

      draw_char(x, y, str_idx[idx], disp_clr);
      x += fontchar[str_idx[idx]].wid + 1;
    }

    for (idx3 = 0; idx3 < delay; idx3 ++)
    {
      wait_for_sync(1);
      pause = 0;

      if (end_of_gap)
        return;
    }
  }
  else //scroll from right, blanking what was displayed in the prior loop
  {
    curr_len  = 0;
    curr_char = 0xffff;
    y = (MAX_ROW / 2) - (FONT_HEIGHT / 2);

    for (idx = 0; idx < 5; idx++)
      word_clr[idx] = rnd(12);

    for (idx = 0; idx < p_len; idx++)
    {
      if (curr_len < (idx + 1))
      {
        curr_char ++;
        curr_len += fontchar[str_idx[curr_char]].wid + 1;
        char_wid[curr_char] = curr_len;
      }

      for (idx2 = 0; idx2 <= curr_char; idx2++)
      {
        word_idx = 0;
        idx3 = MAX_COL - idx + char_wid[idx2];
        draw_char(idx3, y, str_idx[idx2], 0x000000); //black out the previous write

        if ((color == 0xffffffff) && (str_idx[idx] == FONT_SPACE_IDX))
          disp_clr = rainbow[word_clr[word_idx++]];
        draw_char(idx3-1, y, str_idx[idx2], disp_clr);
      }

      wait_for_sync(1);
      pause = 0;

      if (end_of_gap)
        return;
    }

    wait_for_sync(delay);
    pause = 0;
  }
}


//This function performs power on (or reset) displays to verify that the LED
//panels are correctly operating prior to the color organ running.
void initial_display(void)
{
  char str[2];

  memset(led, 0, sizeof(led));

  //Display the panel number in each panel to identify correct hookup.
  str[0] = '1'; str[1] = 0;
  show_text(0, 0, 0x0000a0, 3, str);
  str[0] = '2';
  show_text(0, 1, 0x0000a0, 3, str);
  str[0] = '3';
  show_text(0, 2, 0x0000a0, 3, str);
  str[0] = '4';
  show_text(0, 3, 0x0000a0, 60, str);
  clear_display();

  show_text(1, 0, 0x00f000, 60, "Let's Go!");

  //Set the end of gap flag so that the first frame will be a peak.
  end_of_gap = 1;
}
