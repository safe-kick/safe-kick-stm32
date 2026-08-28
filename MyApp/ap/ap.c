#include "ap.h"
#include "bsp.h"
#include "buzzer.h"
#include "mq3.h"
#include "motor_control.h"
#include "uart.h"

#include <string.h>

/*
 * 함수 정리:
 * - echo_rx_char(): UART 입력 echo
 * - append_rx_char(): 명령 문자열 누적
 * - trim_rx_cmd(): 명령 끝 공백 제거
 * - send_weight_response(): 하중값 전송
 * - start_weight_stream()/stop_weight_stream(): 하중 스트림 시작/정지
 * - send_mq3_response(): MQ-3 1회 측정값 전송
 * - update_mq3_operation(): baseline/안내음/실측 비차단 상태 처리
 * - start_mq3_stream()/stop_mq3_stream(): MQ-3 스트림 시작/정지
 * - process_command(): UART 명령 처리
 * - apMain(): 애플리케이션 메인 루프
 */
/*
 * 발판 네 모서리의 HX711 설정이다.
 * 마지막 두 값은 부팅 시 덮어쓸 offset과 로드셀별 kg 변환 scale이다.
 */
static HX711_t hx1 = {GPIOB, GPIO_PIN_0,  GPIOB, GPIO_PIN_1,  0, 83387.0f};
static HX711_t hx2 = {GPIOB, GPIO_PIN_2,  GPIOB, GPIO_PIN_10, 0, 77144.0f};
static HX711_t hx3 = {GPIOB, GPIO_PIN_12, GPIOB, GPIO_PIN_13, 0, 82693.0f};
static HX711_t hx4 = {GPIOB, GPIO_PIN_14, GPIOB, GPIO_PIN_15, 0, 85663.0f};

/* UART 한 줄 명령 조립 상태와 센서 스트림 실행 상태. */
static char rx_cmd[64];
static uint32_t rx_cmd_len = 0;
static uint8_t rx_terminator_seen = 0;
static uint8_t weight_stream_active = 0;
static uint32_t weight_last_sample_time = 0;
static uint8_t mq3_stream_active = 0;
static uint32_t mq3_last_sample_time = 0;

typedef enum {
    MQ3_OPERATION_IDLE = 0,
    MQ3_OPERATION_BASELINE,
    MQ3_OPERATION_PRE_MEASURE_BUZZER,
    MQ3_OPERATION_MEASURE
} mq3_operation_t;

static mq3_operation_t mq3_operation = MQ3_OPERATION_IDLE;
static uint8_t mq3_legacy_session = 0;
static uint8_t mq3_sample_count = 0;
static uint32_t mq3_sample_sum = 0;
static uint32_t mq3_operation_time = 0;

#define WEIGHT_SAMPLE_COUNT       1U
#define WEIGHT_STREAM_INTERVAL_MS 1000U
/* baseline과 실제 측정은 각각 8회, 500ms 간격을 유지한다. */
#define MQ3_BASELINE_SAMPLE_COUNT  8U
#define MQ3_MEASURE_SAMPLE_COUNT   8U
#define MQ3_BASELINE_INTERVAL_MS   500U
#define MQ3_MEASURE_INTERVAL_MS    500U
/* 실제 측정 전에 한 번만 울리는 유한한 안내음 길이. */
#define MQ3_PRE_MEASURE_BUZZER_MS  1000U

static void echo_rx_char(char ch)
{
    if (ch == '\r') {
        uartPrintf(0, "\r\n");
        return;
    }

    uartWrite(0, (uint8_t *)&ch, 1);
}

static void append_rx_char(char ch)
{
    /* 줄 종결자와 제어문자는 명령 본문에 저장하지 않는다. */
    if (ch == '\r' || ch == '\n') {
        return;
    }

    if ((unsigned char)ch < 32U) {
        return;
    }

    if (rx_cmd_len < (sizeof(rx_cmd) - 1)) {
        rx_cmd[rx_cmd_len++] = ch;
    } else {
        /* 잘린 명령을 실행하지 않도록 버퍼 전체를 폐기한다. */
        rx_cmd_len = 0;
        rx_cmd[0] = '\0';
    }
}

static void trim_rx_cmd(char *cmd)
{
    size_t len = strlen(cmd);

    while (len > 0 && (cmd[len - 1] == ' ' || cmd[len - 1] == '\t')) {
        cmd[len - 1] = '\0';
        len--;
    }
}

static void send_weight_response(void)
{
    /* Raspberry Pi는 개별 하중과 네 채널 합계 TOTAL을 함께 사용한다. */
    float fl = HX711_GetKg(&hx1, WEIGHT_SAMPLE_COUNT);
    float fr = HX711_GetKg(&hx2, WEIGHT_SAMPLE_COUNT);
    float rl = HX711_GetKg(&hx3, WEIGHT_SAMPLE_COUNT);
    float rr = HX711_GetKg(&hx4, WEIGHT_SAMPLE_COUNT);
    float total = fl + fr + rl + rr;

    uartPrintf(0,
               "FL:%.2f FR:%.2f RL:%.2f RR:%.2f TOTAL:%.2f\r\n",
               fl, fr, rl, rr, total);
}

