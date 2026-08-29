#pragma once

#include "hw_def.h"

void motorControlInit(void);
/* 메인 루프에서 반복 호출해 non-blocking 속도 ramp를 진행한다. */
void motorControlUpdate(void);
void motorControlUnlock(void);
void motorControlLock(void);
void motorControlPause(void);
void motorControlIncreaseSpeed(uint8_t step_percent);
void motorControlDecreaseSpeed(uint8_t step_percent);
void motorControlLimitSpeed(void);
void motorControlResumeSpeed(void);
bool motorControlIsUnlocked(void);
uint8_t motorControlGetSpeedPercent(void);
