# ⚙️ safe-kick-stm32
Safe Kick - STM32 펌웨어 (HAL + UART + 센서 제어)

## 📁 폴더 구조

```text
safe-kick/
│
├── Core/
│   ├── Inc/                      # 헤더 파일
│   │   ├── main.h
│   │   ├── adc.h
│   │   ├── gpio.h
│   │   ├── hx711.h
│   │   ├── stm32f4xx_hal_conf.h
│   │   ├── stm32f4xx_it.h
│   │   └── usart.h
│   │
│   ├── Src/                      # 소스 파일
│   │   ├── main.c
│   │   ├── adc.c
│   │   ├── gpio.c
│   │   ├── hx711.c
│   │   ├── stm32f4xx_hal_msp.c
│   │   ├── stm32f4xx_it.c
│   │   ├── syscalls.c
│   │   ├── sysmem.c
│   │   ├── system_stm32f4xx.c
│   │   └── usart.c
│   │
│   └── Startup/
│       └── startup_stm32f411retx.s
│
├── Drivers/
│   ├── CMSIS/
│   │   ├── Device/
│   │   └── Include/
│   │
│   └── STM32F4xx_HAL_Driver/
│       ├── Inc/
│       └── Src/
│
├── MyApp/
│   ├── ap/
│   │   ├── ap.c
│   │   └── ap.h
│   │
│   ├── bsp/
│   │   ├── bps.c
│   │   └── bsp.h
│   │
│   ├── common/
│   │   └── hw_def.h
│   │
│   └── hw/
│       └── driver/
│           ├── hx711.c
│           ├── hx711.h
│           ├── led.c
│           ├── led.h
│           ├── mq3.c
│           ├── mq3.h
│           ├── uart.c
│           └── uart.h
│
├── .gitignore
├── README.md
├── README_UART_MQ3_WEIGHT.md
├── CMakeLists.txt
├── CMakePresets.json
├── safe-kick.ioc
├── roadsellEX.ioc
├── roadsellEX Debug.launch
├── STM32F411RETX_FLASH.ld
├── STM32F411RETX_RAM.ld
├── STM32F411xx_FLASH.ld
└── cmake/
    ├── gcc-arm-none-eabi.cmake
    └── starm-clang.cmake
```


# SAFEKICK UART / MQ3 / WEIGHT 정리

이 문서는 STM32F411 보드와 Raspberry Pi 사이 UART 통신, MQ3 알코올 센서, 4채널 무게 측정 동작을 정리한 설명서다.

## 전체 구조

- STM32는 센서값을 측정해서 UART로 전송한다.
- Raspberry Pi는 받은 센서값으로 최종 판단을 수행한다.
- STM32는 판정을 하지 않고 데이터 수집 역할만 담당한다.

## 센서 역할

- MQ3: 음주 여부 판단용 알코올 센서
- HX711 4개: 동승자 유무 확인용 무게 센서

## UART 기본 동작

- UART2 사용
- baudrate: 115200
- 입력은 줄바꿈 기준으로 명령을 처리한다.
- 터미널에서 직접 명령을 입력해서 테스트할 수 있다.

## MQ3 동작

### `CHECK_MQ3`

- 명령을 받으면 baseline 1회 전송
- baseline은 MQ3를 8회 측정한 평균값이다.
- 이후 MQ3 측정값 8회를 전송한다.
- 각 측정 사이 간격은 0.5초다.

### `TEST_MQ3`

- 명령을 받으면 MQ3 값을 0.5초 간격으로 계속 전송한다.
- 스트림을 멈추려면 `STOP_TEST_MQ3`를 보낸다.

### MQ3 전송 예시

```text
MQ3_BASELINE:627
MQ3:756
MQ3:674
MQ3:608
MQ3:656
MQ3:677
MQ3:685
MQ3:710
MQ3:652
```

### MQ3 관련 포인트

- MQ3 raw 값은 12비트 ADC 값이다.
- 센서 예열이 충분하지 않으면 값 변화가 작을 수 있다.
- 측정 환경이 개방되어 있으면 알코올 반응이 약할 수 있다.
- Raspberry Pi는 baseline과 현재값 차이로 판정하는 방식이 적합하다.

## 무게 동작

### `CHECK_WEIGHT`

- 명령을 받으면 무게 측정을 시작한다.
- 1초에 1회씩 계속 전송한다.
- 출력 형식은 아래와 같다.

```text
FL:0.00 FR:0.00 RL:0.00 RR:0.00 TOTAL:0.00
```

### `STOP_WEIGHT`

- 무게 스트림을 멈춘다.

## 현재 명령어 목록

- `CHECK_MQ3`
- `TEST_MQ3`
- `STOP_TEST_MQ3`
- `CHECK_WEIGHT`
- `STOP_WEIGHT`

## 현재 STM32 파일 역할

- `MyApp/ap/ap.c`: 명령 처리, 센서 전송 흐름
- `MyApp/hw/driver/mq3.c`: MQ3 ADC 읽기
- `MyApp/hw/driver/hx711.c`: 무게 센서 읽기
- `Core/Src/adc.c`: ADC1 초기화
- `Core/Src/usart.c`: UART2 초기화
- `Core/Src/stm32f4xx_it.c`: USART2 인터럽트 연결

## 테스트 방법

- UART 터미널을 연다.
- `CHECK_MQ3` 또는 `TEST_MQ3`를 입력한다.
- MQ3 응답을 확인한다.
- `CHECK_WEIGHT`를 입력해서 무게 스트림을 확인한다.
- 스트림을 멈출 때는 `STOP_TEST_MQ3`, `STOP_WEIGHT`를 보낸다.

## 판단 분리

- STM32: 센서값 측정과 UART 전송
- Raspberry Pi: 센서값 수신, baseline 비교, 음주/동승 판단