static void start_weight_stream(void)
{
    /* 첫 샘플은 다음 1초 주기에 보내 탑승자가 자세를 잡을 시간을 준다. */
    uartPrintf(0, "[CHECK_WEIGHT]\r\n");
    weight_stream_active = 1;
    weight_last_sample_time = HAL_GetTick();
}

static void stop_weight_stream(void)
{
    if (weight_stream_active) {
        weight_stream_active = 0;
        uartPrintf(0, "[END_WEIGHT]\r\n");
    }
}

static void send_mq3_response(void)
{
    uint16_t mq3 = mq3ReadOnce();
    uartPrintf(0, "MQ3:%u\r\n", mq3);
}

static void begin_mq3_measure(uint32_t now)
{
    buzzerStop();
    uartPrintf(0, "MEASURE_BEGIN\r\n");
    mq3_operation = MQ3_OPERATION_MEASURE;
    mq3_sample_count = 1U;
    mq3_operation_time = now;
    send_mq3_response();
}

static void start_mq3_baseline(uint8_t legacy_session)
{
    if (mq3_operation != MQ3_OPERATION_IDLE) {
        uartPrintf(0, "ERR:MQ3_BUSY\r\n");
        return;
    }

    mq3_legacy_session = legacy_session;
    uartPrintf(0,
               legacy_session ? "[CHECK_MQ3]\r\n" : "[CHECK_MQ3_BASELINE]\r\n");
    mq3_sample_sum = mq3ReadOnce();
    mq3_sample_count = 1U;
    mq3_operation_time = HAL_GetTick();
    mq3_operation = MQ3_OPERATION_BASELINE;
}

static void start_mq3_measure(void)
{
    if (mq3_operation != MQ3_OPERATION_IDLE) {
        uartPrintf(0, "ERR:MQ3_BUSY\r\n");
        return;
    }

    mq3_legacy_session = 0U;
    mq3_operation = MQ3_OPERATION_PRE_MEASURE_BUZZER;
    mq3_operation_time = HAL_GetTick();
    uartPrintf(0, "[CHECK_MQ3_MEASURE]\r\n");
    buzzerStart();
}

static void cancel_mq3_operation(void)
{
    if (mq3_operation == MQ3_OPERATION_PRE_MEASURE_BUZZER) {
        buzzerStop();
    }
    mq3_operation = MQ3_OPERATION_IDLE;
    mq3_legacy_session = 0U;
    mq3_sample_count = 0U;
    mq3_sample_sum = 0U;
}

static void update_mq3_operation(uint32_t now)
{
    if (mq3_operation == MQ3_OPERATION_BASELINE &&
        (now - mq3_operation_time) >= MQ3_BASELINE_INTERVAL_MS) {
        mq3_sample_sum += mq3ReadOnce();
        mq3_sample_count++;
        mq3_operation_time = now;

        if (mq3_sample_count >= MQ3_BASELINE_SAMPLE_COUNT) {
            uartPrintf(0,
                       "MQ3_BASELINE:%u\r\n",
                       (uint16_t)(mq3_sample_sum / MQ3_BASELINE_SAMPLE_COUNT));
            if (mq3_legacy_session) {
                mq3_operation = MQ3_OPERATION_PRE_MEASURE_BUZZER;
                mq3_operation_time = now;
                buzzerStart();
            } else {
                uartPrintf(0, "[END_MQ3_BASELINE]\r\n");
                cancel_mq3_operation();
            }
        }
        return;
    }

    if (mq3_operation == MQ3_OPERATION_PRE_MEASURE_BUZZER &&
        (now - mq3_operation_time) >= MQ3_PRE_MEASURE_BUZZER_MS) {
        begin_mq3_measure(now);
        return;
    }

    if (mq3_operation == MQ3_OPERATION_MEASURE &&
        (now - mq3_operation_time) >= MQ3_MEASURE_INTERVAL_MS) {
        send_mq3_response();
        mq3_sample_count++;
        mq3_operation_time = now;

        if (mq3_sample_count >= MQ3_MEASURE_SAMPLE_COUNT) {
            uartPrintf(0, "MEASURE_END\r\n");
            uartPrintf(0,
                       mq3_legacy_session ? "[END_MQ3]\r\n" : "[END_MQ3_MEASURE]\r\n");
            cancel_mq3_operation();
        }
    }
}

static void send_mq3_session(void)
{
    /* 기존 시리얼 도구 호환: baseline 뒤 새 유한 안내음/실측 흐름을 실행한다. */
    start_mq3_baseline(1U);
}

