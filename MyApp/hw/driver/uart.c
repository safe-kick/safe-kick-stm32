#include "uart.h"

/*
 * 함수 정리:
 * - uartInit(): UART 수신 인터럽트 시작
 * - HAL_UART_RxCpltCallback(): 수신 데이터 버퍼 저장
 * - uartAvailable(): 읽을 수 있는 데이터 개수 확인
 * - uartWrite(): UART 송신
 * - uartPrintf(): printf 스타일 송신
 * - uartRead(): 1바이트 읽기
 */
extern UART_HandleTypeDef huart2;
#define UART_RX_BUF_LENGTH 256

/* 인터럽트는 head에 쓰고 메인 루프는 tail에서 읽는 원형 버퍼다. */
static uint8_t rx_buf[UART_RX_BUF_LENGTH];
static volatile uint32_t rx_buf_head =0;
static volatile uint32_t rx_buf_tail=0;
static uint8_t rx_data;


bool uartInit(void){
    /* 첫 1바이트 수신을 등록하고 이후 수신은 callback에서 계속 재등록한다. */
    HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    return true;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART2){
        rx_buf[rx_buf_head] =rx_data;
        rx_buf_head = (rx_buf_head+1)%UART_RX_BUF_LENGTH;

        /* 다음 바이트를 놓치지 않도록 즉시 RX 인터럽트를 다시 건다. */
        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}

uint32_t uartAvailable(uint8_t ch){
    /* head/tail의 wrap-around를 고려해 대기 중인 바이트 수를 계산한다. */
    if (rx_buf_head >= rx_buf_tail) {
        return rx_buf_head - rx_buf_tail;
    }

    return UART_RX_BUF_LENGTH - rx_buf_tail + rx_buf_head;
}


uint32_t uartWrite(uint8_t ch, uint8_t *p_data,uint32_t length){
    if (HAL_UART_Transmit(&huart2, p_data, length, 100) == HAL_OK) return length;
   
    return 0;
}


uint32_t uartPrintf(uint8_t ch, char* fmt,...){
    /* 센서 출력 한 줄이 256바이트를 넘지 않는다는 전제로 사용한다. */
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
