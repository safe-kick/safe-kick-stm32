#pragma once

#include "hw_def.h"

void mq3Init(void);
uint16_t mq3ReadOnce(void);
uint16_t mq3ReadAverage(uint8_t count);