static void start_mq3_stream(void)
{
    mq3_stream_active = 1;
    mq3_last_sample_time = 0;
    uartPrintf(0, "MQ3_STREAM_ON\r\n");
}

static void stop_mq3_stream(void)
{
    mq3_stream_active = 0;
    uartPrintf(0, "MQ3_STREAM_OFF\r\n");
}

static void process_command(const char *cmd)
{
    /* 센서 시험 명령과 실제 운행 제어 명령을 한 곳에서 분기한다. */
    if (strcmp(cmd, "CHECK_MQ3") == 0) {
        send_mq3_session();
        return;
    }

    if (strcmp(cmd, "CHECK_MQ3_BASELINE") == 0) {
        start_mq3_baseline(0U);
        return;
    }

    if (strcmp(cmd, "CHECK_MQ3_MEASURE") == 0) {
        start_mq3_measure();
        return;
    }

    if (strcmp(cmd, "TEST_MQ3") == 0) {
        start_mq3_stream();
        return;
    }

    if (strcmp(cmd, "STOP_TEST_MQ3") == 0) {
        stop_mq3_stream();
        return;
    }

    if (strcmp(cmd, "CHECK_WEIGHT") == 0) {
        start_weight_stream();
        return;
    }

    if (strcmp(cmd, "STOP_WEIGHT") == 0) {
        stop_weight_stream();
        uartPrintf(0, "WEIGHT_STREAM_OFF\r\n");
        return;
    }

    if (strcmp(cmd, "LOCK") == 0) {
        /* 잠금은 경고 ramp와 달리 출력과 릴레이를 즉시 차단한다. */
        cancel_mq3_operation();
        buzzerStop();
        motorControlLock();
        stop_weight_stream();
        uartPrintf(0, "LOCK_OK\r\n");
        return;
    }

    if (strcmp(cmd, "UNLOCK") == 0) {
        motorControlUnlock();
        uartPrintf(0, "UNLOCK_OK\r\n");
        return;
    }

    if (strcmp(cmd, "MOTOR_STATE") == 0) {
        uartPrintf(0, "MOTOR:%s SPEED:%u\r\n",
                   motorControlIsUnlocked() ? "UNLOCKED" : "LOCKED",
                   motorControlGetSpeedPercent());
        return;
    }

    if (strcmp(cmd, "BUZZ_ON") == 0) {
        /* 과적 경고 중에도 릴레이는 유지하고 목표 속도만 30%로 낮춘다. */
        buzzerStart();
        motorControlLimitSpeed();
        return;
    }

    if (strcmp(cmd, "BUZZ_OFF") == 0) {
        buzzerStop();
        motorControlResumeSpeed();
        return;
    }

    if (cmd[0] != '\0') {
        uartPrintf(0, "ERR:UNKNOWN_CMD\r\n");
    }
}

void apMain(void)
{
    /* 모든 출력은 잠금 상태로 초기화한 뒤 센서 영점을 잡는다. */
    uartInit();
    motorControlInit();
    buzzerInit();
    uartPrintf(0, "Tare...\r\n");
    /* 이 구간에 하중이 있으면 해당 무게가 영점으로 저장된다. */
    HX711_Tare(&hx1, 10);
    HX711_Tare(&hx2, 10);
    HX711_Tare(&hx3, 10);
    HX711_Tare(&hx4, 10);
    uartPrintf(0, "READY\r\n");

    while (1) {
        uint32_t now = HAL_GetTick();

        buzzerUpdate();
        update_mq3_operation(now);
        /* 두 update 함수는 delay 없이 시간 차이만 확인해 메인 루프를 막지 않는다. */
        motorControlUpdate();

        if (mq3_stream_active && (now - mq3_last_sample_time >= 500U)) {
            mq3_last_sample_time = now;
            send_mq3_response();
        }

        if (weight_stream_active &&
            (now - weight_last_sample_time >= WEIGHT_STREAM_INTERVAL_MS)) {
            weight_last_sample_time = now;
            send_weight_response();
        }

        while (uartAvailable(0) > 0) {
            char ch = (char)uartRead(0);
            echo_rx_char(ch);
            if (ch == '\r' || ch == '\n') {
                /* CRLF를 하나의 줄 끝으로 처리해 명령이 두 번 실행되지 않게 한다. */
                if (rx_terminator_seen) {
                    continue;
                }

                rx_terminator_seen = 1;
                rx_cmd[rx_cmd_len] = '\0';
                trim_rx_cmd(rx_cmd);

                if (rx_cmd[0] != '\0') {
                    process_command(rx_cmd);
                }

                rx_cmd_len = 0;
                rx_cmd[0] = '\0';
            } else {
                rx_terminator_seen = 0;
                append_rx_char(ch);
            }
        }
    }
}
