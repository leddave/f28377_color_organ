/*****************************************************************************
 * This is the f28377_color_organ project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "display.h"
#include "rfft.h"
#include "utils.h"
#include "floods.h"
#include "font.h"
#include "led_driver.h"
#include "msg.h"


//Define the overall array of LED panels. The current design is for each panel
//to be a separate string of LEDs, driven by a separate Launchpad GPIO. For this
//reason, it is important that these definitions match with the global variables
//col[][] and panel_off[][].
#define NUM_PANEL    NUM_STRINGS

#define PANEL_ROW    8
#define PANEL_COL    36
#define TBT_STEP     3
#define MAX_ROW     (PANEL_ROW * 2)
#define MAX_COL     (PANEL_COL * 2)

#define TBT_ROW          (MAX_ROW / TBT_STEP)
#define TBT_COL          (MAX_COL / TBT_STEP)

#define TBT_CHAN_BLOCKS  (TBT_ROW * TBT_COL / COLOR_CHANNELS / 2)

#ifdef FLOODS
#define HYST_SIZE    3
#endif

//Global variables
#pragma DATA_SECTION(led_ping, "leddata")
//#pragma DATA_SECTION(led_pong, "leddata")
LED_MAIN  led_ping[MAX_LEDS];  //array of color information used by the LED driver
//LED_MAIN  led_pong[MAX_LEDS];

uint32_t  work_buff[MAX_LEDS]; //temporary work buffer (can be used by all displays)
uint32_t  disp_frames;
uint32_t  display_frames;      //used only in the display driver
uint16_t  color_channels;
uint16_t  last_chan_cnt;
uint16_t  display;
uint16_t  gap_display;
uint16_t  gap_color;
uint16_t  gap_timer;
uint16_t  bass_chan;
uint16_t  midr_chan;
uint16_t  led_response;
uint16_t  flood_response;
uint16_t  avail_disp;
float     audio_gain;

LED_MAIN  color_tab_left[MAX_CHANNELS];
LED_MAIN  color_tab_right[MAX_CHANNELS];
uint32_t  channel_color[MAX_CHANNELS]; //current color for each channel
uint16_t  channel_freq[MAX_CHANNELS+1]; //current freq bin for each channel

//Temporary storage for the two-by-two function
uint16_t tbt_map[TBT_ROW][TBT_COL/2];

extern volatile uint16_t frame_sync;
extern volatile uint16_t end_of_gap;
extern volatile uint32_t gap_frames;
extern volatile uint32_t frame_cnt;

//These globals are defined in the CPU -> CLA message RAM
extern uint16_t  ppong_fill;
extern uint16_t  ppong_use;

//Needed for the high res equalizer display
extern float RFFTmagBuff1[RFFT_SIZE/2+1];
extern float RFFTmagBuff2[RFFT_SIZE/2+1];

extern Msg_Default_Values defaults;
extern Msg_Available_Capabilities capabilities;


// The following tables are built to convert a 2D y:x position into an index
// in the 1D arrays of LEDs.

#pragma DATA_SECTION(col, "ramConsts")

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


//This determines the panel's position in the logical [row, col] array.
const uint16_t panel_off[NUM_PANEL][2] =
{{0,         0},          //panel 0:  y=0, x=0
 {PANEL_ROW, 0},          //panel 1:  y=8, x=0
 {0,         PANEL_COL},  //panel 2:  y=0, x=36
 {PANEL_ROW, PANEL_COL}}; //panel 3:  y=8, x=36


#pragma DATA_SECTION(rainbow, "ramConsts")

//The function get_rgb() can be used to break these colors into component values.
const uint32_t rainbow[12] = {0xf00000,  //red
                              0xc82800,  //orange
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

#pragma DATA_SECTION(chan_bin, "ramConsts")
#pragma DATA_SECTION(chan_clr, "ramConsts")

//This array defines the range of FFT bins for each color organ channel.  At 86Hz
//per bin, the starting frequency of each bin is x*86.
const uint16_t chan_bin[MAX_CHANNELS+1][MAX_CHANNELS+2] = {
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

/*  Three channel color organ frequency ranges:
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)
    (Middle C = c4, 262Hz)

    Channel 1:     0..172Hz    a0-f3  bin: 0..1     color: Red
    Channel 2:   172..1032Hz   f3-c6  bin: 2..11    color: Blue
    Channel 3:  1032..15000Hz  c6-c8+ bin: 12..175  color: Green
*/
 {0, 2, 12, 175, 0, 0, 0, 0, 0, 0, 0, 0}, //3

/*  Four channel color organ frequency ranges. Same as 3 channel plus
    high frequency channel.

    Channel 1:     0..172Hz    a0-f3  bin: 0..1     color: Red
    Channel 2:   172..1032Hz   f3-c6  bin: 2..11    color: Blue
    Channel 3:  1032..4214Hz   c6-c8 bin: 12..48    color: Green
    Channel 4:  4214..15000Hz  c8+   bin: 49..175   color: Yellow
*/
 {0, 2, 12, 49, 175, 0, 0, 0, 0, 0, 0, 0}, //4

/*  Five channel, ~two octaves per color organ channel
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:      0..172Hz   a0-f3  bin: 0..1      color: Red
    Channel 2:    172..430Hz   f3-a5  bin: 2..4      color: Purple
    Channel 3:    430..1032Hz  a5-c6  bin: 5..11     color: Blue
    Channel 4:   1032..4214Hz  c6-c8  bin: 12..48    color: Green
    Channel 5:   4214..15000Hz c8+    bin: 49..175   color: Yellow
*/
 {0, 2, 5, 12, 49, 175, 0, 0, 0, 0, 0, 0}, //5

/*  Six channel, color organ channel
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:      0..172Hz   a0-f3  bin: 0..1      color: Red
    Channel 2:    172..344Hz   f3-f4  bin: 2..3      color: Purple
    Channel 3:    344..688Hz   f4-f5  bin: 4..7      color: Blue
    Channel 4:    688..1720Hz  f5-a7  bin: 8..19     color: L green
    Channel 5:   1720..4214Hz  a7-c8  bin: 20..48    color: Green
    Channel 6:   4214..15000Hz c8+    bin: 49..175   color: Yellow
*/
 {0, 2, 4, 8, 20, 49, 175, 0, 0, 0, 0, 0},

/*  Seven channel, ~one octave per color organ channel
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:      0..172Hz   a0-f3  bin: 0..1      color: Red
    Channel 2:    172..344Hz   f3-f4  bin: 2..3      color: Purple
    Channel 3:    344..688Hz   f4-f5  bin: 4..7      color: Blue
    Channel 4:    688..1290Hz  f5-e6  bin: 8..14     color: L green
    Channel 5:   1290..2322Hz  e6-d7  bin: 15..26    color: Green
    Channel 6:   2322..4214Hz  d7-c8  bin: 27..48    color: Lime
    Channel 7:   4214..15000Hz c8+    bin: 49..175   color: Yellow
*/
 {0, 2, 4, 8, 15, 27, 49, 175, 0, 0, 0, 0}, //7

/*  Eight channel color organ channel
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:     0.. 86Hz    a0-f2   bin: 0        color: Red
    Channel 2:    86..172Hz    f2-f3   bin: 1        color: Fushia
    Channel 3:   172..344Hz    f3-f4   bin: 2..3     color: Purple
    Channel 4:   344..688Hz    f4-f5   bin: 4..7     color: Blue
    Channel 5:   688..1118Hz   f5-d6   bin: 8..12    color: Aqua
    Channel 6:  1118..1978Hz   d6-b7   bin: 13..22   color: Cyan
    Channel 7:  1978..2838Hz   b7-f7   bin: 23..32   color: Green
    Channel 8:  4214..15000Hz  c8+     bin: 49..175  color: Yellow
*/
 {0, 1, 2, 4, 8, 13, 23, 49, 175, 0, 0, 0},

/*  Nine channel color organ channel
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:     0.. 86Hz    a0-f2   bin: 0        color: Red
    Channel 2:    86..172Hz    f2-f3   bin: 1        color: Fushia
    Channel 3:   172..344Hz    f3-f4   bin: 2..3     color: Purple
    Channel 4:   344..688Hz    f4-f5   bin: 4..7     color: Blue
    Channel 5:   688..1118Hz   f5-d6   bin: 8..12    color: Aqua
    Channel 6:  1118..1978Hz   d6-b7   bin: 13..22   color: Cyan
    Channel 7:  1978..2838Hz   b7-f7   bin: 23..32   color: Green
    Channel 8:  2838..4214Hz   f7-c8   bin: 33..48   color: Lime
    Channel 9:  4214..15000Hz  c8+     bin: 49..175  color: Yellow
*/
 {0, 1, 2, 4, 8, 13, 23, 33, 49, 175, 0, 0}, //9

/*  Ten channel color organ
    Frequency ranges: (44.1Khz sample rate, 86Hz per bin)

    Channel 1:     0.. 86Hz    a0-f2   bin: 0        color: Red
    Channel 2:    86..172Hz    f2-f3   bin: 1        color: Orange
    Channel 3:   172..344Hz    f3-f4   bin: 2..3     color: Purple
    Channel 4:   344..602Hz    f4-d5   bin: 4..6     color: Blue
    Channel 5:   602..860Hz    d5-a6   bin: 7..9     color: Aqua
    Channel 6:   860..1290Hz   a6-e6   bin: 10..14   color: Cyan
    Channel 7:  1290..1978Hz   e6-b7   bin: 15..22   color: L Green
    Channel 8:  1978..3612Hz   b7-g7   bin: 23..35   color: Green
    Channel 9:  3612..4214Hz   g7-c8   bin: 36..48   color: Lime
    Channel 10: 4214..15000Hz  c8+     bin: 49..175  color: Yellow
*/
 {0, 1, 2, 4, 7, 10, 15, 23, 36, 49, 175, 0}};


//This array maps into the rainbow[] array to assign colors to each color organ channel.
const uint16_t chan_clr[MAX_CHANNELS+1][MAX_CHANNELS] =
{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 8, 4, 0, 0, 0, 0, 0, 0, 0}, //3
 {0, 8, 4, 2, 0, 0, 0, 0, 0, 0},
 {0, 10, 8, 4, 2, 0, 0, 0, 0, 0}, //5
 {0, 10, 8, 5, 4, 2, 0, 0, 0, 0},
 {0, 10, 8, 5, 4, 3, 2, 0, 0, 0}, //7
 {0, 11, 10, 8, 7, 6, 4, 2, 0, 0},
 {0, 11, 10, 8, 7, 6, 4, 3, 2, 0}, //9
 {0, 1, 11, 8, 7, 6, 5, 4, 3, 2}};


void display_init(void)
{
  uint16_t idx;

  memset(led_ping, 0, sizeof(led_ping));
//  memset(led_pong, 0, sizeof(led_pong));

  color_channels = 7;
  last_chan_cnt = 7;
  led_response = 2;
  flood_response = 2;
  avail_disp = (uint16_t)(defaults.disp_mask_hi) << 8 | defaults.disp_mask_lo;
  audio_gain = 1.0;

  gap_frames = 9999;
  disp_frames = 99999;
  display     = 9999;
  display_frames = 99999;
  gap_display = 9999;
  gap_color   = 9999;
  gap_timer   = 9999;

  //find the approximate color channel boundaries for Bass and Midrange
  for (idx = 0; idx < MAX_CHANNELS; idx++)
  {
    if (chan_bin[color_channels][idx] < 3)
         bass_chan = idx;
    if (chan_bin[color_channels][idx] < 30)
         midr_chan = idx;

    //Load default colors and freq range
    channel_color[idx] = rainbow[chan_clr[color_channels][idx]];
    channel_freq[idx]  = chan_bin[color_channels][idx];
  }

  channel_freq[MAX_CHANNELS] = chan_bin[color_channels][MAX_CHANNELS];

#ifdef FLOODS
  init_floods();
#endif
}


