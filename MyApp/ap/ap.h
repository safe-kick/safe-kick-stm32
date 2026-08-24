#pragma once
#include "hw_def.h"
#include "bsp.h"
#include "uart.h"
#include "hx711.h"

/* UART 명령, 센서 스트림, actuator update를 실행하는 firmware main loop. */
void apMain(void);
