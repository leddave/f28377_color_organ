************************************************************************
* Program: 20 Channel Stereo Color Organ
*
* Purpose: To transform normal stereo audio output into 10 channels of
*          dancing, pulsing colored lights (per audio channel).
*
* Supported LED strings:
*          WS2811 pixel strings
*          WS2812 flexible strips
*          (for more information, WS28xx datasheets are online)
*
* Supported Micro Controllers:
*          TI F28377 (TI LaunchPad) 200Mhz development board
*
* Required Tools:
*          CCSv6 or v7, C2000 compilers, ControlSuite
*
* Required Launchpad connections:
*   Audio In: (eg. headphone jack from iPhone, iPad, etc.)
*     J3-pin 27: Left channel positive
*     J3-pin 29: Right channel positive
*     Any GND:   Common (negative) from the audio device
*
*   LED Lights:
*     J4-pin 40: Data wire of WS2811 or WS2812 lights, Left chan, string 1
*     J4-pin 39: Data wire of WS2811 or WS2812 lights, Left chan, string 2
*     J4-pin 38: Data wire of WS2811 or WS2812 lights, Right chan, string 1
*     J4-pin 37: Data wire of WS2811 or WS2812 lights, Right chan, string 2
*     J4-pin 34: Data wire of WS2812 (only) lights, Left chan flood
*     J4-pin 33: Data wire of WS2812 (only) lights, Right chan flood
*     Any GND:   Ground wires of all LED strings
*
*   Multi-color organ control: (to run two color organs together)
*     J4-pin 36: Display mode. Requires compiling with either MASTER or SLAVE
*     J4-pin 35: Display mode. Requires compiling with either MASTER or SLAVE
*                These GPIOs are set in output mode for MASTER, input mode
*                for SLAVE.
*
*   Note for multi-color organ use: High frequency artifacts can appear in the
*   highest channels of either color organ if they are not properly connected
*   together.  To avoid this, you need to do one or more of the following:
*     1) Ground (-5V) all F28377 EVMs and 5V power supplies together.
*     2) Run all F28377 EVMs from the same 5V power supply.
*     3) Use some type of amplifier (such as a multi-channel headphone amp)
*        to isolate the audio signal for each color organ (instead of using
*        simple "Y" splitters).
*
*   5V Power Supply:
*     Any GND:   Ground (or -5V) of power supply
*                (+5V connects to the +5V of the LEDs)
*
*   Launchpad Power:
*     Currently, power must be supplied via the USB jack.
*
************************************************************************

Important parts of the code:

1) globals.h:
   This file defines all the important parameters for the program, such as
   sampling rate, frame rate, etc.

2) main.c:
   This file contains much of the initialization and control code, and the
   main processing loop. The processing that happens each frame is:
     a) Start the ADC timer.
     b) Collect samples for Left and Right channel inputs.
     c) Stop the ADC timer.
     d) Perform an FFT on each L/R channel, creating frequency bins.
     e) Scan the frequency bins, finding the max value for each channel.
     f) Call the display driver to transform channel values into pixel values.
     g) Call the LED driver to output the pixel values to the LED strings.

3) my_adc.c:
   These routines setup and control ADC_A, channels 0 and 1 to capture Left and
   Right audio into digital 12-bit samples at the rate of 44KHz.

4) rfft.c:
   This file contains functions to call the optimized floating point FFT library
   that converts samples into frequency space bins.

5) display.c:
   This file contains routines for deciding how to display the frequency data (FFT
   output) in the defined array(s) of LED lights. It first converts the frequency
   bin data into channel values (one value per channel per L/R), and then chooses
   which LEDs get a channel's color and brightness.

6) led_driver.c:
   This file contains the code that bit bangs the display data out to the LEDs.
   Selection of WS2811 vs WS2812 is done via CCS project properties. The bit bang
   code is optimized to output 4 strings simultaneously. This application is
   setup to use strings 1 and 2 for the Left channel, and strings 3 and 4 for the
   Right.  You can use fewer strings and change this configuration, but it is
   not recommended to modify this function. This means your led[] struct will
   need to contain memory for 4 equal length strings. If you want to use 200 LEDs
   per L/R, it is more efficient to define 4 strings of 100 rather than 2 of 200,
   since 4 strings can be bit banged in half the time of 2 strings of double the
   length.  These strings are 1D physical arrays. You will need mapping code to
   define a logical arrangement (e.g rows and columns) for the display routines
   to use, then map into the physical array for output.


Building Notes:

The following #defines are optionally defined in the CCS project properties:
  1) WS2811 - needed if the 4 primary LED strings are WS2811 type LEDs.
  2) WS2812 - needed if the 4 primary LED strings are WS2812 type LEDs.
  3) FLOODS - needed if using the two extra GPIOs listed above for color floods.
  4) LARGE_ARRAY - needed if compiling for the larger of the two array sizes
     defined in display.c.
  5) FLASH - needed when compiling to store the program in F28377s flash. Note,
     this requires changing which .cmd file is used as described below.

  When connecting two color organs together for syncronized operation, each
  will need one of the following defined: (not all displays may be supported)
  6) MASTER - will choose the display.
  7) SLAVE - will use the master's display choice.

To build for RAM (CCS fast turnaround debugging)

  1) Right click on f28377.cmd and make sure "Exclude from Build" is NOT
     checked.
  2) Right click on f28377_flash.cmd and make sure "Exclude from Build" IS
     checked.
  3) Under Project->Properties->CCS Build->C2000 Compiler0->Advanced Options->
     Predefined Symbols, make sure that "FLASH" is not listed in the "Pre-
     define NAME" box. (If it is listed, click on the Delete icon with the
     red x).
  4) Click Ok on the Project Properties window.
  5) Click Project->Clean. If this doesn't start a build, then follow with
     Project->Build Project. The project name must be highlighted.

To build for Flash (running without CCS after initial load/flashing)

  1) Right click on f28377.cmd and make sure "Exclude from Build" IS
     checked.
  2) Right click on f28377_flash.cmd and make sure "Exclude from Build" is
     NOT checked.
  3) Under Project->Properties->CCS Build->C2000 Compiler0->Advanced Options->
     Predefined Symbols, make sure that "FLASH" is listed in the "Pre-
     define NAME" box. (If not listed, click on the Add icon with the
     green + and add FLASH).
  4) Click Ok on the Project Properties window.
  5) Click Project->Clean. If this doesn't start a build, then follow with
     Project->Build Project. The project name must be highlighted.
  6) Load the program into the flash by loading the program with CCS.


Starting the CCS Debugger:

  1) Connect the LaunchPad to the computer using the USB cable.
  2) In the Target Configuration window, right click on f28377.ccxml and
     select "Launch Selected Configuration". The debugger windows should
     open (the context changes from CCS Edit to CCS Debug).
  3) Click on the Connect Target icon.
  4) Click on the Load Program icon, and browse to rgb_f28377.out. Click Ok.
     If you have built for flash, CCS will now erase the flash and write the
     program sections to flash. Otherwise, CCS will load to RAM.
  5) You are ready to debug. Set breakpoints, start running or stepping through
     code, inspect memory, etc.
