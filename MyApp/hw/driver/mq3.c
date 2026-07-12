#include "mq3.h"
#include "adc.h"
#include "bsp.h"

void mq3Init(void)
{
    /* ADC1 is initialized in MX_ADC1_Init(). */
}

uint16_t mq3ReadOnce(void)
{
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

        if (i + 1U < count) {
            delay_ms(500);
        }
    }

    return (uint16_t)(sum / count);
}
