#include "mq3.h"
#include "adc.h"
#include "bsp.h"

/*
 * 함수 정리:
 * - mq3Init(): MQ-3 센서 초기화
 * - mq3ReadOnce(): ADC 1회 측정
 * - mq3ReadAverage(): 여러 번 측정 후 평균값 반환
 */
void mq3Init(void)
{
    /* 핀과 ADC 설정은 CubeMX가 생성한 MX_ADC1_Init()에서 완료된다. */
    /* ADC1 is initialized in MX_ADC1_Init(). */
}

uint16_t mq3ReadOnce(void)
{
    /* Software trigger 방식으로 ADC1을 한 번 변환한다. */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);

    uint16_t value = (uint16_t)HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}

uint16_t mq3ReadAverage(uint8_t count)
{
    if (count == 0U) {
        return 0;
    }

    uint32_t sum = 0;

    for (uint8_t i = 0; i < count; i++) {
        sum += mq3ReadOnce();

        /* 센서 반응 변화를 반영하도록 평균 샘플 사이에 500ms 간격을 둔다. */
        if (i + 1U < count) {
            delay_ms(500);
        }
    }

    return (uint16_t)(sum / count);
}
