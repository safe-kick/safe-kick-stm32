#include "ap.h"
#include "bsp.h"
#include "mq3.h"
#include "uart.h"

#include <string.h>

static HX711_t hx1 = {GPIOB, GPIO_PIN_0,  GPIOB, GPIO_PIN_1,  0, 85350.0f};
static HX711_t hx2 = {GPIOB, GPIO_PIN_2,  GPIOB, GPIO_PIN_10, 0, 78960.0f};
static HX711_t hx3 = {GPIOB, GPIO_PIN_12, GPIOB, GPIO_PIN_13, 0, 84640.0f};
static HX711_t hx4 = {GPIOB, GPIO_PIN_14, GPIOB, GPIO_PIN_15, 0, 87680.0f};

static char rx_cmd[64];
static uint32_t rx_cmd_len = 0;
static uint8_t rx_terminator_seen = 0;
static uint8_t weight_stream_active = 0;
static uint32_t weight_last_sample_time = 0;
static uint8_t mq3_stream_active = 0;
static uint32_t mq3_last_sample_time = 0;

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
    const uint8_t times = 10;
    float fl = HX711_GetKg(&hx1, times);
    float fr = HX711_GetKg(&hx2, times);
    float rl = HX711_GetKg(&hx3, times);
    float rr = HX711_GetKg(&hx4, times);
    float total = fl + fr + rl + rr;

    uartPrintf(0,
               "FL:%.2f FR:%.2f RL:%.2f RR:%.2f TOTAL:%.2f\r\n",
               fl, fr, rl, rr, total);
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
    send_mq3_baseline();
    send_mq3_measurements(8);
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
        weight_stream_active = 1;
        weight_last_sample_time = 0;
        send_weight_response();
        weight_last_sample_time = HAL_GetTick();
        uartPrintf(0, "WEIGHT_STREAM_ON\r\n");
        return;
    }

    if (strcmp(cmd, "STOP_WEIGHT") == 0) {
        weight_stream_active = 0;
        uartPrintf(0, "WEIGHT_STREAM_OFF\r\n");
        return;
    }

    if (cmd[0] != '\0') {
        uartPrintf(0, "ERR:UNKNOWN_CMD\r\n");
    }
}

void apMain(void)
{
    uartInit();

    uartPrintf(0, "Tare...\r\n");
    HX711_Tare(&hx1, 10);
    HX711_Tare(&hx2, 10);
    HX711_Tare(&hx3, 10);
    HX711_Tare(&hx4, 10);
    uartPrintf(0, "READY\r\n");

    while (1) {
        uint32_t now = HAL_GetTick();

        if (mq3_stream_active && (now - mq3_last_sample_time >= 500U)) {
            mq3_last_sample_time = now;
            send_mq3_response();
        }

        if (weight_stream_active && (now - weight_last_sample_time >= 1000U)) {
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
                    uartPrintf(0, "[CMD:%s]\r\n", rx_cmd);
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
