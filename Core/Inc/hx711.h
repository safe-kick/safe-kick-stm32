#ifndef __HX711_H__
#define __HX711_H__

#include "main.h"

typedef struct
{
    GPIO_TypeDef *DT_Port;
    uint16_t DT_Pin;

    GPIO_TypeDef *SCK_Port;
    uint16_t SCK_Pin;

    int32_t offset;
    float scale;      // 추가
} HX711_t;

uint8_t HX711_IsReady(HX711_t *hx);

int32_t HX711_Read(HX711_t *hx);

int32_t HX711_ReadAverage(HX711_t *hx, uint8_t times);

void HX711_Tare(HX711_t *hx, uint8_t times);

int32_t HX711_GetValue(HX711_t *hx, uint8_t times);
float HX711_GetKg(HX711_t *hx, uint8_t times);

#endif
