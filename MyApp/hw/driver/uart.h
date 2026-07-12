#pragma once
#include "hw_def.h"

bool uartInit(void);
bool uartOpen(uint8_t ch,uint32_t baud);
uint32_t uartWrite(uint8_t ch, uint8_t *p_data,uint32_t length);
uint32_t uartPrintf(uint8_t ch, char* fmt,...);
uint32_t uartRead(uint8_t ch);
uint32_t uartAvailable(uint8_t ch);
