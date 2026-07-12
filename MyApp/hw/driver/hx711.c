#include "hx711.h"

static void HX711_Delay(void)
{
    for(volatile int i = 0; i < 10; i++);
}

uint8_t HX711_IsReady(HX711_t *hx)
{
    return (HAL_GPIO_ReadPin(
                hx->DT_Port,
                hx->DT_Pin)
            == GPIO_PIN_RESET);
}

int32_t HX711_Read(HX711_t *hx)
{
    uint32_t data = 0;

    while(HAL_GPIO_ReadPin(hx->DT_Port, hx->DT_Pin));

    for(int i = 0; i < 24; i++)
    {
        HAL_GPIO_WritePin(hx->SCK_Port,
                          hx->SCK_Pin,
                          GPIO_PIN_SET);

        HX711_Delay();

        data <<= 1;

        if(HAL_GPIO_ReadPin(hx->DT_Port,
                            hx->DT_Pin))
        {
            data |= 1;
        }

        HAL_GPIO_WritePin(hx->SCK_Port,
                          hx->SCK_Pin,
                          GPIO_PIN_RESET);

        HX711_Delay();
    }

    // 25번째 클럭 (채널 A, Gain 128)
    HAL_GPIO_WritePin(hx->SCK_Port,
                      hx->SCK_Pin,
                      GPIO_PIN_SET);

    HX711_Delay();

    HAL_GPIO_WritePin(hx->SCK_Port,
                      hx->SCK_Pin,
                      GPIO_PIN_RESET);

    HX711_Delay();

    // 부호 확장
    if(data & 0x800000)
    {
        data |= 0xFF000000;
    }

    return (int32_t)data;
}

int32_t HX711_ReadAverage(HX711_t *hx,
                          uint8_t times)
{
    int64_t sum = 0;

    for(int i = 0; i < times; i++)
    {
        sum += HX711_Read(hx);
    }

    return (int32_t)(sum / times);
}

void HX711_Tare(HX711_t *hx,
                uint8_t times)
{
    hx->offset =
        HX711_ReadAverage(hx, times);
}

int32_t HX711_GetValue(HX711_t *hx,
                       uint8_t times)
{
    return HX711_ReadAverage(hx, times)
           - hx->offset;
}

float HX711_GetKg(HX711_t *hx, uint8_t times)
{
    return (float)HX711_GetValue(hx, times) / hx->scale;
}