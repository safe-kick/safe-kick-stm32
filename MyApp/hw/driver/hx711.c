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

bool HX711_Read(HX711_t *hx, uint32_t timeout_ms, int32_t *value)
{
    uint32_t data = 0;
    uint32_t started_at = HAL_GetTick();

    /* DT가 LOW가 되지 않으면 메인 루프를 영구 정지시키지 않고 실패를 반환한다. */
    while(HAL_GPIO_ReadPin(hx->DT_Port, hx->DT_Pin))
    {
        if ((HAL_GetTick() - started_at) >= timeout_ms)
        {
            return false;
        }
    }

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

    *value = (int32_t)data;
    return true;
}

bool HX711_ReadAverage(HX711_t *hx,
                       uint8_t times,
                       uint32_t timeout_ms,
                       int32_t *average)
{
    int64_t sum = 0;
    int32_t sample;

    if (times == 0U)
    {
        return false;
    }

    for(int i = 0; i < times; i++)
    {
        if (!HX711_Read(hx, timeout_ms, &sample))
        {
            return false;
        }
        sum += sample;
    }

    *average = (int32_t)(sum / times);
    return true;
}

bool HX711_Tare(HX711_t *hx,
                uint8_t times,
                uint32_t timeout_ms)
{
    int32_t average;

    /* 현재 무부하 평균을 이후 모든 측정에서 뺄 영점값으로 저장한다. */
    if (!HX711_ReadAverage(hx, times, timeout_ms, &average))
    {
        return false;
    }

    hx->offset = average;
    return true;
}

bool HX711_GetValue(HX711_t *hx,
                    uint8_t times,
                    uint32_t timeout_ms,
                    int32_t *value)
{
    int32_t average;

    if (!HX711_ReadAverage(hx, times, timeout_ms, &average))
    {
        return false;
    }

    *value = average - hx->offset;
    return true;
}

bool HX711_GetKg(HX711_t *hx,
                 uint8_t times,
                 uint32_t timeout_ms,
                 float *kg)
{
    int32_t value;

    if (hx->scale == 0.0f ||
        !HX711_GetValue(hx, times, timeout_ms, &value))
    {
        return false;
    }

    /* scale은 설치 상태에 따라 채널별로 보정한 raw-counts/kg 값이다. */
    *kg = (float)value / hx->scale;
    return true;
}
