#pragma once
#include "hw_def.h"

bool uartInit(void);
/* ch는 향후 다중 UART 확장용이며 현재 구현은 USART2만 사용한다. */
bool uartOpen(uint8_t ch,uint32_t baud);
uint32_t uartWrite(uint8_t ch, uint8_t *p_data,uint32_t length);
uint32_t uartPrintf(uint8_t ch, char* fmt,...);
uint32_t uartRead(uint8_t ch);
uint32_t uartAvailable(uint8_t ch);
