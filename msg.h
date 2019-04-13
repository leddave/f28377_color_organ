/* This header defines messages for F2837x to Phone communication.
 *
 * All structures defined here use char only to avoid issues with endianess
 * with 16- or 32-bit data types being sent over the byte-wise UART transceiver.
 * Actually, no 32-bit types are needed, and the 16-bit types are split into
 * two chars with _hi as the MSB and _lo as the LSB.
 *
 * In normal use, the connected device should send the following two messages
 * prior to sending MSG_SET_xxx messages:
 *
 *  1st: MSG_GET_CAPABILITIES
 *  2nd: MSG_GET_DEFAULTS
 */
#ifndef MSG_H
#define MSG_H


#define MSG_MAX_TEXT_LEN               16
#define MSG_DISPLAY_TEXT_LEN           20
#define MSG_MAX_IN_MSG_LEN             32 /* in bytes, max uart read burst. */

/* Phone to F2837x Command codes: */

#define MSG_GET_CAPABILITIES           0x80
#define MSG_GET_DEFAULTS               0x81
#define MSG_SET_CHANNEL_COUNT          0x82
#define MSG_SET_CHANNEL_COLOR          0x83
#define MSG_SET_CHANNEL_FREQ           0x84
#define MSG_SET_AVAILABLE_DISPLAYS     0x85
#define MSG_SET_AUDIO_GAIN             0x86
#define MSG_SET_LED_RESPONSE           0x87
#define MSG_SET_TEXT                   0x88
#define MSG_CONTROL                    0x89 // run, pause, halt, save, etc.

/* F2837x to phone Command codes: */

#define MSG_DEFAULT_VALUES             0x90 // response to MSG_GET_DEFAULTS
#define MSG_AVAILABLE_CAPABILITIES     0x91 // response to MSG_GET_CAPABILITIES
#define MSG_RESPONSE                   0x92 // response to MSG_SET_xxx and MSG_CONTROL

/* Define the range of valid Command codes */
#define MSG_FIRST_CMD_CODE             MSG_GET_DEFAULTS
#define MSG_LAST_CMD_CODE              MSG_RESPONSE

/* Control codes */
#define MSG_MODE_RUN                   0x01 // normal run mode
#define MSG_MODE_PAUSE                 0x02 // freezes the current display
#define MSG_MODE_HALT                  0x03 // goes to gap mode (blank screen)
#define MSG_MODE_TEXT                  0x04 // displays a text message
#define MSG_MODE_STOP                  0x05 // blanks then stops LED bit-bangs
#define MSG_MODE_SAVE                  0x06 // writes the current configuration to flash
#define MSG_MODE_RESTORE               0x07 // reloads the built-in defaults. Does not write to flash

/* Msg result codes */
#define MSG_OKAY                       0x01 //
#define MSG_INVALID_MSG                0xff //
#define MSG_INVALID_CHANNEL            0xfe //
#define MSG_INVALID_DATA               0xfd //


typedef struct
{
  char cmd;                            //Command codes listed above
  char len;                            //Number of bytes in the message body.
                                       //Len never includes this header and can be zero.
} Msg_Header;

#define MSG_HEADER_SIZE                sizeof(Msg_Header)


typedef struct
{
  char mask_hi;                        //unique bit to ID each display - MSB
  char mask_lo;                        //LSB (low 8 bits)
  char name[MSG_DISPLAY_TEXT_LEN];     //null terminated display name

} Msg_Display;


typedef struct
{
  char chan;                           //channel number - data ignored if invalid
  char r;                              //red value (see capabilities for max)
  char g;                              //green value
  char b;                              //blue value
  char start_bin;                      //starting freq bin (see notes below)
  char end_bin;                        //ending   freq bin
} Msg_Channel_Data;


/***** Phone to F2837x Messages: *****/

typedef struct //MSG_GET_DEFAULTS
{
  Msg_Header hdr;
                                       //no message body needed (len = 0)
} Msg_Get_Defaults;


typedef struct //MSG_GET_CAPABILITIES
{
  Msg_Header hdr;
                                       //no message body needed (len = 0)
} Msg_Get_Capabilities;


/* The following "Set" messages will be applied when they are received.
 * So, if you change the channel count to 5, then send a command for
 * channel 6, that message will be ignored.
 */

typedef struct //MSG_SET_CHANNEL_COUNT
{
  Msg_Header hdr;
  char       count;          //number of desired channels (see capabilities)

} Msg_Set_Channel_Count;


