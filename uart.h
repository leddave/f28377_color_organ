#ifndef UART_H
#define UART_H

//#include <stdio.h>
//#include <string.h>
//#include "F28x_Project.h"     // Device Headerfile and Examples Include File
//#include "F2837xS_sci.h"

#define UART_MAX_RX_MSG                  32

//#define UART_A_BASE                    0x00007200
//#define UART_B_BASE                    0x00007210
//#define UART_C_BASE                    0x00007220
//#define UART_D_BASE                    0x00007230

void uart_init(uint32_t baud);
void uart_init_fifo(void);
void uart_send(char tx);
void uart_send_msg(char *msg);
char uart_recv(char *rx);
char uart_recv_msg(char *msg, uint16_t ms_timeout);

#endif //UART_H
