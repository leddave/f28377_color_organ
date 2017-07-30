/*
  This file contains beat detection algorithms.
*/
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "beat.h"

#define GAP_LENGTH                     (FRAMES_PER_SEC * 3)
#define GAP_LENGTH_LONG                (FRAMES_PER_SEC * 8) /* a gap longer than nornal inter-song time */
#define MAX_BPM                        200 /* beats per minute */
#define PEAK_SPACING                  ((FRAMES_PER_SEC * 60) / MAX_BPM)

//Locally defined globals:
volatile uint32_t gap_frames;
volatile uint16_t gap_found;
volatile uint16_t gap_long_found;
volatile uint16_t end_of_gap;
volatile uint16_t peak_flag;
uint16_t ramp_frames;
uint32_t peak_last;
int32_t  peak_threshold;
uint16_t power_avg_idx;
int32_t  power_avg;
int32_t  power_avg_buff[PEAK_FRAMES];


extern uint16_t chan_max_left[COLOR_CHANNELS];
extern uint16_t chan_max_right[COLOR_CHANNELS];


void beat_init(void)
{
  gap_frames = 0;
  gap_found  = 0;
  end_of_gap = 0;
  peak_flag  = 0;
  peak_last  = 0;
  power_avg  = 0;
  ramp_frames= 0;
  peak_threshold = 0;
  gap_long_found = 0;
  power_avg_idx = PEAK_FRAMES-1;
  memset(power_avg_buff, 0, sizeof(power_avg_buff));
}


//These algorithms will use the peaks of the FFTs that run each frame.
void beat_detect(void)
{

}


//This function will add the current channel data to the running average, then
//determine if this frame is a peak, meaning that the power (magnitude) of this
//frame is at least x% above the average.
void update_average_power(void)
{
  uint16_t idx;
  uint32_t sum = 0;

  peak_flag = 0;

  if (gap_found) //Can't be a peak if we're in silence
  {
    power_avg = 0;
    peak_threshold = 0;
  }
  else //normal frames with music playing
  {
    if (end_of_gap) //This is the first frame following a gap
    {
      peak_flag = 1;
      peak_last = PEAK_SPACING;  //allow a peak to be flagged initially
      ramp_frames = 0;
      power_avg_idx = PEAK_FRAMES-1;
    }

    //This is used to count frames following a gap
    if (ramp_frames < PEAK_FRAMES)
      ramp_frames ++;

    //Combine all L and R channels except the highest one to create a power value
    //for this frame.
    for (idx = 0; idx < COLOR_CHANNELS-1; idx++)
    {
      sum += chan_max_left[idx] + chan_max_right[idx];
    }

    //Update the index prior to adding. This way, if the buffer is needed by another
    //function, the index will be pointing to the latest entry filled. The index is
    //maintained to use power_avg_buff[] as a ring (circular) buffer.
    power_avg_idx ++;
    if (power_avg_idx == PEAK_FRAMES)
      power_avg_idx = 0;

    power_avg_buff[power_avg_idx] = sum;
    power_avg = 0;

    for (idx = 0; idx < ramp_frames; idx++)
    {
      power_avg += power_avg_buff[idx];
    }

    //Find the average and 37% threshold
    power_avg /= ramp_frames;
    peak_threshold = power_avg + (power_avg >> 2) + (power_avg >> 3);

    //See if this frame is a peak
    if (sum >= peak_threshold)
    {
      //The call to rnd() is a dummy call to better randomize the seed.
      peak_flag = 1 + rnd(1);
    }

    //If not enough frames have passed since the last peak, kill it.
    if (peak_flag)
    {
      if (peak_last < PEAK_SPACING)
        peak_flag = 0;
      else
        peak_last = 0;
    }

    peak_last ++;
  }
}


//This function sets a flag when a sequence of silent frames is found. A second
//flag is then set to mark the end of the gap.
void gap_detect(void)
{
  uint16_t idx;
  uint16_t all_zero = 1;

  end_of_gap = 0;

  //Look for all zeros for this frame:
  for (idx = 0; idx < COLOR_CHANNELS; idx ++)
  {
    if ((chan_max_left[idx] > 0) || (chan_max_right[idx] > 0))
    {
      all_zero = 0;
      break;
    }
  }

  if (all_zero)
    gap_frames ++;
  else
  {
    gap_frames = 0; //reset the frame count

    if (gap_found)
    {
      gap_found = 0;
      end_of_gap = 1;
      gap_long_found = 0;
    }
  }

  if (gap_frames > GAP_LENGTH)
  {
    //Once the gap is found the flag is not cleared until the gap ends.
    gap_found = 1;
  }

  if (gap_frames > GAP_LENGTH_LONG)
  {
    //Once the gap is found the flag is not cleared until the gap ends.
    gap_long_found = 1;
  }
}


//This function performs analysis of the processed FFT data for beat detection
//and display purposes.
void beat_detect_prep(void)
{
  gap_detect();
  update_average_power();
}
