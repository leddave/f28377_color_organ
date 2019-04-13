/*****************************************************************************
 * This is the rgb_28377 project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "uart.h"
#include "msg.h"


//Global variables
extern uint16_t  peak_flag;
extern uint16_t  color_channels;
extern uint16_t  last_chan_cnt;
extern uint16_t  led_response;
extern uint16_t  flood_response;
extern uint16_t  avail_disp;
extern float     audio_gain;
extern uint32_t  channel_color[MAX_CHANNELS]; //current color for each channel
extern uint16_t  channel_freq[MAX_CHANNELS+1]; //current freq bin for each channel

extern Msg_Default_Values defaults;
extern Msg_Available_Capabilities capabilities;

extern void reload_channel_data(uint16_t chan);

//Read a message and return the message ID and the data into a user buffer.
char msg_recv(char *ptr)
{
  uint16_t    idx;
  char        valid;
  Msg_Header *hdr  = (Msg_Header *)ptr;
  char       *data = (char *)&hdr[1];  //position the body after the header

  valid = uart_recv(&hdr->cmd); //read a message command or get zero

  if ((valid) &&
      (hdr->cmd >= MSG_FIRST_CMD_CODE) &&
      (hdr->cmd <= MSG_LAST_CMD_CODE))
  {
    uart_recv(&hdr->len); //read the message length
    idx = 0;

    for (idx = 0; idx < hdr->len; idx ++)
    {
      uart_recv(&data[idx]); //read the message body
    }

    return (hdr->cmd);
  }
  else //not a valid color organ command
  {
    return (0);
  }
}


void msg_send_header(Msg_Header *hdr)
{

  uart_send(hdr->cmd);
  uart_send(hdr->len);
}


void msg_send_data(char *data, char len)
{
  uint16_t    idx;

  for (idx = 0; idx < len; idx ++)
  {
    uart_send(data[idx]);
  }
}


//Send a message to UART-B.
void msg_send(char *ptr)
{
  Msg_Header *hdr  = (Msg_Header *)ptr;
  char       *data = (char *)&hdr[1];  //position the body after the header

  msg_send_header(hdr);
  msg_send_data(data, hdr->len);
}


//Handle a received message.
void msg_process(char cmd, char *ptr)
{
  Msg_Response resp_msg;

  resp_msg.result = MSG_OKAY;

  switch (cmd)
  {
    case MSG_GET_CAPABILITIES:
    {
      Msg_Header hdr;
      uint16_t remdisp = MAX_DISPLAYS - capabilities.num_displays;

      hdr.cmd = MSG_AVAILABLE_CAPABILITIES;
      hdr.len = sizeof(Msg_Available_Capabilities) -
                sizeof(Msg_Header) -
               (sizeof(Msg_Display) * remdisp);

      msg_send_header(&hdr);
      msg_send_data(&capabilities.min_chan, hdr.len);
      return; //don't send standard response
    }
    case MSG_GET_DEFAULTS:
    {
      Msg_Header hdr;
      uint16_t remchan = MAX_CHANNELS - defaults.chan;

      hdr.cmd = MSG_DEFAULT_VALUES;
      hdr.len = sizeof(Msg_Default_Values) -
                sizeof(Msg_Header) -
               (sizeof(Msg_Channel_Data) * remchan);

      msg_send_header(&hdr);
      msg_send_data(&defaults.chan, hdr.len);
      return; //don't send standard response
    }
    case MSG_SET_CHANNEL_COUNT:
    {
      Msg_Set_Channel_Count *chan_cnt = (Msg_Set_Channel_Count *)ptr;
      if ((chan_cnt->count >= MIN_CHANNELS) &&
          (chan_cnt->count <= MAX_CHANNELS))
      {
        last_chan_cnt  = color_channels;
        color_channels = chan_cnt->count;

        reload_channel_data(color_channels);
        peak_flag = 1;
      }
      else
        resp_msg.result = MSG_INVALID_DATA;
      break;
    }
    case MSG_SET_CHANNEL_COLOR:
    {
      Msg_Set_Channel_Color *chan_clr = (Msg_Set_Channel_Color *)ptr;
      uint16_t chan = chan_clr->chan;

      if (chan < color_channels)
      {
        channel_color[chan] = ((uint32_t)(chan_clr->r) << 16) +
                              ((uint32_t)(chan_clr->g) << 8) +
                              ((uint32_t)(chan_clr->b));
        peak_flag = 1;
      }
      else
        resp_msg.result = MSG_INVALID_CHANNEL;
      break;
    }
    case MSG_SET_CHANNEL_FREQ:
    {
      Msg_Set_Channel_Freq *chan_freq = (Msg_Set_Channel_Freq *)ptr;
      uint16_t chan = chan_freq->chan;

      if (chan < color_channels)
      {
        channel_freq[chan]   = chan_freq->start_bin;
        channel_freq[chan+1] = chan_freq->end_bin+1;
        peak_flag = 1;
      }
      else
        resp_msg.result = MSG_INVALID_CHANNEL;
      break;
    }
    case MSG_SET_AVAILABLE_DISPLAYS:
    {
      uint16_t temp;
      Msg_Set_Available_Displays *av_disp = (Msg_Set_Available_Displays *)ptr;
      temp  = av_disp->mask_lo;
      temp |= (uint16_t)(av_disp->mask_hi) << 8;

      if (temp != 0) //ignore if no displays are selected
      {
        avail_disp = temp;
        defaults.disp_mask_hi = av_disp->mask_hi;
        defaults.disp_mask_lo = av_disp->mask_lo;
      }
      break;
    }
    case MSG_SET_AUDIO_GAIN:
    {
      signed char gain;
      Msg_Set_Audio_Gain *aud_gain = (Msg_Set_Audio_Gain *)ptr;
      gain = aud_gain->gain;
      if ((gain >= 0xfe) && (gain <= 0x02))
      {
        audio_gain = 1.0 + ((float)(gain) * 0.25);
      }
      break;
    }
    case MSG_SET_LED_RESPONSE:
    {
      Msg_Led_Response *led_resp = (Msg_Led_Response *)ptr;
      if ((led_resp->led > 0) && (led_resp->led < 4))
        led_response = led_resp->led;
      if ((led_resp->flood > 0) && (led_resp->flood < 4))
        flood_response = led_resp->flood;
      peak_flag = 1;
      break;
    }
    default:
      resp_msg.result = MSG_INVALID_MSG;
      break;
  }

  resp_msg.hdr.cmd = MSG_RESPONSE;
  resp_msg.hdr.len = 1;
  msg_send((char *)&resp_msg);
}