#pragma CODE_SECTION(reload_channel_data, "ramCode")

void reload_channel_data(uint16_t chan)
{
  uint16_t idx;

  for (idx = 0; idx < MAX_CHANNELS; idx++)
  {
    channel_color[idx] = rainbow[chan_clr[chan][idx]];

    channel_freq[idx]  = chan_bin[chan][idx];
    channel_freq[MAX_CHANNELS] = chan_bin[chan][MAX_CHANNELS];
  }
}


#pragma CODE_SECTION(wait_for_sync, "ramCode")

//This function blocks for the specified number of frame syncs to
//expire.
void wait_for_sync(uint32_t num_syncs)
{
  uint16_t idx;
  uint16_t pause;

  pause = 0;

  for (idx = 0; idx < num_syncs; idx ++)
  {
    while (frame_sync == 0)
    {  }

    frame_sync = 0;

    if (pause == 0)
    {
      #ifdef USE_CLA
        Cla1ForceTask1();

//        #ifdef FLOODS
//        Cla1ForceTask2();
//        #endif
      #else
        led_driver();
      #endif
    }

    pause = 1;
  }
}


#pragma CODE_SECTION(find_next_color, "ramCode")
int16_t lastcolor[MAX_CHANNELS * 2];

//DEBUG CODE DELETE:
//uint16_t hist[100];
//uint16_t hidx = 0;

