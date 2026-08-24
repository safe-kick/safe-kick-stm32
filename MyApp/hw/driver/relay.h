#pragma once

#include "hw_def.h"

/* PA7 릴레이 출력과 software 잠금 상태를 함께 관리한다. */
void relayInit(void);
void relayOn(void);
void relayOff(void);
bool relayIsOn(void);
