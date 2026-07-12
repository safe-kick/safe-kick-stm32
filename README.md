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