//This function looks through an array of counts (per channel) looking for a
//count > 0. If found, it returns it; if not, returns -1.
int16_t find_next_color(uint16_t startmask, uint16_t *currmask)
{
  int16_t  idx, cv;
  uint16_t flag, dir;

  if (*currmask == 0) //all colors have been used; reload available colors
    *currmask = startmask;

  cv   = rnd(color_channels);
  flag = 1 << cv;

  if ((flag & *currmask) == 0) //this color is taken; search for another one
  {
    dir = rnd(100);

    for (idx = 0; idx < color_channels; idx ++)
    {
      if (dir < 50) //search downward
      {
        cv --;
        if (cv < 0)
          cv = color_channels-1;
      }
      else //search upward
      {
        cv ++;
        if (cv >= color_channels)
          cv = 0;
      }

      flag = 1 << cv;

      if ((flag & *currmask) != 0)
        break;
    }
  }

  *currmask &= ~flag; //remove this color

//DELETE THIS:::
//if (hidx < 100)
//{
//hist[hidx++] = cv;
//hist[hidx++] = *currmask;
//}

  return(cv); //found one
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


//This function creates a table of channel colors, per side. It can be called
//just once per frame since the colors don't change within a frame.
#pragma CODE_SECTION(build_color_table, "ramCode")

void build_color_table(uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  scale;
  uint16_t  r, g, b;
  uint32_t  color;

  //Create a table of colors for this frame
  for (idx = 0; idx < color_channels; idx ++)
  {
    scale = left_val[idx];
    color = channel_color[idx];
    get_rgb(color, &r, &g, &b);
    color_tab_left[idx].r = (r * scale) >> 8;
    color_tab_left[idx].g = (g * scale) >> 8;
    color_tab_left[idx].b = (b * scale) >> 8;

    scale = right_val[idx];
    get_rgb(color, &r, &g, &b);
    color_tab_right[idx].r = (r * scale) >> 8;
    color_tab_right[idx].g = (g * scale) >> 8;
    color_tab_right[idx].b = (b * scale) >> 8;
  }
}


#pragma CODE_SECTION(clear_display, "ramCode")

void clear_display(void)
{
  memset(led_ping, 0, sizeof(led_ping));
//  memset(led_pong, 0, sizeof(led_pong));
}


#pragma CODE_SECTION(do_gap_display, "ramCode")

//This function displays things while the audio is silent.  It must be allowed to save its
//state and exit each frame so that the gap detect can run....
void do_gap_display(void)
{
  uint16_t idx, last_disp;
  uint16_t last_color;

  #ifndef SLAVE
  if (gap_frames % (FRAMES_PER_SEC * 10) == 0)
  {
    idx = rnd(9999);
    gap_timer   = 0;
    last_disp   = gap_display;
    gap_display = idx % 12;
    last_color  = gap_color;
    gap_color   = rainbow[rnd(12)];

    if (gap_display == last_disp) //make sure the text changes
    {
      gap_display ++;
      if (gap_display == 12)
        gap_display = 0;
    }

    if (gap_color == last_color) //make sure the color changes
    {
      gap_color ++;
      if (gap_color == 12)
        gap_color = 0;
    }
  }

  if (gap_timer < 4)
  {
    switch (gap_display)
    {
      case 0:
        show_text(1, 0, gap_color, 0, "Hello?");
        break;
      case 1:
        show_text(1, 0, gap_color, 0, "Let's Boogie!");
        break;
      case 2:
        show_text(1, 0, gap_color, 0, "Get Down!");
        break;
      case 3:
        show_text(1, 0, gap_color, 0, "Crank It Up!");
        break;
      case 4:
        show_text(1, 0, gap_color, 0, "WOO HOO!");
        break;
      case 5:
        show_text(1, 0, gap_color, 0, "Rock n Roll!");
        break;
      case 6:
        show_text(1, 0, gap_color, 0, "R U There?");
        break;
      case 7:
        show_text(1, 0, gap_color, 0, "Weeeeeeee!");
        break;
      case 8:
        show_text(1, 0, gap_color, 0, "Call Me!?");
        break;
      case 9:
        show_text(1, 0, gap_color, 0, "Say What?");
        break;
      case 10:
        show_text(1, 0, gap_color, 0, "Where r you?");
        break;
      case 11:
        show_text(1, 0, gap_color, 0, "Party Time!");
        break;
    }
  }

  if (gap_timer == 90)
    clear_display();
  gap_timer ++;
  #endif

  #ifdef FLOODS
  // Left-Right color fades
  //wash();
  flood_gap(gap_color, gap_color, gap_timer);

  reset_hyst();
  #endif
}


#if 0
#pragma CODE_SECTION(copy_pixel, "ramCode")
void copy_pixel(LED_MAIN *led, uint16_t dest, uint16_t src)
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
#endif


#pragma CODE_SECTION(line_segments, "ramCode")

#define MIN_LINE_FRAMES                (FRAMES_PER_SEC * 10)
#define LINE_END                       ((MAX_ROW/2/2) * MAX_COL/2)

//This function creates a display where short moving horizontal line segments
//are randomly assigned to color channels. It is symmetric horizontally and
//vertically, with movement from the center outward.
void line_segments(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2;
  uint16_t  next, len;
  uint16_t  cv;
  uint16_t  rw, cl;
  uint16_t *buff;
  int16_t   chan_cnt[MAX_CHANNELS];
  LED_MAIN *cv_clr1;
  LED_MAIN *cv_clr2;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];


  if (peak_flag & (disp_frames > MIN_LINE_FRAMES))
  {
    disp_frames = 0;

    for (idx = 0; idx < color_channels; idx ++)
      chan_cnt[idx] = (LINE_END / color_channels) + 2;

    //Create a new random arrangement of line segments in the work buffer.
    //The arrangement is stored in a "run length encoded" (RLE) fashion.
    buff = (uint16_t *)&work_buff;
    idx  = 0;

    while (idx <= LINE_END)
    {
      idx2 = rnd(99999);
      cv  = idx2 % color_channels;   //random channel
      len = (idx2 % 4) + 3;          //random line length

      if (chan_cnt[cv] <= 0) //this channel has been "used up". Find another one
      {
        for (idx2 = 0; idx2 < color_channels; idx2 ++)
        {
          cv ++;
          if (cv >= color_channels) cv = 0;
          if (chan_cnt[cv] > 0) break;
        }
      }

     *buff++ = cv;   //store the channel in word 0
     *buff++ = len;  //store the segment length in word 1

      chan_cnt[cv] -= len;
      idx += len;
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  idx  = 0;
  cv   = *buff++;
  len  = *buff++;
  next = len;

  for (rw = 0; rw < MAX_ROW/2; rw += 2)
  {
    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      if (idx == next)
      {
        cv  = *buff++;
        len = *buff++;
        next += len;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rw+1][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-1-rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-2-rw][cl];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rw+1][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-1-rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-2-rw][MAX_COL-1-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
    }
  }
}

#define LINE_MIN_SEG                  3
#define LINE_MAX_SEG                  6
#define LINE_ROW_SEG                  ((MAX_COL/2) / LINE_MIN_SEG)

#pragma CODE_SECTION(line_segment2, "ramCode")

//This function creates a display where short moving horizontal line segments
//are randomly assigned to color channels. It is symmetric horizontally and
//vertically, with movement from the center outward.
void line_segment2(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx, rot;
  uint16_t  idx2, idx3;
  uint16_t  next, len;
  int16_t   cv;
  uint16_t  rw, cl;
  uint16_t *buff;
  uint16_t  startmask;
  uint16_t  currmask;
  LED_MAIN *cv_clr1;
  LED_MAIN *cv_clr2;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  //Compute the lower-left quadrant.  Others are mirror image.
  //Create a new random arrangement of line segments in the work buffer.
  buff = (uint16_t *)&work_buff;

//  if (peak_flag & (disp_frames > MIN_LINE_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;
    startmask = 0;

    for (idx = 0; idx < color_channels; idx ++)
      startmask |= 1 << idx;

    currmask = startmask;
//      chan_cnt[idx] = ((MAX_ROW/4*MAX_COL/2) / color_channels) + 1;

    for (rw = 0; rw < MAX_ROW/2; rw += 2)
    {
      idx  = 0;
      idx3 = 2;
      if ((rw % 4) == 0)
        buff[0] = rnd(6) + 10;  //store the speed factor, 10 to 15
      else
        buff[0] = rnd(5) + 3;   //store the speed factor, 3 to 7
      buff[1] = MAX_COL/2-1; //current rotation

      while (idx < MAX_COL/2)
      {
        idx2= rnd(99999);
        len = (idx2 % (LINE_MAX_SEG-LINE_MIN_SEG)) + LINE_MIN_SEG;  //line length
        cv = find_next_color(startmask, &currmask);

        if ((len + idx) > MAX_COL/2)
          len = MAX_COL/2 - idx - 1;

        //don't let len equal 1
        if (len < 2)
          len = 0xff; //abort the end of the column

        buff[idx3++] = cv;   //store the channel in word 0
        buff[idx3++] = len;  //store the segment length in word 1

        idx += len + 1;
      }

      buff[idx3++] = 0xff;   //mark the end
      buff[idx3++] = 0xff;

      buff += (LINE_ROW_SEG + 1) * 2; //position for the next column
    }
  }

  clear_display();
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (rw = 0; rw < MAX_ROW/2; rw += 2)
  {
    idx2 = buff[0]; //get the row's speed
    rot  = buff[1];
    if ((disp_frames % idx2) == 0) //update the rotation and save
    {
      rot --;
      if (rot < 0)
        rot = MAX_COL/2-1;
      buff[1] = rot;
    }

    idx  = 0;
    idx3 = 4;
    cv   = buff[2];
    len  = buff[3];
    next = len;

    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      if (idx == next)
      {
        cv  = buff[idx3++];
        len = buff[idx3++];
        next += len;
        //leave a blank col
        cl ++;
        rot --;
        if (rot < 0)
          rot = MAX_COL/2-1;

        if ((len == 0xff) || (len < 2)) break;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rw+1][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[MAX_ROW-1-rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[MAX_ROW-2-rw][rot];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rw+1][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx2 = col[MAX_ROW-1-rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[MAX_ROW-2-rw][MAX_COL-1-rot];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
      rot --; //update for the next iteration
      if (rot < 0)
        rot = MAX_COL/2-1;
    }

    buff += (LINE_ROW_SEG + 1) * 2; //position for the next column
  }
}


#pragma CODE_SECTION(arrows, "ramCode")

#define MIN_ARROW_FRAMES                (FRAMES_PER_SEC * 10)
#define ARROW_WIDTH                     3
uint16_t display_flip;

//This function creates a display where the channels are drawn as nested
//horizontal arrows.  They are randomly flipped by swapping left-right sides.
void arrows(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx3, wid;
  uint16_t  cv;
  uint16_t  rw, cl;
  uint16_t  start_cv, start_wid;
  uint16_t *row1;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  //Choose a new pattern to display - on a peak boundary
  if (peak_flag & (disp_frames > MIN_ARROW_FRAMES))
  {
    disp_frames = 0;
    display_flip = rnd(10000) & 1;

    start_cv  = color_channels - 1;
    start_wid = ARROW_WIDTH - 1;

    for (rw = 0; rw < MAX_ROW/2; rw ++)
    {
      //Compute the upper-left quadrant
      row1 = (uint16_t *)&work_buff[rw * (MAX_COL/2)];
      cv   = start_cv;
      wid  = start_wid;

      for (cl = 0; cl < MAX_COL/2; cl ++)
      {
        if (display_flip) // Left/Right panel invert the pattern
        {
          row1[(MAX_COL/2 - 1) - cl] = cv;
        }
        else
        {
          row1[cl] = cv;
        }

        wid --;
        if (wid == 0)
        {
          wid = ARROW_WIDTH;
          if (cv > 0)
            cv --;
          else
            cv = color_channels - 1;
        }
      }

      start_wid ++;
      if (start_wid > ARROW_WIDTH)
      {
        start_wid = 1;
        start_cv  ++;

        if (start_cv == color_channels)
          start_cv = 0;
      }
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;

  for (rw = 0; rw < MAX_ROW/2; rw ++)
  {
    idx  = rw + (MAX_ROW/2);
    row1 = (uint16_t *)&work_buff[rw * (MAX_COL/2)];

    for (cl = 0; cl < MAX_COL/2; cl ++)
    {
      cv = row1[cl];

      //Write the left side pixels:
      idx3 = col[idx][cl];
      led[idx3].r = color_tab_left[cv].r;
      led[idx3].g = color_tab_left[cv].g;
      led[idx3].b = color_tab_left[cv].b;
      idx3 = col[(MAX_ROW/2) - (rw+1)][cl];
      led[idx3].r = color_tab_left[cv].r;
      led[idx3].g = color_tab_left[cv].g;
      led[idx3].b = color_tab_left[cv].b;

      //Write the right side pixels:
      idx3 = col[idx][(MAX_COL-1) - cl];
      led[idx3].r = color_tab_right[cv].r;
      led[idx3].g = color_tab_right[cv].g;
      led[idx3].b = color_tab_right[cv].b;
      idx3 = col[(MAX_ROW/2) - (rw+1)][(MAX_COL-1) - cl];
      led[idx3].r = color_tab_right[cv].r;
      led[idx3].g = color_tab_right[cv].g;
      led[idx3].b = color_tab_right[cv].b;
    }
  }
}


#pragma CODE_SECTION(draw_arrow, "ramCode")

//Draws a single arrow.  xpos defines the leftmost x position.
void draw_arrow(LED_MAIN *led, uint16_t chan, uint16_t lr, uint16_t xpos, uint16_t dir)
{
  int16_t  x, rw;
  uint16_t idx, idx2;
  uint16_t r, g, b;
  uint32_t temp;

  if (lr == 0) //left
  {
    r = color_tab_left[chan].r;
    g = color_tab_left[chan].g;
    b = color_tab_left[chan].b;
  }
  else
  {
    r = color_tab_right[chan].r;
    g = color_tab_right[chan].g;
    b = color_tab_right[chan].b;
  }

  temp = r + g + b;

  if (temp > 0)
  {
    if (dir == 1) //moving left
    {
      for (rw = 0; rw < MAX_ROW/2; rw ++)
      {
        x = (int16_t)xpos + 6 - rw;
        if (rw == (MAX_ROW/2-1))
          x ++;

        for (idx = 0; idx < 2; idx ++)
        {
          x += idx;

          if ((x >= 0) && (x < MAX_COL))
          {
            idx2 = col[rw][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
            idx2 = col[MAX_ROW - (rw+1)][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
          }
        }
      }
    }
    else //moving right
    {
      for (rw = 0; rw < MAX_ROW/2; rw ++)
      {
        x = (int16_t)xpos + rw - 6;
        if (rw == (MAX_ROW/2-1))
          x --;

        for (idx = 0; idx < 2; idx ++)
        {
          x += idx;

          if ((x >= 0) && (x < MAX_COL))
          {
            idx2 = col[rw][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
            idx2 = col[MAX_ROW - (rw+1)][x];
            led[idx2].r = r;
            led[idx2].g = g;
            led[idx2].b = b;
          }
        }
      }
    }
  }
}


#pragma CODE_SECTION(ripple, "ramCode")

//This function draws randomly moving ripples, 1 per channel per side.
void ripple(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx;
  uint16_t *buff;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  //Choose a new pattern to display - on a peak boundary
//  if (peak_flag & (disp_frames > MIN_ARROW_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;

    //Compute the left side.  Right side is a mirror image.
    //Create a new random arrangement of line segments in the work buffer.
    buff = (uint16_t *)&work_buff;

    for (idx = 0; idx < color_channels; idx ++)
    {
      //Create left channel arrow
      buff[0] = rnd(5) + 1;        //move factor
      buff[1] = rnd(4) + 1;        //pause factor
      buff[2] = rnd(MAX_COL/2);    //current X pos
      buff[3] = 0;                 //current count
      buff[4] = 1;                 //current state: 1=moving, 0=paused
      if (buff[2] < (MAX_COL/4))
        buff[5] = 2;               //dir: 2=moving right
      else
        buff[5] = 1;               //dir: 1=moving left

      //Create right channel arrow
      buff[6] = rnd(5) + 1;        //move factor
      buff[7] = rnd(4) + 1;        //pause factor
      buff[8] = MAX_COL/2 + rnd(MAX_COL/2);  //current X pos
      buff[9] = 0;                 //current count
      buff[10] = 1;                //current state: 1=moving, 0=paused
      if (buff[8] < (MAX_COL*3/4))
        buff[11] = 2;              //dir: 2=moving right
      else
        buff[11] = 1;              //dir: 1=moving left

      buff += 6 * 2; //position for the next channel
    }
  }

  clear_display();
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (idx = 0; idx < color_channels; idx ++)
  {
    //Move left channel arrow
    if (buff[4] == 1) //if moving
    {
      if (buff[5] == 1) //moving left
      {
        buff[2] --;
        if (buff[2] < 1)
          buff[5] = 2;
      }
      else //moving right
      {
        buff[2] ++;
        if (buff[2] > (PANEL_COL-3))
          buff[5] = 1;
      }

      buff[3] ++;
      if (buff[3] == buff[0]) //reached move count
      {
//        buff[4] = 0;
buff[4] = 1;
        buff[3] = 0;
      }
    }
    else //paused
    {
      buff[3] ++;
      if (buff[3] == buff[1]) //reached pause count
      {
        buff[4] = 1;
        buff[3] = 0;
      }
    }

    draw_arrow(led, idx, 0, buff[2], buff[5]);

    //Move right channel arrow
    if (buff[10] == 1) //if moving
    {
      if (buff[11] == 1) //moving left
      {
        buff[8] --;
        if (buff[8] < PANEL_COL)
          buff[11] = 2;
      }
      else //moving right
      {
        buff[8] ++;
        if (buff[8] > (MAX_COL-3))
          buff[11] = 1;
      }

      buff[9] ++;
      if (buff[9] == buff[6]) //reached move count
      {
        buff[10] = 0;
        buff[9]  = 0;
      }
    }
    else //paused
    {
      buff[9] ++;
      if (buff[9] == buff[7]) //reached pause count
      {
        buff[10] = 1;
        buff[9]  = 0;
      }
    }

    draw_arrow(led, idx, 1, buff[8], buff[11]);

    buff += 6 * 2; //position for the next channel
  }
}


#define MIN_CURVE_FRAMES              (FRAMES_PER_SEC * 10)
#define MAX_RANGE                     (MAX_ROW - CURVE_WIDTH)
#define MAX_X                          2.0
#define X_INC                         (MAX_X * 2.0 / (float)MAX_COL)
#define X_INC2                        (MAX_X / (float)MAX_COL)
#define MIN_A                          0.4
#define MAX_A                          0.8
#define NUM_A                          25
#define A_INC                         ((MAX_A - MIN_A) / NUM_A)
#define MIN_B                          4.5
#define MAX_B                          6.5
#define NUM_B                          25
#define B_INC                         ((MAX_B - MIN_B) / NUM_B)
#define MIN_C                          5.0
#define MAX_C                          16.0
#define NUM_C                          22
#define C_INC                         ((MAX_C - MIN_C) / NUM_C)

#if 0
uint16_t curve_flip;

#pragma DATA_SECTION(curve_rows, "ramConsts")
const uint16_t curve_rows[COLOR_CHANNELS] = {2, 2, 1, 2, 2, 1, 2, 2, 1, 1};
#define CURVE_WIDTH                    4 /* width of channels 0+1 */

#pragma CODE_SECTION(curves, "ramCode")

//This function creates a display where the channels are drawn as a polynomial
//curve:
//
//  y = a * x^5 - b * x^3 + c * x
//
//    0.4 <= a <= 0.8
//    4.5 <= b <= 6.5
//    5.0 <= c <= 16.0
//   -2.0 <= x <= 2.0
//
//The curve is then scaled and shifted to fit into rows 0 to (max - wave_width).
void curves(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  cv;
  uint16_t  rw, cl;
  int16_t   pos, idx3;
  int16_t  *buff;
  float     a, b, c;
  float     x, x3, x5;
  float     max, min;
  float     range, scale;
  float     shift, temp;
  float    *curv;
  LED_MAIN *cv_clr;

  if (peak_flag & (disp_frames > MIN_CURVE_FRAMES))
  {
    disp_frames = 0;
    curve_flip  = rnd(100) & 0x01;
    buff = (int16_t *)&work_buff[0];
    curv = (float *)&work_buff[MAX_COL*2];
    pos = rnd(MAX_ROW - CURVE_WIDTH - 1); //get starting point on left side

    x = -MAX_X;
    a = MIN_A + ((float)rnd(NUM_A) * A_INC);
    b = MIN_B + ((float)rnd(NUM_B) * B_INC);
    c = MIN_C + ((float)rnd(NUM_C) * C_INC);
    min = 99999.9;
    max =-99999.9;

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      //Compute the curve's path left to right. This will be the bottom row
      //of channel 0. All others are relative to it, wrapping top to bottom.

      x3 = x * x;
      x5 = x * x3 * x3;
      x3*= x;

      temp = (a * x5) - (b * x3) + (c * x);
      curv[cl] = temp;

      if (temp > max) max = temp;
      if (temp < min) min = temp;

      x += X_INC;
    }

    range = max - min;
    scale = (float)MAX_RANGE / range;
    shift = min * scale * -1.0;

    //Scale and shift the curve into the display rows
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      buff[cl] = (int16_t)((curv[cl] * scale) + shift + 0.50);
    }
  }

  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (int16_t *)&work_buff[0];

  for (cl = 0; cl < MAX_COL; cl ++)
  {
    cv  = 0;     //start each column with channel 0
    idx = 0;

    if (curve_flip)
      pos = (MAX_ROW-1)-buff[cl]; //bottom row for channels 0
    else
      pos = buff[cl];             //bottom row for channels 0

    if (cl < (MAX_COL/2))
      cv_clr = &color_tab_left[cv];
    else
      cv_clr = &color_tab_right[cv];

    for (rw = 0; rw < MAX_ROW; rw ++)
    {
      idx3 = col[pos][cl];
      led[idx3].r = cv_clr->r;
      led[idx3].g = cv_clr->g;
      led[idx3].b = cv_clr->b;

      idx ++;
      if (idx == curve_rows[cv])
      {
        idx = 0;
        cv ++;
        if (cv == color_channels)
          cv = 0;

        if (cl < (MAX_COL/2))
          cv_clr = &color_tab_left[cv];
        else
          cv_clr = &color_tab_right[cv];
      }

      if (curve_flip)
      {
        pos --;
        if (pos < 0)
          pos = MAX_ROW-1;
      }
      else
      {
        pos ++;
        if (pos >= MAX_ROW)
          pos = 0;
      }
    }
  }
}
#endif


#if 0
//table to map color channels to curves, two channels per curve
#pragma DATA_SECTION(curve_chan, "ramConsts")
#if COLOR_CHANNELS == 3
const uint16_t curve_chan[5 * 2] = {2, 2, 2, 2,
                                    1, 1, 1, 0,
                                    0, 0};
#endif
#if COLOR_CHANNELS == 5
const uint16_t curve_chan[5 * 2] = {4, 4, 3, 3,
                                    1, 1, 2, 2,
                                    0, 0};
#endif
#if COLOR_CHANNELS == 7
const uint16_t curve_chan[5 * 2] = {5, 6, 3, 3,
                                    4, 4, 2, 2,
                                    0, 1};
#endif
#if COLOR_CHANNELS == 10
const uint16_t curve_chan[5 * 2] = {2, 3, 9, 8,
                                    7, 6, 4, 5,
                                    0, 1};
#endif

#pragma CODE_SECTION(curve2, "ramCode")

//This function creates a display where the channels are drawn as polynomial
//curves:
//
//  y = a * x^5 - b * x^3 + c * x
//
//    0.4 <= a <= 0.8
//    4.5 <= b <= 6.5
//    5.0 <= c <= 16.0
//   -2.0 <= x <= 2.0
//
//The 10 channels are combined into 5, and 5 curves are calculated. The curves
//are then scaled and shifted to fit into rows 0 to (max - wave_width).
void curve2(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx;
  uint16_t  cv1;
  uint16_t  clr_scale1;
  uint16_t  cl;
  int16_t   pos1;
  int16_t   idx3;
  uint16_t  r1, g1, b1;
  float     a, b, c;
  float     x, x3, x5;
  float     max, min;
  float     range, scale;
  float     shift, temp;
  float    *buff;
  float    *tcurv, *curv;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  buff = (float *)&work_buff[0];        //storage for display row values

//  if (peak_flag & (disp_frames > MIN_CURVE_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;

    //Recompute the curves static data
    for (idx = 0; idx < 5; idx ++)
    {
      buff[idx  ]  = rnd(100) & 0x01; //curve_flip
      buff[idx+5]  = MIN_A + ((float)rnd(NUM_A) * A_INC); // a
      buff[idx+10] = MIN_B + ((float)rnd(NUM_B) * B_INC); // b
      buff[idx+15] = MIN_C + ((float)rnd(NUM_C) * C_INC); // c
      buff[idx+20] = 1.0; //current inc/dec of C term
      buff[idx+25] = (float)((idx/2)+1); //speed
      idx3 = rnd(100) & 0x03;

      switch(idx3)
      {
         case 0:
           buff[idx+30] = -MAX_X; //starting x
           buff[idx+35] = X_INC;  //x increment
           break;
         case 1:
           buff[idx+30] = 0;
           buff[idx+35] = X_INC2;
           break;
         case 2:
           buff[idx+30] = MAX_X; //starting x
           buff[idx+35] = -X_INC;  //x increment
           break;
         case 3:
           buff[idx+30] = -MAX_X; //starting x
           buff[idx+35] = X_INC2;  //x increment
           break;
      }
    }
  }

  //Compute/update the curves data
  for (idx = 0; idx < 5; idx ++)
  {
    tcurv = (float *)&work_buff[100];  //temp storage
    curv  = (float *)&work_buff[200+(idx*100)];  //curve data

    a = buff[idx+5];
    b = buff[idx+10];
    c = buff[idx+15];
    idx3 = (int16_t)buff[idx+25];

//don't update the curve when zero...?
    if ((disp_frames % idx3) == 0)
    {
      c += ((float)C_INC * buff[idx+20]);
      if (c > MAX_C)
      {
        c = MAX_C;
        buff[idx+20] = -1.0;
      }
      else
      if (c < MIN_C)
      {
        c = MIN_C;
        buff[idx+20] = 1.0;
      }
      buff[idx+15] = c;
    }
    min = 99999.9;
    max =-99999.9;
    x   = buff[idx+30];

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      x3 = x * x;
      x5 = x * x3 * x3;
      x3*= x;

      temp = (a * x5) - (b * x3) + (c * x);
      tcurv[cl] = temp;

      if (temp > max) max = temp;
      if (temp < min) min = temp;
      x += buff[idx+35];
    }

    range = max - min;
    scale = (float)(MAX_ROW-2) / range;
    shift = min * scale * -1.0;

    //Scale and shift the curve into the display rows
    for (cl = 0; cl < MAX_COL; cl ++)
    {
      curv[cl] = (int16_t)((tcurv[cl] * scale) + shift + 0.50);
    }
  }


  clear_display();
  build_color_table(left_val, right_val);

  disp_frames ++;

  for (idx = 0; idx < 5; idx ++)
  {
    curv = (float *)&work_buff[200+(idx*100)];  //curve data

    for (cl = 0; cl < MAX_COL; cl ++)
    {
      if (cl == 0) //get left side channel brightness
      {
        idx3 = idx * 2;
        cv1  = curve_chan[idx3];
        clr_scale1 = left_val[cv1];
        if (left_val[curve_chan[idx3 + 1]] > clr_scale1)
          clr_scale1 = left_val[curve_chan[idx3 + 1]];

        get_rgb(rainbow[chan_clr[cv1]], &r1, &g1, &b1);
        r1 = (r1 * clr_scale1) >> 8;
        g1 = (g1 * clr_scale1) >> 8;
        b1 = (b1 * clr_scale1) >> 8;
      }
      else
      if (cl == (MAX_COL/2)) //get right side channel brightness
      {
        idx3 = idx * 2;
        cv1  = curve_chan[idx3];
        clr_scale1 = right_val[cv1];
        if (right_val[curve_chan[idx3 + 1]] > clr_scale1)
          clr_scale1 = right_val[curve_chan[idx3 + 1]];

        get_rgb(rainbow[chan_clr[cv1]], &r1, &g1, &b1);
        r1 = (r1 * clr_scale1) >> 8;
        g1 = (g1 * clr_scale1) >> 8;
        b1 = (b1 * clr_scale1) >> 8;
      }

      if (buff[idx]) //curve flip
        pos1 = (MAX_ROW-2)-curv[cl]; //bottom row for curve 0..2
      else
        pos1 = curv[cl];             //bottom row for curve 0..2

      if (clr_scale1 > 0)
      {
        idx3 = col[pos1][cl];
        led[idx3].r = r1;
        led[idx3].g = g1;
        led[idx3].b = b1;
        idx3 = col[pos1+1][cl];
        led[idx3].r = r1;
        led[idx3].g = g1;
        led[idx3].b = b1;
      }
    }
  }
}
#endif


#if 0
#pragma CODE_SECTION(chevron, "ramCode")

#define MIN_CHEVRON_FRAMES             (FRAMES_PER_SEC * 2)
uint16_t chevron_flip[4];
#define CHEV_PAT                       4
#pragma DATA_SECTION(chevron_db, "ramConsts")
const char chevron_db[CHEV_PAT] = {0x0, 0xf, 0x3, 0xc};

//This function creates a display with 4 vertical chevrons. They will independently
//flip upside down, while maintaining a symmetric arrangement.
void chevron(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx3;
  int16_t   cv;
  uint16_t  rw, cl, qd;
  int16_t   row, sx, ex;
  uint16_t  r, g, b;

  if (peak_flag & (disp_frames > MIN_CHEVRON_FRAMES))
  {
    disp_frames = 0;
    idx3 = chevron_db[rnd(CHEV_PAT)];

    for (idx = 0; idx < 4; idx ++)
      chevron_flip[idx] = (idx3 >> idx) & 1;
  }

  disp_frames ++;

  build_color_table(left_val, right_val);

  for (qd = 0; qd < 4; qd ++)
  {
    sx = qd * (MAX_COL/4); //get starting x column index

    for (cl = 0; cl < MAX_COL/8; cl ++)
    {
      ex  = sx + (MAX_COL/4 - 1 - cl); //symmetric ending x column

      if (chevron_flip[qd])
      {
        row = MAX_ROW - 1 - cl;
      }
      else
      {
        row = cl;
      }

      idx = 0;
      cv  = 0;

      if (qd < 2)
      {
        r = color_tab_left[cv].r;
        g = color_tab_left[cv].g;
        b = color_tab_left[cv].b;
      }
      else
      {
        r = color_tab_right[cv].r;
        g = color_tab_right[cv].g;
        b = color_tab_right[cv].b;
      }

      for (rw = 0; rw < MAX_ROW; rw ++)
      {
        //Write the left side pixels:
        idx3 = col[row][sx+cl];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;
        idx3 = col[row][ex];
        led[idx3].r = r;
        led[idx3].g = g;
        led[idx3].b = b;

        idx ++;
        if (idx == curve_rows[cv])
        {
          idx = 0;
          cv ++;
          if (cv == color_channels)
            cv = 0;

          if (qd < 2)
          {
            r = color_tab_left[cv].r;
            g = color_tab_left[cv].g;
            b = color_tab_left[cv].b;
          }
          else
          {
            r = color_tab_right[cv].r;
            g = color_tab_right[cv].g;
            b = color_tab_right[cv].b;
          }
        }

        if (chevron_flip[qd])
        {
          row --;
          if (row < 0)
            row = MAX_ROW-1;
        }
        else
        {
          row ++;
          if (row == MAX_ROW)
            row = 0;
        }
      }
    }
  }
}
#endif


#define SYM_NOUN                       33
#pragma DATA_SECTION(sym_noun, "ramConsts")
const char sym_noun[SYM_NOUN][4] =
{"BABY", "BIRD", "BONE", "BUM ",
 "BOY ", "CAT ", "COW ", "PIE ",
 "DAY ", "DOG ", "DRUM", "EYE ",
 "FIRE", "FOOD", "FOOT", "GIRL", "HAND", "HEAD",
 "LOVE", "MEAT", "MOON", "CAKE",
 "PIG ", "LIP ", "MAN ", "STAR",
 "TOY ", "YOU ", "TEAM", "THEM",
 "WHO ", "WOOD", "WORM"};

#define SYM_VERB                       34
#pragma DATA_SECTION(sym_verb, "ramConsts")
const char sym_verb[SYM_VERB][4] =
{"BANG", "BEAT", "BLOW", "BOOM", "CHEW", "CHUG",
 "EAT ", "GOT ", "HEY ", "JUMP", "LIKE", "LOVE",
 "MAKE", "MOO ", "NOT ", "PEE ", "POO ", "POP ",
 "POW ", "RUN ", "PLUG", "SING",
 "SAY ", "SEE ", "SING", "SLAM", "STOP",
 "TRY ", "WAKE", "WHAT", "YES ",
 "ZAP ", "ZING", "ZOOM"};

#define SYM_ADJ                        31
#pragma DATA_SECTION(sym_adj, "ramConsts")
const char sym_adj[SYM_ADJ][4] =
{"BAD ", "BIG ", "DAMN", "DRY ", "EASY", "FAT ",
 "COLD", "COOL", "DEEP", "FAST", "HOT ", "MAD ",
 "ODD ", "OLD ", "WET ", "WOW ", "OILY",
 "FUZZ", "GOOD", "HARD", "LONG",
 "LOUD", "SING", "SLIM", "SOFT", "SLOW",
 "THIN", "TORN", "UGLY", "WARM", "WILD"};

uint16_t get_verb(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_VERB);

  if (sym_verb[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_verb[idx][0];
  str[1] = sym_verb[idx][1];
  str[2] = sym_verb[idx][2];
  str[3] = sym_verb[idx][3];

  return(siz);
}

uint16_t get_noun(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_NOUN);

  if (sym_noun[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_noun[idx][0];
  str[1] = sym_noun[idx][1];
  str[2] = sym_noun[idx][2];
  str[3] = sym_noun[idx][3];

  return(siz);
}

uint16_t get_adj(char *str)
{
  uint16_t siz;
  uint16_t idx = rnd(SYM_ADJ);

  if (sym_adj[idx][3] == ' ')
    siz = 3;
  else
    siz = 4;

  str[0] = sym_adj[idx][0];
  str[1] = sym_adj[idx][1];
  str[2] = sym_adj[idx][2];
  str[3] = sym_adj[idx][3];

  return(siz);
}


#pragma CODE_SECTION(vertical, "ramCode")

#define MIN_VERTICAL_FRAMES           (FRAMES_PER_SEC * 8)
#define VERT_LINE_END                 (MAX_ROW * (MAX_COL/2/2))
#define VERT_MIN_SEG                  3
#define VERT_MAX_SEG                  8
#define VERT_COL_SEG                  (MAX_ROW / VERT_MIN_SEG)

uint16_t sym_do_text;
uint16_t chan_used;
uint16_t sym_size[2];
uint16_t sym_text_row[8];
char     sym_text[2][4];


//This function creates a display that is L-R symmetric, creating small moving
//vertical pieces. Randomly it is overlaid with pseudo-word text.
void vertical(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  int16_t   idx, rot, cv;
  uint16_t  idx2, idx3;
  uint16_t  next, len;
  uint16_t  scale;
  uint16_t  rw, cl, xpos;
  uint16_t  r, g, b;
  uint32_t  r1, g1, b1;
  uint32_t  color, temp;
  uint16_t *buff;
  uint16_t  startmask;
  uint16_t  currmask;
  LED_MAIN *cv_clr1;
  LED_MAIN *cv_clr2;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  //Set a local peak
  if (disp_frames >= MIN_VERTICAL_FRAMES)
    peak_flag = 1;

  //Choose a new pattern to display - on a peak boundary
//  if (peak_flag & (disp_frames > MIN_VERTICAL_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;
    startmask = 0;
    chan_used = color_channels;

    sym_do_text = rnd(100);
    if (sym_do_text < 34)
      sym_do_text = 1;
    else
      sym_do_text = 0;

    if (sym_do_text)
    {
      //don't allow bass channels in the background (they are used for text)
      for (idx = 0; idx < color_channels; idx ++)
      {
        if (channel_freq[idx] < 2) //skip bass frequency bins
        {
          chan_used --;
        }
        else
        {
          startmask |= 1 << idx;
        }
      }

      for (idx = 0; idx < 8; idx ++)
      {
        sym_text_row[idx] = rnd(MAX_ROW-FONT_HEIGHT-3) + 3;

        if (idx > 0)
        {
          if (sym_text_row[idx] > (sym_text_row[idx-1]+2))
            sym_text_row[idx] = sym_text_row[idx-1]+2;
          if (sym_text_row[idx] < (sym_text_row[idx-1]-2))
            sym_text_row[idx] = sym_text_row[idx-1]-2;
        }
      }

      for (idx = 0; idx < 2; idx ++)
      {
        idx2 = rnd(3);
        switch (idx2)
        {
          case 0: //Verb + Noun
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_verb(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_noun(&sym_text[idx][0]);
            }
            break;
          case 1: //Adj + Noun
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_adj(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_noun(&sym_text[idx][0]);
            }
            break;
          case 2: //Verb + Adj
            if (idx == 0) //get a verb
            {
              sym_size[idx] = get_verb(&sym_text[idx][0]);
            }
            else // get a noun
            {
              sym_size[idx] = get_adj(&sym_text[idx][0]);
            }
            break;
        }
      }
    }
    else //no text, use all channels
    {
      for (idx = 0; idx < color_channels; idx ++)
        startmask |= 1 << idx;
    }

    currmask = startmask;

    //Compute the left side.  Right side is a mirror image.
    //Create a new random arrangement of line segments in the work buffer.
    buff = (uint16_t *)&work_buff;

    for (cl = 0; cl < MAX_COL/2; cl += 2)
    {
      idx  = 0;
      idx3 = 2;
      if ((cl % 4) == 0)
        buff[0] = rnd(6) + 10;   //store the speed factor, 10 to 15
      else
        buff[0] = rnd(5) + 3;   //store the speed factor, 3 to 7
      buff[1] = MAX_ROW/2; //current rotation

      while (idx < MAX_ROW)
      {
        idx2 = rnd(99999);
        len = (idx2 % (VERT_MAX_SEG-VERT_MIN_SEG)) + VERT_MIN_SEG;    //line length

        cv = find_next_color(startmask, &currmask);

        if ((len + idx) > MAX_ROW)
          len = MAX_ROW - idx - 1;

        //don't let len equal 1
        if (len < 2)
          len = 0xff; //abort the end of the column

        buff[idx3++] = cv;   //store the channel in word 0
        buff[idx3++] = len;  //store the segment length in word 1

        idx += len + 1;
      }

      buff[idx3++] = 0xff;   //mark the end
      buff[idx3++] = 0xff;

      buff += (VERT_MAX_SEG + 1) * 2; //position for the next column
    }
  }

  clear_display();
  build_color_table(left_val, right_val);

  disp_frames ++;
  buff = (uint16_t *)&work_buff;

  for (cl = 0; cl < MAX_COL/2; cl += 2)
  {
    idx2 = buff[0]; //get the column's speed
    rot = buff[1];
    if ((disp_frames % idx2) == 0) //update the rotation and save
    {
      rot --;
      if (rot < 0)
        rot = MAX_ROW-1;
      buff[1] = rot;
    }

    idx  = 0;
    idx3 = 4;
    cv   = buff[2];
    len  = buff[3];
    next = len;

    for (rw = 0; rw < MAX_ROW; rw ++)
    {
      if (idx == next)
      {
        cv  = buff[idx3++];
        len = buff[idx3++];
        next += len;
        //leave a blank row
        rw ++;
        rot --;
        if (rot < 0)
          rot = MAX_ROW-1;

        if ((len == 0xff) || (len < 2)) break;
      }

      //Get the base color for this line
      cv_clr1 = &color_tab_left[cv];
      cv_clr2 = &color_tab_right[cv];

      idx2 = col[rot][cl];
      led[idx2].r = cv_clr1->r;     //left side
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;
      idx2 = col[rot][cl+1];
      led[idx2].r = cv_clr1->r;
      led[idx2].g = cv_clr1->g;
      led[idx2].b = cv_clr1->b;

      idx2 = col[rot][MAX_COL-1-cl]; //right side
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;
      idx2 = col[rot][MAX_COL-2-cl];
      led[idx2].r = cv_clr2->r;
      led[idx2].g = cv_clr2->g;
      led[idx2].b = cv_clr2->b;

      idx ++;
      rot --; //update for the next iteration
      if (rot < 0)
        rot = MAX_ROW-1;
    }

    buff += (VERT_MAX_SEG + 1) * 2; //position for the next column
  }

  if (sym_do_text)
  {
    temp = rainbow[chan_clr[color_channels][0]];
    get_rgb(temp, &r, &g, &b);

    //Combine channels 0 and 1 into a single channel.
    //Don't overwrite higher channels when there's no bass.
    if ((left_val[0] > 0) && (left_val[1] > 0))
    {
      scale = (left_val[0] > left_val[1]) ? left_val[0] : left_val[1];
      r1 = (r * scale);
      g1 = (g * scale);
      b1 = (b * scale);
      color = ((r1 << 8) & 0xff0000) +
               (g1 & 0x00ff00) +
               (b1 >> 8);

      xpos = MAX_COL/2;

      for (idx = sym_size[0]-1; idx >= 0; idx --) //left side
      {
        idx2 = font_get_idx(sym_text[0][idx]);
        xpos -= (font_char_len(idx2) + 1);
        draw_char(led, xpos, sym_text_row[idx], idx2, color);
      }
    }

    if ((right_val[0] > 0) && (right_val[1] > 0))
    {
      scale = (right_val[0] > right_val[1]) ? right_val[0] : right_val[1];
      r1 = (r * scale);
      g1 = (g * scale);
      b1 = (b * scale);
      color = ((r1 << 8) & 0xff0000) +
               (g1 & 0x00ff00) +
               (b1 >> 8);

      xpos = MAX_COL/2 + 1;

      for (idx = 0; idx < sym_size[1]; idx ++) //right side
      {
        idx2 = font_get_idx(sym_text[1][idx]);
        draw_char(led, xpos, sym_text_row[idx], idx2, color);
        xpos += font_char_len(idx2) + 1;
      }
    }
  }
}


#pragma CODE_SECTION(fill_rect, "ramCode")

void fill_rect(uint16_t bot_rw, uint16_t left_cl, uint16_t rsiz, uint16_t csiz, LED_MAIN *main, LED_MAIN clr)
{
  uint16_t rw, cl, idx;

  for (rw = 0; rw < rsiz; rw ++)
  {
    for (cl = 0; cl < csiz; cl ++)
    {
      idx = col[bot_rw + rw][left_cl + cl];
      main[idx].r = clr.r;
      main[idx].g = clr.g;
      main[idx].b = clr.b;
    }
  }
}


#pragma CODE_SECTION(two_by_two, "ramCode")

#define MIN_TBT_FRAMES                (FRAMES_PER_SEC * 10)

#ifdef SLAVE
uint32_t prev_mode;
#endif

uint16_t tbt_cl_size = TBT_STEP;

//This function creates a display where each 2x2 pixel subarray is assigned a
//random color organ channel.  When the peak flag is set (after a minumum number
//of frames), it will shuffle the 2x2s. For the large array, 3x3s are created.
void two_by_two(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val, uint32_t mode)
{
  int16_t  idx2, idx3;
  int16_t  rw, cl;
  int16_t  idx, blk_cnt;
  LED_MAIN  cv_clr;
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  switch (mode)
  {
    case 0: //fixed arrow
    case 1: //fixed fade
      tbt_cl_size = PANEL_COL / (color_channels + 2);
      break;
    default:
      tbt_cl_size = TBT_STEP;
  }

  blk_cnt = PANEL_COL / tbt_cl_size;

  //Choose a new pattern to display - on a peak boundary
//  if (peak_flag & (disp_frames > MIN_TBT_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;

    for (rw = 0; rw < TBT_ROW; rw ++)
    {
      for (cl = 0; cl < TBT_COL/2; cl ++)
      {
        tbt_map[rw][cl] = 99; //initialize to all empty
      }
    }


    switch (mode)
    {
      case 0: //fixed arrow

        //create a symmetric, fixed row-based "arrow" pattern
        for (cl = 0; cl < blk_cnt; cl ++)
          if (cl < color_channels)
            tbt_map[2][cl] = cl;

        for (rw = 0; rw < 2; rw ++) //rows 0 and 4, 1 and 3
        {
          for (cl = 0; cl < color_channels; cl ++) //fill one half of the columns
          {
            tbt_map[rw]  [cl+(2-rw)] = tbt_map[2][cl];
            tbt_map[4-rw][cl+(2-rw)] = tbt_map[2][cl];
          }
        }
        break;

      case 1: //fixed fade

        //create a symmetric, fading gradient pattern
        for (cl = 0; cl < blk_cnt; cl ++)
          if (cl < color_channels)
            tbt_map[2][cl] = cl;

        for (rw = 0; rw < 2; rw ++) //rows 0 and 4, 1 and 3
        {
          idx2 = 2 - rw;
          idx3 = rw - 2;

          for (cl = 0; cl < blk_cnt; cl ++) //fill one half of the columns
          {
            if ((cl+idx2) < color_channels)
              tbt_map[rw][cl] = cl+idx2;

            if ((cl+idx3) <= 0)
              tbt_map[4-rw][cl] = 0;
            else if ((cl+idx3) < color_channels)
              tbt_map[4-rw][cl] = cl+idx3;
          }
        }
        break;

      case 2: //random blocks
        {
          uint16_t startmask = 0;
          uint16_t currmask;

          if (mode > 1) //not a fixed pattern
          {
            for (idx = 0; idx < color_channels; idx ++)
              startmask |= 1 << idx;
          }

          currmask = startmask;

          for (rw = 0; rw < TBT_COL; rw ++)
          {
            for (cl = 0; cl < blk_cnt; cl ++) //fill one half of the columns
            {
              idx2 = rnd(color_channels);
              idx2 = find_next_color(startmask, &currmask);

              tbt_map[rw][cl] = idx2;
            }
          }
        }
        break;
    }

    //L/R panel invert
    if (rnd(100) > 50)
    {
      for (rw = 0; rw < TBT_ROW; rw ++)
      {
        for (cl = 0; cl < blk_cnt/2; cl ++)
        {
          idx3 = tbt_map[rw][cl];
          tbt_map[rw][cl] = tbt_map[rw][blk_cnt-(cl+1)];
          tbt_map[rw][blk_cnt-(cl+1)] = idx3;
        }
      }
    }

    //top/bottom invert
    if (rnd(100) > 50)
    {
      for (rw = 0; rw < TBT_ROW/2; rw ++)
      {
        for (cl = 0; cl < blk_cnt; cl ++)
        {
          idx3 = tbt_map[rw][cl];
          tbt_map[rw][cl] = tbt_map[TBT_ROW-1-rw][cl];
          tbt_map[TBT_ROW-1-rw][cl] = idx3;
        }
      }
    }

    clear_display();
  }

  disp_frames ++;
  build_color_table(left_val, right_val);

  blk_cnt = PANEL_COL / tbt_cl_size;


  for (rw = 0; rw < TBT_ROW; rw ++)
  {
    for (cl = 0; cl < blk_cnt; cl ++)
    {
      idx = tbt_map[rw][cl]; //color of this block

      //Scale the brightness to the current channel intensity
      if (idx == 99) //set to black
      {
        cv_clr.r = 0;
        cv_clr.g = 0;
        cv_clr.b = 0;
      }
      else
      {
        cv_clr = color_tab_left[idx];
      }

      //draw in the left panel
      fill_rect(rw*TBT_STEP+1, PANEL_COL-((cl+1)*tbt_cl_size), TBT_STEP, tbt_cl_size, led, cv_clr);

      if (rw == 0) //set row 0 to 25% of row 1
      {
        for (idx2 = 0; idx2 < tbt_cl_size; idx2 ++)
        {
          idx3 = col[0][PANEL_COL-((cl+1)*tbt_cl_size)+idx2];
          led[idx3].r = cv_clr.r >> 3;
          led[idx3].g = cv_clr.g >> 3;
          led[idx3].b = cv_clr.b >> 3;
        }
      }

      //draw in the right panel
      if (idx != 99)
      {
        cv_clr = color_tab_right[idx];
      }
      fill_rect(rw*TBT_STEP+1, PANEL_COL+(cl*tbt_cl_size), TBT_STEP, tbt_cl_size, led, cv_clr);

      if (rw == 0) //set row 0 to 25% of row 1
      {
        for (idx2 = 0; idx2 < tbt_cl_size; idx2 ++)
        {
          idx3 = col[0][PANEL_COL+(cl*tbt_cl_size)+idx2];
          led[idx3].r = cv_clr.r >> 3;
          led[idx3].g = cv_clr.g >> 3;
          led[idx3].b = cv_clr.b >> 3;
        }
      }
    }
  }
}


#pragma CODE_SECTION(draw_bar, "ramCode")

void draw_bar(LED_MAIN *led, int16_t x, uint16_t y, uint16_t w, uint16_t r, uint16_t g, uint16_t b)
{
  uint16_t idx, idx2, idx3;

  for (idx = 0; idx < y; idx ++)
  {
    for (idx2 = 0; idx2 < w; idx2 ++)
    {
      idx3 = col[idx][idx2+x];
      led[idx3].r = r;
      led[idx3].g = g;
      led[idx3].b = b;
    }
  }
}


#pragma CODE_SECTION(equalizer, "ramCode")
#define EQ_HOLD       1

//This function creates an audio equalizer with a vertical bar for each
//left and right channel.
void equalizer(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2, idx3, t;
  uint16_t  r, g, b;
  uint16_t  x, y, w;
  uint32_t  color;
  uint16_t *left;
  uint16_t *right;
  uint16_t *lcnt;
  uint16_t *rcnt;
  LED_MAIN *led;


//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

  left  = (uint16_t *)&work_buff[0];
  right = (uint16_t *)&work_buff[MAX_CHANNELS];
  lcnt  = (uint16_t *)&work_buff[MAX_CHANNELS*2];
  rcnt  = (uint16_t *)&work_buff[MAX_CHANNELS*3];

  if (peak_flag) //initialize the bins
  {
    memset(left, 0,  color_channels * sizeof(uint16_t));
    memset(right, 0, color_channels * sizeof(uint16_t));
  }
  else
  {
    for (idx = 0; idx < color_channels; idx ++)
    {
      //if new volume is louder, use it directly.
      if (left[idx] <= left_val[idx])
      {
        lcnt[idx] = EQ_HOLD;
      }
      else //decrease volume slowly
      {
        if (lcnt[idx] == 0)
        {
          t = 0;
          idx3 = (left[idx] >> 1); //%50 (fastest)
          for (idx2 = 0; idx2 < led_response; idx2++)
          {
            t += idx3;
            idx3 >>= 1; //add 25% then 12.5 for 75% and 87.5% (slowest)
          }

          if (left_val[idx] < t)
            left_val[idx] = t;
        }
        else
        {
          left_val[idx] = left[idx]; //hold
          lcnt[idx] --;
        }
      }

      //if new volume is louder, use it directly.
      if (right[idx] <= right_val[idx])
      {
        rcnt[idx] = EQ_HOLD;
      }
      else //decrease volume slowly
      {
        if (rcnt[idx] == 0)
        {
          t = 0;
          idx3 = (right[idx] >> 1); //%50 (fastest)
          for (idx2 = 0; idx2 < led_response; idx2++)
          {
            t += idx3;
            idx3 >>= 1; //add 25% then 12.5 for 75% and 87.5% (slowest)
          }

          if (right_val[idx] < t)
            right_val[idx] = t;
        }
        else
        {
          right_val[idx] = right[idx]; //hold
          rcnt[idx] --;
        }
      }
    }
  }

  //clear the display each frame
  clear_display();
  w = PANEL_COL / color_channels;

  for (idx = 0; idx < color_channels; idx ++) //channel loop
  {
    color = channel_color[idx];
    get_rgb(color, &r, &g, &b);

    x = PANEL_COL - ((idx + 1) * w);
    y = left[idx] >> 4;
    if (y > 0)  y ++;
    if (y > 16) y = 16;

    draw_bar(led, x, y, w, r, g, b);

    x = PANEL_COL + (idx * w);
    y = right[idx] >> 4;
    if (y > 0)  y ++;
    if (y > 16) y = 16;

    draw_bar(led, x, y, w, r, g, b);
  }

  memcpy(left, left_val,  color_channels * sizeof(uint16_t));
  memcpy(right, right_val, color_channels * sizeof(uint16_t));
}


#pragma CODE_SECTION(equalizer_hires, "ramCode")

//This function creates an audio equalizer with vertical bars, but each
//column is a different mix of frequency bins:
//   0..15 (16), 16..46x2 ea(16), 48..176x32 ea (4)
//Colors will blend each of the defined channels evenly.
void equalizer_hires(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2, idx3;
  int16_t   r, g, b;
  int16_t   r2, g2, b2;
  int16_t   ri, gi, bi;
  uint16_t  x, y;
  int16_t   col, t;
  int16_t   w;
  float     temp, max;
  uint32_t  color;
  uint16_t *left, *left2;
  uint16_t *right, *right2;
  uint16_t *lcnt;
  uint16_t *rcnt;
  uint32_t *clr_data;
  LED_MAIN *led;


  led = &led_ping[0];

  left  = (uint16_t *)&work_buff[0];
  right = (uint16_t *)&work_buff[PANEL_COL];
  left2 = (uint16_t *)&work_buff[PANEL_COL*2];
  right2= (uint16_t *)&work_buff[PANEL_COL*3];
  lcnt  = (uint16_t *)&work_buff[PANEL_COL*4];
  rcnt  = (uint16_t *)&work_buff[PANEL_COL*5];
  clr_data = (uint32_t *)&work_buff[PANEL_COL*6];

  w = PANEL_COL / (color_channels - 1); //columns between colors
  col = w * (color_channels - 1);       //total columns
//  rem = PANEL_COL - col;                //extra columns

  if (peak_flag) //initialize the column colors
  {
    idx2 = 0;
    idx3 = 0;

    memset(left2, 0,  PANEL_COL * sizeof(uint16_t));
    memset(right2, 0, PANEL_COL * sizeof(uint16_t));

    for (idx = 0; idx < col; idx ++)
    {
      if (idx2 == 0)
      {
        color = channel_color[idx3];
        r  = (color >> 16) & 0xff;
        g  = (color >>  8) & 0xff;
        b  = (color      ) & 0xff;
        color = channel_color[idx3+1];
        r2 = (color >> 16) & 0xff;
        g2 = (color >>  8) & 0xff;
        b2 = (color      ) & 0xff;
        t  = (r2 - r);
        ri = t / w;
        t  = (g2 - g);
        gi = t / w;
        t  = (b2 - b);
        bi = t / w;
      }

      clr_data[idx] = ((uint32_t)r << 16) + ((uint32_t)g << 8) + b;
      r += ri;
      g += gi;
      b += bi;

      idx2 ++;
      if (idx2 == w)
      {
        idx2 = 0;
        idx3 ++;
      }
    }
  }

  //Grab the first 16 range bins, skipping DC.
  idx2 = 1;

  for (idx = 0; idx < 16; idx ++)
  {
    //left channel
    temp = RFFTmagBuff1[idx2];
    temp = temp * temp;
    if (temp < (float)MAX_RGB_VAL)
      left[idx] = temp;
    else
      left[idx] = MAX_RGB_VAL;

    temp = RFFTmagBuff2[idx2];
    temp = temp * temp;
    if (temp < (float)MAX_RGB_VAL)
      right[idx] = temp;
    else
      right[idx] = MAX_RGB_VAL;

    idx2++;
  }

  //Grab the next 32 range bins, using 2 bins for each column.
  for (idx = 16; idx < 32; idx ++)
  {
    //left channel
    temp = RFFTmagBuff1[idx2];
    max  = RFFTmagBuff1[idx2+1];
    if (temp > max)
      max = temp;
    max = max * max;
    if (max < (float)MAX_RGB_VAL)
      left[idx] = max;
    else
      left[idx] = MAX_RGB_VAL;

    temp = RFFTmagBuff2[idx2];
    max  = RFFTmagBuff2[idx2+1];
    if (temp > max)
      max = temp;
    max = max * max;
    if (max < (float)MAX_RGB_VAL)
      right[idx] = max;
    else
      right[idx] = MAX_RGB_VAL;

    idx2 += 2;
  }

  //Grab the next 128 range bins, using 32 bins for each column.
  for (idx = 32; idx < col; idx ++)
  {
    //left channel
    max = 0;
    for (idx3 = 0; idx3 < 32; idx3++)
    {
      temp = RFFTmagBuff1[idx2+idx3];
      if (temp > max)
        max = temp;
    }
    max = max * max;
    if (max < (float)MAX_RGB_VAL)
      left[idx] = max;
    else
      left[idx] = MAX_RGB_VAL;

    max = 0;
    for (idx3 = 0; idx3 < 32; idx3++)
    {
      temp = RFFTmagBuff2[idx2+idx3];
      if (temp > max)
        max = temp;
    }

    max = max * max;
    if (max < (float)MAX_RGB_VAL)
      right[idx] = max;
    else
      right[idx] = MAX_RGB_VAL;

    idx2 += 32;
  }

  clear_display();

  for (idx = 0; idx < col; idx ++) //column loop
  {
    if (left2[idx] <= left[idx])
    {
      lcnt[idx] = EQ_HOLD;
    }
    else //decrease volume slowly
    {
      if (lcnt[idx] == 0)
      {
        t = 0;
        idx3 = (left2[idx] >> 1); //%50 (fastest)
        for (idx2 = 0; idx2 < led_response; idx2++)
        {
          t += idx3;
          idx3 >>= 1; //add 25% then 12.5 for 75% and 87.5% (slowest)
        }

        if (left[idx] < t)
          left[idx] = t;
      }
      else
      {
        left[idx] = left2[idx]; //hold
        lcnt[idx] --;
      }
    }

    if (right2[idx] <= right[idx])
    {
      rcnt[idx] = EQ_HOLD;
    }
    else //decrease volume slowly
    {
      if (rcnt[idx] == 0)
      {
        t = 0;
        idx3 = (right2[idx] >> 1); //%50 (fastest)
        for (idx2 = 0; idx2 < led_response; idx2++)
        {
          t += idx3;
          idx3 >>= 1; //add 25% then 12.5 for 75% and 87.5% (slowest)
        }

        if (right[idx] < t)
          right[idx] = t;
      }
      else
      {
        right[idx] = right2[idx]; //hold
        rcnt[idx] --;
      }
    }

    color = clr_data[idx];
    get_rgb(color, (uint16_t *)&r, (uint16_t *)&g, (uint16_t *)&b);

    x = PANEL_COL - (idx + 1);
    y = left[idx] >> 4;
    if (y > 0)  y ++;
    if (y > 16) y = 16;

    draw_bar(led, x, y, 1, r, g, b);

    x = PANEL_COL + idx;
    y = right[idx] >> 4;
    if (y > 0)  y ++;
    if (y > 16) y = 16;

    draw_bar(led, x, y, 1, r, g, b);
  }

  memcpy(left2, left,  PANEL_COL * sizeof(uint16_t));
  memcpy(right2, right, PANEL_COL * sizeof(uint16_t));
}


#if 0
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


  for (idx = 0; idx < color_channels; idx ++)
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
#endif


typedef struct
{
  uint16_t  state:2;   //0=not moving, 1=up, 2=down
  uint16_t  rate:3;    //x*5 frames. valid 1 fast ... 7 slow
  uint16_t  count:3;
  uint16_t  frames:10;
  uint16_t  x:7;
  uint16_t  y:7;
} lavadata;


//GLOBAL HERE FOR TESTING ONLY
//  uint16_t  x, xlim;
//  uint16_t  y, ylim;
#pragma CODE_SECTION(draw_blob, "ramCode")
void draw_blob(LED_MAIN *led, uint16_t channel, uint16_t blob, uint16_t lr, lavadata *lptr)
{
  uint16_t  idx;
  uint16_t  r, g, b;
  uint16_t  rw, cl;
  uint16_t  x, xlim;
  uint16_t  y, ylim;

  if (lr == 0) //left
  {
    r = color_tab_left[channel].r;
    g = color_tab_left[channel].g;
    b = color_tab_left[channel].b;
  }
  else //right
  {
    r = color_tab_right[channel].r;
    g = color_tab_right[channel].g;
    b = color_tab_right[channel].b;
  }

  //Don't draw a black blob (it won't move either)
  if ((r == 0) && (g == 0) && (b == 0))
    return;

  if (blob == 0)
  {
    if (lptr->count == 2)
    {
      xlim = 7;
      ylim = 4;
    }
    else
    {
      xlim = 6;
      ylim = 3;
    }
  }
  else
  {
    if (lptr->count == 2)
    {
      xlim = 4;
      ylim = 7;
    }
    else
    {
      xlim = 3;
      ylim = 6;
    }
  }

  //Draw the blob
  for (rw = 0; rw < ylim; rw ++)
  {
    for (cl = 0; cl < xlim; cl ++)
    {
      x = lptr->x + cl;
      y = lptr->y + rw;

      if ((x < MAX_COL) && (y < MAX_ROW))  //clip
      {
        if (((rw == 0) && (cl == 0)) ||    //remove corners
            ((rw == 0) && (cl == (xlim-1))) ||
            ((rw == (ylim-1)) && (cl == 0)) ||
            ((rw == (ylim-1)) && (cl == (xlim-1))))
        {
        }
        else
        {
          //Write the left side pixels:
          idx = col[y][x];
          led[idx].r = r;
          led[idx].g = g;
          led[idx].b = b;
        }
      }
    }
  }

  //Update the blob position and state
  lptr->frames ++;

  switch (lptr->state)
  {
    case 0:  //not moving
      if (lptr->frames == (lptr->rate * 14))
      {
        lptr->frames = 0;

        if (lptr->y < MAX_ROW/2)
        {
          lptr->state = 1;
        }
        else
        {
          lptr->state = 2;
        }
      }
      break;
    case 1:  //up
      if (lptr->frames == (lptr->rate * 7))
      {
        lptr->frames = 0;

        if (lptr->y < MAX_ROW-2)
        {
          lptr->y ++;
        }
        else
        {
          lptr->state = 0;
        }
      }
      break;
    case 2:  //down
      if (lptr->frames == (lptr->rate * 7))
      {
        lptr->frames = 0;

        if (lptr->y > 0)
        {
          lptr->y --;
        }
        else
        {
          lptr->state = 0;
        }
      }
      break;
  }
}


#if 0
#pragma CODE_SECTION(lavalamp, "ramCode")

#define MIN_LAVALAMP_FRAMES             (FRAMES_PER_SEC * 12)
#define LAVABLOBS                       2

#pragma DATA_SECTION(lava_order, "ramConsts")
#if COLOR_CHANNELS == 3
const uint16_t lava_order[COLOR_CHANNELS] = {1, 2, 0};
#endif
#if COLOR_CHANNELS == 5
const uint16_t lava_order[COLOR_CHANNELS] = {4, 2, 1, 3, 0};
#endif
#if COLOR_CHANNELS == 7
const uint16_t lava_order[COLOR_CHANNELS] = {6, 4, 5, 3, 1, 2, 0};
#endif
#if COLOR_CHANNELS == 10
const uint16_t lava_order[COLOR_CHANNELS] = {6, 8, 4, 1, 9, 5, 2, 7, 3, 0};
#endif

//This function creates a display that mimics a lavalamp.
void lavalamp(uint16_t peak_flag, uint16_t *left_val, uint16_t *right_val)
{
  uint16_t  idx, idx2, idx3;
  uint16_t  count;
  lavadata *lptr = (lavadata *)&work_buff[MAX_LEDS>>1];
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
 //   led = &led_pong[0];

//  if (peak_flag & (disp_frames > MIN_LAVALAMP_FRAMES))
  if (peak_flag)
  {
    disp_frames = 0;

    for (idx = 0; idx < color_channels; idx ++)
    {
      for (idx2 = 0; idx2 < 2; idx2 ++) //left/right
      {
        count = rnd(100);

        if (count < 25)
        {
          lptr->count = 4; //small
          count = 4;
        }
        else
        {
          lptr->count = 2; //large
          count = 2;
        }

        for (idx3 = 0; idx3 < count; idx3 ++)
        {
          lptr->rate   = rnd(6)+1;
          lptr->frames = 0;
          lptr->count  = count;
          lptr->x = rnd(MAX_COL/2 - 4) + (idx2 * (MAX_COL/2));
          lptr->y = rnd(MAX_ROW - 3);

          if (lptr->y > (MAX_ROW/2))
            lptr->state = 2;
          else
            lptr->state = 1;

          lptr ++;
        }
      }
    }

    lptr = (lavadata *)&work_buff[MAX_LEDS>>1];
  }

  disp_frames ++;

  //clear the display each frame
  clear_display();
  build_color_table(left_val, right_val);

  for (idx = 0; idx < color_channels; idx ++)
  {
    for (idx2 = 0; idx2 < 2; idx2 ++) //left/right
    {
      count = lptr->count;

      for (idx3 = 0; idx3 < count; idx3 ++)
      {
        draw_blob(led, lava_order[idx], idx3, idx2, lptr);

        lptr ++;
      }
    }
  }
}
#endif


#define HIGH_BIAS     0x20
#define HIGH_PASS     0x10
#define ADC_CPU_NOISE 0.7
#define ADC_CLA_NOISE 0.1
#define REDUCE_NOISE

#pragma CODE_SECTION(color_organ_prep, "ramCode")


//This function prepares the FFT output bins for the display routines.
#ifndef FLOODS
void color_organ_prep(float *fft_bin, uint16_t *chan_val)
#else
void color_organ_prep(uint16_t  side,
                         float *fft_bin,
                      uint16_t *chan_val,
                      uint16_t *top_chan,
                      uint16_t *hyst_sum)
#endif
{
  uint16_t idx;
  uint16_t idx2;
  uint16_t idx3;
  uint16_t t, end;
  float    temp, max;

  #ifdef BAND_SCALE
  float    maxlo, maxmid, maxhi;
  maxhi = 0;
  maxmid= 0;
  maxlo = 0;
  #endif

  uint16_t prev_cv[MAX_CHANNELS];

  idx3 = 1;

  for (idx = 0; idx < color_channels; idx++)
  {
    //Save off the last frame's value
    prev_cv[idx] = chan_val[idx];

    max = 0;
    end = channel_freq[idx+1]; //this depends on one extra entry in the array

    for (idx2 = channel_freq[idx]; idx2 < end; idx2 ++)
    {
      #ifdef REDUCE_NOISE
        #ifdef USE_CLA
        temp = fft_bin[idx3] - ADC_CLA_NOISE;
        #else
        temp = fft_bin[idx3] - ADC_CPU_NOISE;
        #endif
      #else
      temp = fft_bin[idx3];
      #endif

      //Square the data to make more difference between large and small values
      temp = temp * temp * audio_gain;

      if (max < temp)
        max = temp;
      idx3 += 1;
    }

    chan_val[idx] = (uint16_t) max;

    #ifdef BAND_SCALE
    if (idx < bass_chan) //low freq channels
    {
      if (maxlo < max)
        maxlo = max;
    }
    else
    if (idx < midr_chan) //mid freq channels
    {
      if (maxmid < max)
        maxmid = max;
    }
    else //high freq channels
    {
      if (maxhi < max)
        maxhi = max;
    }

    #else //scale each channel independently

    if (max > (float)MAX_RGB_VAL)
    {
      temp = max / (float)MAX_RGB_VAL;
      chan_val[idx] /= temp;
    }
    #endif

    //Don't pass very small values through for some channels.
    if (((idx == 0) || (idx == color_channels-1)) && (chan_val[idx] < HIGH_PASS))
      chan_val[idx] = 0;
  }

  //Since the highest channel registers lower with the same signal strength
  //give it a boost if it's not zero.
  if (chan_val[color_channels-1] > 0)
    chan_val[color_channels-1] += HIGH_BIAS;

  //Give half as much boost to the next-to-highest channel.
  if (chan_val[color_channels-2] > 0)
    chan_val[color_channels-2] += (HIGH_BIAS >> 1);

#ifdef BAND_SCALE
  //Scale the data to the maximum component RGB value.
  if (maxlo > (float)MAX_RGB_VAL)
  {
    temp = maxlo / (float)MAX_RGB_VAL;

    for (idx = 0; idx < bass_chan; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxmid > (float)MAX_RGB_VAL)
  {
    temp = maxmid / (float)MAX_RGB_VAL;

    for (idx = bass_chan; idx < midr_chan; idx++)
    {
      chan_val[idx] /= temp;
    }
  }

  if (maxhi > (float)MAX_RGB_VAL)
  {
    temp = maxhi / (float)MAX_RGB_VAL;

    for (idx = midr_chan; idx < color_channels; idx++)
    {
      chan_val[idx] /= temp;
    }
  }
#endif

  //Incandescent bulbs cannot flash at 15 or 30Hz. Apply a fading factor to
  //slow the LEDs down a bit. We don't want to put people into a trance... ;^)
  for (idx = 0; idx < color_channels; idx++)
  {
    t = 0;
    idx3 = (prev_cv[idx] >> 2); //%25 (fastest)
    for (idx2 = 1; idx2 < led_response; idx2++)
      t += idx3; //25%, 50% or 75%

    //If the new value is less than x%, set it to x% instead.
    if (chan_val[idx] < t)
      chan_val[idx] = t;
  }

  #ifdef FLOODS
  //This function assumes left side is called first.
  update_hyst(side, color_channels);
  #endif
}


#pragma CODE_SECTION(draw_char, "ramCode")

//Draw a character starting at lower-left x,y, and clip to the array.
void draw_char(LED_MAIN *led, int16_t x, int16_t y, int16_t idx, uint32_t color)
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
  int16_t  idx, idx2, idx3;
  uint16_t s_len, p_len;
  uint16_t word_idx;
  uint16_t curr_len;
  uint16_t curr_char;
  uint32_t disp_clr;
  uint16_t str_idx[FONT_MAX_CHAR];
  uint16_t char_wid[FONT_MAX_CHAR];
  uint16_t word_clr[5];
  LED_MAIN *led;

//  if (ppong_fill == 0)
    led = &led_ping[0];
//  else
//    led = &led_pong[0];

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

    if (color == 0xffffffff)
      disp_clr = rainbow[rnd(12)];

    if (dispmode == 0) //center in one panel
    {
      x = panel_off[panel][1] + (PANEL_COL/2) - (p_len/2);
      y = panel_off[panel][0] + (PANEL_ROW/2) - (FONT_HEIGHT/2);
      if (x < 0) x = 0;

      for (idx = 0; idx < s_len; idx++)
      {
        draw_char(led, x, y, str_idx[idx], disp_clr);
        x += fontchar[str_idx[idx]].wid + 1;
      }
    }
    else if (dispmode == 1) //center in total array, but don't split a character
    {
      idx2 = s_len / 2;
      idx3 = idx2-1;
      if (str[idx3] == ' ') idx3 --; //ignore a space

      y = (MAX_ROW / 2) - (FONT_HEIGHT / 2);
      x = PANEL_COL - (fontchar[str_idx[idx3]].wid + 1);

      for (idx = idx3; idx >= 0; idx--) //draw left panel
      {
        draw_char(led, x, y, str_idx[idx], disp_clr);
        x -= (fontchar[str_idx[idx-1]].wid + 1);
      }

      x = PANEL_COL;
      idx3 = idx2;
      if (str[idx3] == ' ') idx3 ++; //ignore a space

      for (idx = idx3; idx < s_len; idx++) //draw right panel
      {
        draw_char(led, x, y, str_idx[idx], disp_clr);
        x += (fontchar[str_idx[idx]].wid + 1);
      }
    }

    for (idx3 = 0; idx3 < delay; idx3 ++)
    {
      wait_for_sync(1);

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
        draw_char(led, idx3, y, str_idx[idx2], 0x000000); //black out the previous write

        if ((color == 0xffffffff) && (str_idx[idx] == FONT_SPACE_IDX))
          disp_clr = rainbow[word_clr[word_idx++]];
        draw_char(led, idx3-1, y, str_idx[idx2], disp_clr);
      }

      wait_for_sync(1);

      if (end_of_gap)
        return;
    }

    wait_for_sync(delay);
  }
}


