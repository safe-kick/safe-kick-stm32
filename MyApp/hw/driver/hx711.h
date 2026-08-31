#pragma once
#include "main.h"

#include <stdbool.h>

typedef struct
{
    /* HX711 1채널의 data/clock 핀과 설치별 보정값. */
    GPIO_TypeDef *DT_Port;
    uint16_t DT_Pin;

    GPIO_TypeDef *SCK_Port;
    uint16_t SCK_Pin;

    int32_t offset; /* 부팅 tare에서 측정한 무부하 raw 값 */
    float scale;    /* kg 변환용 raw-counts/kg 보정계수 */
} HX711_t;

bool HX711_Read(HX711_t *hx, uint32_t timeout_ms, int32_t *value);
bool HX711_ReadAverage(HX711_t *hx,
                       uint8_t times,
                       uint32_t timeout_ms,
                       int32_t *average);
bool HX711_Tare(HX711_t *hx, uint8_t times, uint32_t timeout_ms);
bool HX711_GetValue(HX711_t *hx,
                    uint8_t times,
                    uint32_t timeout_ms,
                    int32_t *value);
bool HX711_GetKg(HX711_t *hx,
                 uint8_t times,
                 uint32_t timeout_ms,
                 float *kg);
