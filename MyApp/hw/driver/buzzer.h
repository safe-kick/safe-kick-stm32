#pragma once

#include "hw_def.h"

/* PA6 부저를 non-blocking 1초 토글 경고음으로 제어한다. */
void buzzerInit(void);
void buzzerStart(void);
void buzzerStop(void);
void buzzerUpdate(void);
