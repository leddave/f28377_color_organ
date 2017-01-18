#include "font.h"


//These #defines point into fontchar[]
#define FONT_SPECIAL_IDX               10
#define FONT_UPPER_IDX                 22
#define FONT_LOWER_IDX                 48


//Not all characters are included.  The display routines will skip any
//character not found in this list.  The list is in groups of ASCII code.
//New characters must be inserted to preserve this ordering, and the offset
//#defines above adjusted so they still point to the correct entries.

const font_t fontchar[NUM_FONT_CHAR] =
{
  { 48, 5, 0x7c82, 0x9282, 0x7c00}, //'0'  Numbers
  { 49, 5, 0x0242, 0xfe02, 0x0200}, //'1'
  { 50, 5, 0x468a, 0x9292, 0x6200}, //'2'
  { 51, 5, 0x4482, 0x9292, 0x6c00}, //'3'
  { 52, 5, 0x3050, 0x90fe, 0x1000}, //'4'
  { 53, 5, 0xe492, 0x9292, 0x0c00}, //'5'
  { 54, 5, 0x7c92, 0x9292, 0x0c00}, //'6'
  { 55, 5, 0x808e, 0x90a0, 0xc000}, //'7'
  { 56, 5, 0x6c92, 0x9292, 0x6c00}, //'8'
  { 57, 5, 0x6092, 0x9292, 0x7c00}, //'9'

  { 32, 1, 0x0000, 0x0000, 0x0000}, //' '  Special chars
  { 33, 1, 0xfa00, 0x0000, 0x0000}, //'!'
  { 35, 5, 0x28fe, 0x28fe, 0x2800}, //'#'
  { 40, 2, 0x7c82, 0x0000, 0x0000}, //'('
  { 41, 2, 0x82c7, 0x0000, 0x0000}, //')'
  { 44, 2, 0x0506, 0x0000, 0x0000}, //','
  { 46, 2, 0x0606, 0x0000, 0x0000}, //'.'
  { 47, 3, 0x0638, 0xc000, 0x0000}, //'/'
  { 58, 4, 0x3636, 0x0000, 0x0000}, //':'
  { 59, 4, 0x3536, 0x0000, 0x0000}, //';'
  { 61, 4, 0x2828, 0x2828, 0x0000}, //'='
  { 63, 5, 0x4080, 0x8a90, 0x6000}, //'?'

  { 65, 6, 0x7fff, 0x9090, 0x907e}, //'A'  Uppercase
  { 66, 6, 0xfefe, 0x9292, 0x926c}, //'B'
  { 67, 6, 0x7cfe, 0x8282, 0x8244}, //'C'
  { 68, 6, 0xfefe, 0x8282, 0x827c}, //'D'
  { 69, 5, 0xfefe, 0x9292, 0x9200}, //'E'
  { 70, 5, 0xfefe, 0x9090, 0x9000}, //'F'
  { 71, 6, 0x7cfe, 0x8292, 0x924e}, //'G'
  { 72, 6, 0xfefe, 0x1010, 0x10fe}, //'H'
  { 73, 5, 0x8282, 0xfe82, 0x8200}, //'I'
  { 74, 6, 0x1c1e, 0x0202, 0x02fc}, //'J'
  { 75, 6, 0xfefe, 0x1028, 0x4482}, //'K'
  { 76, 5, 0xfefe, 0x0202, 0x0200}, //'L'
  { 77, 6, 0xfefe, 0x4030, 0x40fe}, //'M'
  { 78, 6, 0xfefe, 0x4020, 0x10fe}, //'N'
  { 79, 6, 0x7cfe, 0x8282, 0x827c}, //'O'
  { 80, 6, 0x7efe, 0x9090, 0x9060}, //'P'
  { 81, 6, 0x7cfe, 0x828a, 0x847a}, //'Q'
  { 82, 6, 0xfefe, 0x9088, 0x9462}, //'R'
  { 83, 6, 0x64f6, 0x9292, 0x924c}, //'S'
  { 84, 5, 0x8080, 0xfe80, 0x8000}, //'T'
  { 85, 6, 0xfcfe, 0x0202, 0x02fc}, //'U'
  { 86, 6, 0xf8fc, 0x0602, 0x04f8}, //'V'
  { 87, 6, 0xfefe, 0x0418, 0x04fe}, //'W'
  { 88, 6, 0xc6ee, 0x2810, 0x28c6}, //'X'
  { 89, 6, 0xc0e0, 0x3e1e, 0x20c0}, //'Y'
  { 90, 5, 0x868a, 0x92a2, 0xc200}, //'Z'

  { 97, 6, 0x0c2a, 0x2a2a, 0x1c02}, //'a'  Lowercase
  { 98, 5, 0xfe22, 0x2222, 0x1c00}, //'b'
  { 99, 4, 0x1c22, 0x2222, 0x0000}, //'c'
  {100, 5, 0x1c22, 0x2222, 0xfe00}, //'d'
  {101, 5, 0x1c2a, 0x2a2a, 0x1800}, //'e'
  {102, 4, 0x107e, 0x9040, 0x0000}, //'f'
  {103, 5, 0x1825, 0x2525, 0x1e00}, //'g'
  {104, 5, 0xfe10, 0x1010, 0x0e00}, //'h'
  {105, 3, 0x22be, 0x0200, 0x0000}, //'i'
  {106, 5, 0x0201, 0x0121, 0xbe00}, //'j'
  {107, 4, 0xfe08, 0x1422, 0x0000}, //'k'
  {108, 3, 0x82fe, 0x0200, 0x0000}, //'l'
  {109, 5, 0x3e20, 0x1e20, 0x1e00}, //'m'
  {110, 4, 0x3e20, 0x201e, 0x0000}, //'n'
  {111, 5, 0x1c22, 0x2222, 0x1c00}, //'o'
  {112, 5, 0x3f24, 0x2424, 0x1800}, //'p'
  {113, 5, 0x1c24, 0x2425, 0x1f00}, //'q'
  {114, 5, 0x1e20, 0x2020, 0x1000}, //'r'
  {115, 5, 0x102a, 0x2a2a, 0x0400}, //'s'
  {116, 4, 0x20fc, 0x2202, 0x0000}, //'t'
  {117, 5, 0x3c02, 0x0202, 0x3c00}, //'u'
  {118, 5, 0x3804, 0x0204, 0x3800}, //'v'
  {119, 5, 0x3c02, 0x3c02, 0x3c00}, //'w'
  {120, 5, 0x2214, 0x0814, 0x2200}, //'x'
  {121, 5, 0x3805, 0x0505, 0x3e00}, //'y'
  {122, 5, 0x2226, 0x2a32, 0x2200}  //'z'
};


