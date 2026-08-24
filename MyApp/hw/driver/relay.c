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
    /* GPIO 출력과 MOTOR_STATE에 사용할 software 상태를 함께 변경한다. */
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_ON_STATE);
    relay_is_on = true;
}

void relayOff(void)
{
    /* 부팅과 LOCK의 기본 상태는 relay OFF다. */
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, RELAY_OFF_STATE);
    relay_is_on = false;
}

bool relayIsOn(void)
{
    return relay_is_on;
}
