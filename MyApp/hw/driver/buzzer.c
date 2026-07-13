#include "buzzer.h"

#define BUZZER_ON_STATE       GPIO_PIN_SET
#define BUZZER_OFF_STATE      GPIO_PIN_RESET
#define BUZZER_TOGGLE_TIME_MS 1000U

static bool buzzer_active = false;
static bool buzzer_output_on = false;
static uint32_t buzzer_last_toggle_time = 0;

static void buzzerWrite(bool on)
{
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

    now = HAL_GetTick();
    if ((now - buzzer_last_toggle_time) >= BUZZER_TOGGLE_TIME_MS) {
        buzzerWrite(!buzzer_output_on);
        buzzer_last_toggle_time = now;
    }
}

bool buzzerIsActive(void)
{
    return buzzer_active;
}
