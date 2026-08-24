#include "motor_control.h"

#include "relay.h"
#include "tim.h"

#define MOTOR_PWM_TIMER                htim1
#define MOTOR_PWM_CHANNEL              TIM_CHANNEL_1
#define MOTOR_NORMAL_SPEED_PERCENT     70U
#define MOTOR_WARNING_SPEED_PERCENT    30U
#define MOTOR_RAMP_STEP_PERCENT        1U
#define MOTOR_RAMP_INTERVAL_MS         50U

static uint8_t current_speed_percent = 0;
static uint8_t target_speed_percent = 0;
static uint32_t last_ramp_time = 0;

static void motorWriteSpeed(uint8_t percent)
{
    uint32_t compare;

    if (percent > 100U) {
        percent = 100U;
    }

    compare = ((__HAL_TIM_GET_AUTORELOAD(&MOTOR_PWM_TIMER) + 1U) * percent) / 100U;
    __HAL_TIM_SET_COMPARE(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, compare);
    current_speed_percent = percent;
}

static void motorSetForward(void)
{
    HAL_GPIO_WritePin(MOTOR_1_GPIO_Port, MOTOR_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_2_GPIO_Port, MOTOR_2_Pin, GPIO_PIN_RESET);
}

static void motorCoast(void)
{
    HAL_GPIO_WritePin(MOTOR_1_GPIO_Port, MOTOR_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_2_GPIO_Port, MOTOR_2_Pin, GPIO_PIN_RESET);
}

void motorControlInit(void)
{
    /* Keep the motor disabled until an explicit UNLOCK command arrives. */
    motorWriteSpeed(0U);
    motorCoast();
    if (HAL_TIM_PWM_Start(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL) != HAL_OK) {
        Error_Handler();
    }
    relayInit();
    target_speed_percent = 0U;
    last_ramp_time = HAL_GetTick();
}

void motorControlUpdate(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_ramp_time) < MOTOR_RAMP_INTERVAL_MS) {
        return;
    }
    last_ramp_time = now;

    if (current_speed_percent < target_speed_percent) {
        uint8_t next = current_speed_percent + MOTOR_RAMP_STEP_PERCENT;
        motorWriteSpeed(next > target_speed_percent ? target_speed_percent : next);
    } else if (current_speed_percent > target_speed_percent) {
        uint8_t next = current_speed_percent - MOTOR_RAMP_STEP_PERCENT;
        motorWriteSpeed(next < target_speed_percent ? target_speed_percent : next);
    }
}

void motorControlUnlock(void)
{
    if (!relayIsOn()) {
        motorWriteSpeed(0U);
        motorSetForward();
        relayOn();
    }
    target_speed_percent = MOTOR_NORMAL_SPEED_PERCENT;
    last_ramp_time = HAL_GetTick();
}

void motorControlLock(void)
{
    target_speed_percent = 0U;
    motorWriteSpeed(0U);
    motorCoast();
    relayOff();
}

void motorControlLimitSpeed(void)
{
    if (relayIsOn()) {
        target_speed_percent = MOTOR_WARNING_SPEED_PERCENT;
    }
}

void motorControlResumeSpeed(void)
{
    if (relayIsOn()) {
        target_speed_percent = MOTOR_NORMAL_SPEED_PERCENT;
    }
}

bool motorControlIsUnlocked(void)
{
    return relayIsOn();
}

uint8_t motorControlGetSpeedPercent(void)
{
    return current_speed_percent;
}