#if 0
#pragma CODE_SECTION(test_rgb, "ramCode")
//test a string of LEDs
void test_rgb(void)
{
  uint16_t idx;

  while(1)
  {
    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0xc0;
      led[idx].g = 0;
      led[idx].b = 0;
    }
    wait_for_sync(30);


    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0xc0;
      led[idx].b = 0;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0;
      led[idx].b = 0xc0;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0x60;
      led[idx].g = 0x60;
      led[idx].b = 0;
    }
    wait_for_sync(30);


    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0;
      led[idx].g = 0x60;
      led[idx].b = 0x60;
    }
    wait_for_sync(30);

    for (idx = 0; idx < 150; idx ++)
    {
      led[idx].r = 0x60;
      led[idx].g = 0;
      led[idx].b = 0x60;
    }
    wait_for_sync(30);
  }
}
#endif


//This function performs power on (or reset) displays to verify that the LED
//panels are correctly operating prior to the color organ running.
#define INIT_DISPLAY_FRAMES               (FRAMES_PER_SEC + (FRAMES_PER_SEC >> 1))
#pragma CODE_SECTION(initial_display, "ramCode")

void initial_display(void)
{
  char str[2];

  clear_display();

  //Display the panel number in each panel to identify correct hookup.
  str[0] = '1'; str[1] = 0;
  show_text(0, 0, 0x0000a0, 3, str);
  str[0] = '2';
  show_text(0, 1, 0x0000a0, 3, str);
  str[0] = '3';
  show_text(0, 2, 0x0000a0, 3, str);
  str[0] = '4';
  show_text(0, 3, 0x0000a0, INIT_DISPLAY_FRAMES, str);
  clear_display();

  #ifndef SLAVE
  show_text(1, 0, 0x00f000, INIT_DISPLAY_FRAMES, "Let's Go!");
  #endif

  //Set the end of gap flag so that the first frame will be a peak.
  end_of_gap = 1;
}


