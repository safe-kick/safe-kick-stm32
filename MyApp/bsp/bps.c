#include "bsp.h"

/*
 * 함수 정리:
 * - delay_ms(): 밀리초 단위 지연
 */
void delay_ms(uint32_t ms)
{
    /* MQ-3 측정 시퀀스처럼 의도적으로 대기해야 하는 구간에만 사용한다. */
    HAL_Delay(ms);
}
