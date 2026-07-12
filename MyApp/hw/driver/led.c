#include "led.h"

void ledToggle(void){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

void ledON(void){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void ledOff(void){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}