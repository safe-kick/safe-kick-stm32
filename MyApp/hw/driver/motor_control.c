#include "motor_control.h"

#include "relay.h"
#include "tim.h"

#define MOTOR_PWM_TIMER                htim1
#define MOTOR_PWM_CHANNEL              TIM_CHANNEL_1
#define MOTOR_MINIMUM_SPEED_PERCENT    30U
#define MOTOR_MAXIMUM_SPEED_PERCENT    70U
#define MOTOR_WARNING_SPEED_PERCENT    30U
#define MOTOR_RAMP_STEP_PERCENT        1U
#define MOTOR_RAMP_INTERVAL_MS         25U

/* current는 실제 PWM, target은 update()가 따라갈 목표 PWM이다. */
static uint8_t current_speed_percent = 0;
static uint8_t target_speed_percent = 0;
static uint32_t last_ramp_time = 0;
static bool warning_limited = false;

static void motorWriteSpeed(uint8_t percent)
{
    uint32_t compare;

    if (percent > 100U) {
        percent = 100U;
    }

    /* TIM1 ARR 범위를 0~100% duty로 선형 변환한다. */
    compare = ((__HAL_TIM_GET_AUTORELOAD(&MOTOR_PWM_TIMER) + 1U) * percent) / 100U;
    __HAL_TIM_SET_COMPARE(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, compare);
    current_speed_percent = percent;
}

static void motorSetForward(void)
{
    /* L298N IN1=HIGH, IN2=LOW 조합을 전진 방향으로 사용한다. */
    HAL_GPIO_WritePin(MOTOR_1_GPIO_Port, MOTOR_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_2_GPIO_Port, MOTOR_2_Pin, GPIO_PIN_RESET);
}

static void motorCoast(void)
{
    /* 두 방향 입력을 LOW로 만들어 H-bridge 출력을 해제한다. */
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
    warning_limited = false;
    last_ramp_time = HAL_GetTick();
}

void motorControlUpdate(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_ramp_time) < MOTOR_RAMP_INTERVAL_MS) {
        return;
    }
    last_ramp_time = now;

    /* delay를 사용하지 않고 25ms마다 1%만 변경해 다른 센서 처리를 유지한다. */
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
    /* 방향과 릴레이만 준비하며, 앞쪽 하중이 감지될 때까지 PWM은 0%로 유지한다. */
    if (!relayIsOn()) {
        motorWriteSpeed(0U);
        motorSetForward();
        relayOn();
    }
    warning_limited = false;
    target_speed_percent = 0U;
    last_ramp_time = HAL_GetTick();
}

void motorControlLock(void)
{
    /* 안전 명령이므로 ramp 없이 PWM 0%, coast, relay OFF를 즉시 적용한다. */
    target_speed_percent = 0U;
    warning_limited = false;
    motorWriteSpeed(0U);
    motorCoast();
    relayOff();
}

void motorControlPause(void)
{
    /* 무인 발판에서는 릴레이와 방향은 유지하고 PWM만 즉시 제거한다. */
    target_speed_percent = 0U;
    motorWriteSpeed(0U);
}

void motorControlIncreaseSpeed(uint8_t step_percent)
{
    uint16_t next;

    if (!relayIsOn() || warning_limited) {
        return;
    }

    if (target_speed_percent == 0U) {
        target_speed_percent = MOTOR_MINIMUM_SPEED_PERCENT;
        /* 최초 전방 하중 확정 시에는 기동 PWM 30%를 즉시 적용한다. */
        motorWriteSpeed(MOTOR_MINIMUM_SPEED_PERCENT);
        return;
    }

    next = (uint16_t)target_speed_percent + step_percent;
    target_speed_percent = next > MOTOR_MAXIMUM_SPEED_PERCENT
        ? MOTOR_MAXIMUM_SPEED_PERCENT
        : (uint8_t)next;
}

void motorControlDecreaseSpeed(uint8_t step_percent)
{
    if (!relayIsOn() || warning_limited || target_speed_percent == 0U) {
        return;
    }

    if (target_speed_percent <= MOTOR_MINIMUM_SPEED_PERCENT + step_percent) {
        target_speed_percent = MOTOR_MINIMUM_SPEED_PERCENT;
    } else {
        target_speed_percent -= step_percent;
    }
}

void motorControlLimitSpeed(void)
{
    /* 경고는 현재보다 빠르게 만들지 않고 최대 출력만 30%로 제한한다. */
    if (relayIsOn()) {
        warning_limited = true;
        if (target_speed_percent > MOTOR_WARNING_SPEED_PERCENT) {
            target_speed_percent = MOTOR_WARNING_SPEED_PERCENT;
        }
    }
}

void motorControlResumeSpeed(void)
{
    warning_limited = false;
}

bool motorControlIsUnlocked(void)
{
    return relayIsOn();
}

uint8_t motorControlGetSpeedPercent(void)
{
    return current_speed_percent;
}
