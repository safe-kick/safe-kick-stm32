#include "bsp.h"

/*
 * 함수 정리:
 * - delay_ms(): 밀리초 단위 지연
 */
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
