#include "hx711.h"

/*
 * 함수 정리:
 * - HX711_Read(): 1회 원시 데이터 읽기
 * - HX711_ReadAverage(): 여러 번 읽어서 평균
 * - HX711_Tare(): 영점 보정
 * - HX711_GetValue(): 영점 보정된 값 반환
 * - HX711_GetKg(): kg 단위로 변환
 */
static void HX711_Delay(void)
{
    /* HX711 SCK high/low 최소 폭을 만족하기 위한 짧은 software delay. */
    for(volatile int i = 0; i < 10; i++);
}

int32_t HX711_Read(HX711_t *hx)
{
    uint32_t data = 0;

    /* DT가 LOW가 되면 24-bit 변환 결과를 읽을 수 있다. 현재 timeout은 없다. */
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

    /* 25번째 클럭은 다음 변환의 채널 A, Gain 128을 선택한다. */
    HAL_GPIO_WritePin(hx->SCK_Port,
                      hx->SCK_Pin,
                      GPIO_PIN_SET);

    HX711_Delay();

    HAL_GPIO_WritePin(hx->SCK_Port,
                      hx->SCK_Pin,
                      GPIO_PIN_RESET);

    HX711_Delay();

    /* HX711의 24-bit 2's complement 값을 int32_t 부호로 확장한다. */
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
    /* 현재 무부하 평균을 이후 모든 측정에서 뺄 영점값으로 저장한다. */
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
    /* scale은 설치 상태에 따라 채널별로 보정한 raw-counts/kg 값이다. */
    return (float)HX711_GetValue(hx, times) / hx->scale;
}