typedef struct //MSG_SET_CHANNEL_COLOR
{
  Msg_Header hdr;
  char       chan;           //channel number - data ignored if invalid
  char       r;              //red value (see capabilities)
  char       g;              //green value
  char       b;              //blue value

} Msg_Set_Channel_Color;


/* Command to send channel freq. Frequency range is sent as bin numbers, relative
 * to FREQ_SPACING. For example, for three channels defined as follows with a
 * frequency spacing of 86Hz:  (86 = round(44100Hz / 2 / 256))
 *    channel 1:  0    to 250Hz
 *    channel 2:  250  to 2000Hz
 *    channel 3:  2000 to 15000Hz
 *
 * The bin numbers in the message would be:
 *    channel 1:  start: 0  end: 2   (  2 = 250Hz / 86 rounded, minus 1)
 *    channel 2:  start: 3  end: 22  ( 22 = 2000Hz / 86 rounded, minus 1)
 *    channel 3:  start: 23 end: 174 (174 = 15000Hz / 86 rounded, minus 1)
 */
typedef struct //MSG_SET_CHANNEL_FREQ
{
  Msg_Header hdr;
  char       chan;           //channel number - data ignored if invalid
  char       start_bin;      //starting freq bin (see above)
  char       end_bin;        //ending   freq bin (see above)

} Msg_Set_Channel_Freq;


typedef struct //MSG_SET_AVAILABLE_DISPLAYS
{
  Msg_Header hdr;
  char       mask_hi;        //bit mask indicating enabled displays - MSB
  char       mask_lo;        //LSB (low 8 bits)
                             //Msg_Default_Values defines a mask for each display
} Msg_Set_Available_Displays;


typedef struct //MSG_SET_AUDIO_GAIN
{
  Msg_Header hdr;
  char       gain;           // 0xfe = -50%, 0xff = -25%, 0=0%, 1 = +25%, 2 = +50%

} Msg_Set_Audio_Gain;


typedef struct //MSG_SET_LED_RESPONSE
{
  Msg_Header hdr;
  char       led;            // How fast the LEDs are allowed to flicker
                             // 1=slow, 2=normal, 3=fast
  char       flood;          // How fast the Floods can change colors
                             // 1=slow, 2=normal, 3=fast
} Msg_Led_Response;


typedef struct //MSG_SET_TEXT
{
  Msg_Header hdr;
  char       secs;           // number of seconds to display the text
  char       text[MSG_MAX_TEXT_LEN]; //null terminated text message

} Msg_Set_Text;


typedef struct //MSG_CONTROL
{
  Msg_Header hdr;
  char       mode;           // See MSG_MODE_xxx defines above
  char       fps;            // frames per second (see capabilities)

} Msg_Control;


/***** F2837x to Phone Messages: *****/

typedef struct //MSG_RESPONSE
{
  Msg_Header hdr;
  char       result;         //success or failure codes defined above

} Msg_Response;


typedef struct //MSG_DEFAULT_VALUES
{
  Msg_Header hdr;
  char       chan;           // number of channels
  char       fps;            // frames per second
  char       disp_mask_hi;   // default bitmap of displays that are enabled
  char       disp_mask_lo;   // LSB
  Msg_Channel_Data data[MAX_CHANNELS]; //number sent is in .chan field

} Msg_Default_Values;


typedef struct //MSG_AVAILABLE_CAPABILITIES
{
  Msg_Header hdr;
  char       panel_rows;     // LED array size in rows (each left/right)
  char       panel_cols;     // LED array size in colums (each left/right)
  char       flood_rows;     // LED flood size in rows (each left/right)
  char       flood_cols;     // LED flood size in colums (each left/right)
  char       min_chan;       // minimum allowable number of channels
  char       max_chan;       // maximum allowable number of channels
  char       freq_res;       // resolution or spacing (in Hz) of frequency bins
  char       min_frame_rate; // min frames per second (update rate)
  char       max_frame_rate; // max frames per second (update rate)
  char       max_r;          // max red component value
  char       max_g;          // max green component value
  char       max_b;          // max blue component value
  char       max_freq_hi;    // max audio freq used (< Nyquist Frequency) - MSB
  char       max_freq_lo;    // LSB
  char       num_displays;   // number of available displays in Msg_Display
  Msg_Display disp[MAX_DISPLAYS]; //number sent is in .displays field
} Msg_Available_Capabilities;


/* F2837x message handlers: */

//Read a message and return the message ID and the data into a user buffer.
char msg_recv(char *ptr);

//Send a message to UART-B.
void msg_send(char *ptr);

//Handle a received message.
void msg_process(char cmd, char *ptr);

#endif
