#include "ap.h"
#include "bsp.h"
#include "buzzer.h"
#include "mq3.h"
#include "relay.h"
#include "uart.h"

#include <string.h>

static HX711_t hx1 = {GPIOB, GPIO_PIN_0,  GPIOB, GPIO_PIN_1,  0, 83387.0f};
static HX711_t hx2 = {GPIOB, GPIO_PIN_2,  GPIOB, GPIO_PIN_10, 0, 77144.0f};
static HX711_t hx3 = {GPIOB, GPIO_PIN_12, GPIOB, GPIO_PIN_13, 0, 82693.0f};
static HX711_t hx4 = {GPIOB, GPIO_PIN_14, GPIOB, GPIO_PIN_15, 0, 85663.0f};

static char rx_cmd[64];
static uint32_t rx_cmd_len = 0;
static uint8_t rx_terminator_seen = 0;
static uint8_t weight_stream_active = 0;
static uint32_t weight_last_sample_time = 0;
static uint8_t mq3_stream_active = 0;
static uint32_t mq3_last_sample_time = 0;

#define WEIGHT_SAMPLE_COUNT       1U
#define WEIGHT_STREAM_INTERVAL_MS 1000U

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
    if (ch == '\r' || ch == '\n') {
        return;
    }

    if ((unsigned char)ch < 32U) {
        return;
    }

    if (rx_cmd_len < (sizeof(rx_cmd) - 1)) {
        rx_cmd[rx_cmd_len++] = ch;
    } else {
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

static void send_mq3_measurements(uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        send_mq3_response();
        if (i + 1U < count) {
            delay_ms(500);
        }
    }
}

static void send_mq3_baseline(void)
{
    uint16_t baseline = mq3ReadAverage(8);
    uartPrintf(0, "MQ3_BASELINE:%u\r\n", baseline);
}

static void send_mq3_session(void)
{
    uartPrintf(0, "[CHECK_MQ3]\r\n");
    send_mq3_baseline();
    send_mq3_measurements(8);
    uartPrintf(0, "[END_MQ3]\r\n");
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
    if (strcmp(cmd, "CHECK_MQ3") == 0) {
        send_mq3_session();
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
        buzzerStop();
        relayOff();
        stop_weight_stream();
        uartPrintf(0, "LOCK_OK\r\n");
        return;
    }

    if (strcmp(cmd, "UNLOCK") == 0) {
        relayOn();
        uartPrintf(0, "UNLOCK_OK\r\n");
        return;
    }

    if (strcmp(cmd, "BUZZ_ON") == 0) {
        buzzerStart();
        return;
    }

    if (strcmp(cmd, "BUZZ_OFF") == 0) {
        buzzerStop();
        return;
    }

    if (cmd[0] != '\0') {
        uartPrintf(0, "ERR:UNKNOWN_CMD\r\n");
    }
}

void apMain(void)
{
    uartInit();
    relayInit();
    buzzerInit();

    uartPrintf(0, "Tare...\r\n");
    HX711_Tare(&hx1, 10);
    HX711_Tare(&hx2, 10);
    HX711_Tare(&hx3, 10);
    HX711_Tare(&hx4, 10);
    uartPrintf(0, "READY\r\n");

    while (1) {
        uint32_t now = HAL_GetTick();

        buzzerUpdate();

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
