#include "buzzer.h"

/*
 * 함수 정리:
 * - buzzerInit(): 부저 초기화
 * - buzzerStart(): 부저 동작 시작
 * - buzzerStop(): 부저 정지
 * - buzzerUpdate(): 주기적으로 ON/OFF 토글
 */
#define BUZZER_ON_STATE       GPIO_PIN_SET
#define BUZZER_OFF_STATE      GPIO_PIN_RESET
#define BUZZER_TOGGLE_TIME_MS 1000U

static bool buzzer_active = false;
static bool buzzer_output_on = false;
static uint32_t buzzer_last_toggle_time = 0;

static void buzzerWrite(bool on)
{
    /* 하드웨어 출력과 software 상태를 항상 같이 갱신한다. */
    HAL_GPIO_WritePin(BUZZER_GPIO_Port,
                      BUZZER_Pin,
                      on ? BUZZER_ON_STATE : BUZZER_OFF_STATE);
    buzzer_output_on = on;
}

void buzzerInit(void)
{
    buzzerStop();
}

void buzzerStart(void)
{
    /* 경고를 즉시 알리기 위해 시작 순간에는 ON으로 출력한다. */
    buzzer_active = true;
    buzzerWrite(true);
    buzzer_last_toggle_time = HAL_GetTick();
}

void buzzerStop(void)
{
    buzzer_active = false;
    buzzerWrite(false);
}

void buzzerUpdate(void)
{
    uint32_t now;

    if (!buzzer_active) {
        return;
    }

    /* HAL_Delay 없이 1초마다 토글해 UART와 센서 루프를 계속 실행한다. */
    now = HAL_GetTick();
    if ((now - buzzer_last_toggle_time) >= BUZZER_TOGGLE_TIME_MS) {
        buzzerWrite(!buzzer_output_on);
        buzzer_last_toggle_time = now;
    }
}