#define MIN_DISPLAY_FRAMES                (FRAMES_PER_SEC * 16)
#pragma CODE_SECTION(do_display, "ramCode")

//This is the main display driver for the color organ.
uint16_t prev_display;

void do_display(uint16_t peak_flag, uint16_t *chan_left, uint16_t *chan_right)
{
  uint16_t idx, dir;
  uint16_t disp_peak;
  uint16_t disp_flag;

  if (display == 9999) //force start-up to initialize a peak
  {
    peak_flag = 1;
  }

  disp_peak = 0;

  //Choose a new pattern to display - on a peak boundary
  #ifndef SLAVE
  if (peak_flag & (display_frames > MIN_DISPLAY_FRAMES))
  {
    disp_peak = 1;
    display_frames = 0;  //for this function's use
    disp_frames = 99999; //for display-local use
    prev_display = display;

    dir = rnd(2);
    disp_flag = 1 << rnd(MAX_DISPLAYS);

    for (idx = 0; idx < MAX_DISPLAYS; idx ++)
    {
      if ((disp_flag & avail_disp) != 0) //got a match
      {
        display = disp_flag;
        break;
      }

      if (dir == 0)
      {
        disp_flag <<= 1;
        if (disp_flag > MAX_DISP_FLAG) //wrap the flag
          disp_flag = 1;
      }
      else
      {
        disp_flag >>= 1;
        if (disp_flag == 0) //wrap the flag
          disp_flag = MAX_DISP_FLAG;
      }
    }


    #ifdef MASTER
      //If running as master, set a new mode value for the slave
      set_mode(display); //can only send values 0..7
    #endif
  }
  #endif //not a slave

  #ifdef SLAVE
  //If running as slave, grab the master's current mode every frame
  prev_display = display;
  get_mode(&display);

  if (display != prev_display)
  {
    peak_flag = 1;
    disp_frames = 99999;
  }
  #endif

  display_frames ++;

  switch (display)
  {
    case 0x01:
      line_segment2(disp_peak, chan_left, chan_right);
      break;

    case 0x02:
      vertical(disp_peak, chan_left, chan_right);
      break;

    case 0x04:
      ripple(disp_peak, chan_left, chan_right);
      break;

    case 0x08:
      equalizer_hires(peak_flag, chan_left, chan_right);
      break;

    case 0x10:
//      lavalamp(disp_peak, chan_left, chan_right);
      equalizer(peak_flag, chan_left, chan_right);
      break;

    case 0x20:
      two_by_two(disp_peak, chan_left, chan_right, 0); //arrow
      break;

    case 0x40:
      two_by_two(disp_peak, chan_left, chan_right, 1); //gradient
      break;

    case 0x80:
      two_by_two(disp_peak, chan_left, chan_right, 2); //random
      break;
  };

  #ifdef FLOODS
  floods(chan_left, chan_right);
  #endif
}
