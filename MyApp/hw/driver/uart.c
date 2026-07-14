#include "uart.h"

/*
 * 함수 정리:
 * - uartInit(): UART 수신 인터럽트 시작
 * - HAL_UART_RxCpltCallback(): 수신 데이터 버퍼 저장
 * - uartAvailable(): 읽을 수 있는 데이터 개수 확인
 * - uartOpen(): UART 포트 오픈 처리
 * - uartWrite(): UART 송신
 * - uartPrintf(): printf 스타일 송신
 * - uartRead(): 1바이트 읽기
 */
extern UART_HandleTypeDef huart2;
#define UART_RX_BUF_LENGTH 256

static uint8_t rx_buf[UART_RX_BUF_LENGTH];
static volatile uint32_t rx_buf_head =0;
static volatile uint32_t rx_buf_tail=0;
static uint8_t rx_data;


bool uartInit(void){
    HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    return true;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART2){
        rx_buf[rx_buf_head] =rx_data;
        rx_buf_head = (rx_buf_head+1)%UART_RX_BUF_LENGTH;

        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}

uint32_t uartAvailable(uint8_t ch){
    if (rx_buf_head >= rx_buf_tail) {
        return rx_buf_head - rx_buf_tail;
    }

    return UART_RX_BUF_LENGTH - rx_buf_tail + rx_buf_head;
}


bool uartOpen(uint8_t ch,uint32_t baud){
    return true;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data,uint32_t length){
    if (HAL_UART_Transmit(&huart2, p_data, length, 100) == HAL_OK) return length;
   
    return 0;
}


uint32_t uartPrintf(uint8_t ch, char* fmt,...){
    char buf[256];
    va_list args;
    int len;
    va_start(args, fmt);
    len = vsnprintf(buf, 256, fmt, args);
    va_end(args);
    
    return uartWrite(ch, (uint8_t *)buf, len);
}

uint32_t uartRead(uint8_t ch){
    uint8_t ret = 0;
    if(uartAvailable(ch)>0){
        ret = rx_buf[rx_buf_tail];
        rx_buf_tail = (rx_buf_tail+1)%UART_RX_BUF_LENGTH;
    }

    return ret;
}
