/*****************************************************************************
 * This is the rgb_28377 project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "F2837xS_sci.h"
#include "uart.h"
#include "globals.h"
#include "utils.h"


//Global variables
extern volatile struct SCI_REGS SciaRegs;
extern volatile struct SCI_REGS ScibRegs;
uint32_t baud_delay_us; //microseconds per byte


//
//  uart_init - Test 1, SCI DLB, 8-bit word, baud rate 0x000F,
//              default, 1 STOP bit, no parity
//
void uart_init(uint32_t baud)
{
    //
    // Note: Clocks were turned on to the SCIA peripheral
    // in the InitSysCtrl() function
    //

    ScibRegs.SCICCR.all = 0x0007;   // 1 stop bit,  No loopback
                           // No parity,8 char bits,
                           // async mode, idle-line protocol
    ScibRegs.SCICTL1.all = 0x0003;  // enable TX, RX, internal SCICLK,
                           // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.all = 0x0003;
    ScibRegs.SCICTL2.bit.TXINTENA = 0;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 0;

    // unTested Baud rate codes:
    //   115200 = 0x0035

    if (baud == 115200)
    {
      ScibRegs.SCIHBAUD.all = 0x0000;
      ScibRegs.SCILBAUD.all = 0x0035; //115200
      baud_delay_us = (1000000 / (115200 / 8)) + 1; //microseconds per byte
    }
    else if (baud == 38400)
    {
      ScibRegs.SCIHBAUD.all = 0x0000;
      ScibRegs.SCILBAUD.all = 0x00a2; //38400
      baud_delay_us = (1000000 / (38400 / 8)) + 1; //microseconds per byte
    }
    else //default to 9600
    {
      ScibRegs.SCIHBAUD.all = 0x0002;
      ScibRegs.SCILBAUD.all = 0x008a; //9600
      baud_delay_us = (1000000 / (9600 / 8)) + 1; //microseconds per byte
    }

    ScibRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
}


//
// uart_init_fifo - Initialize the UART FIFO
//
void uart_init_fifo(void)
{
    ScibRegs.SCIFFTX.all = 0xE040;
    ScibRegs.SCIFFRX.all = 0x205f; //0x2044
    ScibRegs.SCIFFCT.all = 0x0;
}


//
// uart_send - Transmit a character from the UART
//
void uart_send(char tx)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST >= 12) {} //pause with 3/4 full fifo
      ScibRegs.SCITXBUF.all = tx;
    delay(baud_delay_us); //delay for 1 byte
}


//
// uart_send_msg - Transmit message via UART
//
void uart_send_msg(char *msg)
{
    int idx;

    idx = 0;
    while(msg[idx] != 0)
    {
      uart_send(msg[idx]);
      idx++;
    }
}


//
// uart_recv - Receive a character from the UART
//
char uart_recv(char *rx)
{
  if ((ScibRegs.SCIRXST.bit.RXRDY != 0) || //there's a char waiting
         (ScibRegs.SCIFFRX.bit.RXFFST > 0))
  {
    delay(baud_delay_us); //delay for 1 byte
    *rx = ScibRegs.SCIRXBUF.bit.SAR;
    return(1);
  }

  else
    return(0); //no char was waiting
}


char uart_recv_msg(char *msg, uint16_t ms_timeout)
{
  uint16_t idx = 0;

  while ((ScibRegs.SCIRXST.bit.RXRDY == 0) && //there's no bytes in Rx Fifo
         (ScibRegs.SCIFFRX.bit.RXFFST == 0) &&
         (idx < ms_timeout))
  {
    delay(1000); //delay for 1 ms
    idx ++;
  }

  idx = 0;

  while ((ScibRegs.SCIRXST.bit.RXRDY != 0) || //there's a char waiting
         (ScibRegs.SCIFFRX.bit.RXFFST > 0))
  {
    msg[idx++] = ScibRegs.SCIRXBUF.all;
    delay(baud_delay_us); //delay for 1 byte

    if (idx == (UART_MAX_RX_MSG-1))
      break;
  }

  msg[idx] = 0; //null terminate

  return(idx); //return char count
}
