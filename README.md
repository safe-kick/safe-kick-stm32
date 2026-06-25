# ⚙️ safe-kick-stm32
Safe Kick - STM32 펌웨어 (HAL + UART)

## 📁 폴더 구조

```
safe-kick-stm32/
│
├── Core/
│   ├── Inc/                    # 헤더 파일
│   │   ├── main.h
│   │   ├── uart.h              # UART 통신
│   │   ├── adc.h               # ADC (MQ-3 가스 센서, 무게 센서)
│   │   └── gpio.h              # GPIO (릴레이 락/언락 제어)
│   │
│   └── Src/                    # 소스 파일
│       ├── main.c
│       ├── uart.c              # 라즈베리파이 UART 송수신
│       ├── adc.c               # 센서값 읽기
│       └── gpio.c              # 릴레이 제어
│
├── Drivers/                    # STM32 HAL 드라이버 (자동 생성)
│
├── .gitignore
└── README.md
```
