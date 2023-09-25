#ifndef _UART_H_
#define _UART_H_

#include "stm32f10x.h"
#include "stdio.h"

#define USART_REC_LEN           200


void USART1_Config(void);
void NVIC_Configuration(void);
void uart_loop_proc(void);


int fputc(int ch, FILE *f);
void USART1_printf(USART_TypeDef *USARTx, uint8_t *Data, ...);

#endif
