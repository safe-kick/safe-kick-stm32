#pragma once

#include "hw_def.h"

/* PA1 ADC1의 MQ-3 원시값을 1회 또는 평균으로 읽는다. */
void mq3Init(void);
uint16_t mq3ReadOnce(void);
uint16_t mq3ReadAverage(uint8_t count);
