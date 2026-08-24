#pragma once

#include "hw_def.h"

void motorControlInit(void);
void motorControlUpdate(void);
void motorControlUnlock(void);
void motorControlLock(void);
void motorControlLimitSpeed(void);
void motorControlResumeSpeed(void);
bool motorControlIsUnlocked(void);
uint8_t motorControlGetSpeedPercent(void);
