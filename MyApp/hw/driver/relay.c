#include "relay.h"

/*
 * 함수 정리:
 * - relayInit(): 릴레이 초기화
 * - relayOn(): 릴레이 ON
 * - relayOff(): 릴레이 OFF
 * - relayIsOn(): 릴레이 상태 확인
 */
#define RELAY_ON_STATE  GPIO_PIN_SET
#define RELAY_OFF_STATE GPIO_PIN_RESET

static bool relay_is_on = false;

void relayInit(void)
{
    relayOff();
}

void relayOn(void)
{
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_ON_STATE);
    relay_is_on = true;
}

void relayOff(void)
{
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_OFF_STATE);
    relay_is_on = false;
}

bool relayIsOn(void)
{
    return relay_is_on;
}