//Return the number of chars until the null termination.
uint16_t font_str_len(char *str)
{
  uint16_t idx;

  for (idx = 0; idx < FONT_MAX_CHAR; idx ++)
  {
    if (str[idx] == 0)
      return (idx);
  }

  return (0);
}


//Return the number of pixels until the null termination.
//Also returns an array of indexes into fontchar[] for the string.
uint16_t font_pix_len(char *str, uint16_t *str_idx)
{
  uint16_t idx, idx2;
  uint16_t sum = 0;

  for (idx = 0; idx < FONT_MAX_CHAR; idx ++)
  {
    if (str[idx] == 0)
      break;

    idx2 = font_get_idx(str[idx]);
    sum += fontchar[idx2].wid + 1;
    str_idx[idx] = idx2;
  }

  return (sum);
}


//This function searches for a character in the font array, and returns the index
//into fontchar[].  If not found, it returns the index to the space (' ') character.
uint16_t font_get_idx(uint16_t code)
{
  uint16_t idx;

  if (code >= 65) //this is an alphabetic character
  {
    if ((code >= 97) && (code <= 122)) //this is lowercase
    {
      return (FONT_LOWER_IDX + (code - 97));
    }
    else

    if (code <= 90) //this is uppercase
    {
      return (FONT_UPPER_IDX + (code - 65));
    }
  }

  else //search for numbers and special chars
  {
    if ((code >= 48) && (code <= 57)) //this is numeric
    {
      return (code - 48);
    }
    else //search for special char
    {
      for (idx = FONT_SPECIAL_IDX; idx < FONT_UPPER_IDX; idx ++)
      {
        if (code == fontchar[idx].asc)
          return (idx);
      }
    }
  }

  return (FONT_SPECIAL_IDX); //index to ' ' char
}
