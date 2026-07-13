# safe-kick-stm32

Safe Kick의 STM32F411RE 펌웨어 프로젝트다. STM32는 MQ-3 알코올 센서와 4개의 로드셀을 측정하고, Raspberry Pi와 USART2로 통신하며 릴레이와 부저를 제어한다.

## 주요 기능

- MQ-3 ADC 측정 및 baseline 전송
- HX711 4채널 무게 측정 및 1초 주기 스트림
- UART 명령 수신과 키보드 입력 echo
- 릴레이 잠금 및 잠금 해제 제어
- 무게 측정과 동시에 동작하는 비차단 부저 경고

## 하드웨어

| 장치 | 핀 |
|---|---|
| USART2 TX | PA2 |
| USART2 RX | PA3 |
| MQ-3 ADC | PA1 |
| BUZZER | PA6 |
| RELAY | PA7 |
| HX711 FL DT / SCK | PB0 / PB1 |
| HX711 FR DT / SCK | PB2 / PB10 |
| HX711 RL DT / SCK | PB12 / PB13 |
| HX711 RR DT / SCK | PB14 / PB15 |

## 폴더 구조

```text
safe-kick/
├── Core/
│   ├── Inc/                    # CubeMX 생성 헤더
│   ├── Src/                    # 초기화 및 인터럽트 코드
│   └── Startup/                # STM32 시작 코드
├── Drivers/                    # STM32 HAL 및 CMSIS 드라이버
├── MyApp/
│   ├── ap/
│   │   └── ap.c                # 명령 처리와 애플리케이션 루프
│   ├── bsp/                    # 보드 공통 기능
│   ├── common/                 # 공통 타입과 HAL 헤더
│   └── hw/driver/
│       ├── hx711.c             # 로드셀 측정
│       ├── mq3.c               # MQ-3 ADC 측정
│       ├── uart.c              # UART 인터럽트 송수신
│       ├── relay.c             # 릴레이 제어
│       └── buzzer.c            # 비차단 부저 제어
├── cmake/                      # STM32 CMake 설정
├── safe-kick.ioc               # STM32CubeMX 프로젝트
├── README_SERIAL_TEST.md       # UART 명령 및 하드웨어 테스트 설명
├── CMakeLists.txt
└── CMakePresets.json
```

## UART 명령

USART2는 `115200-8-N-1`, flow control 없음으로 사용한다.

| 명령 | 동작 |
|---|---|
| `CHECK_MQ3` | MQ-3 baseline과 측정값 전송 |
| `TEST_MQ3` | MQ-3 연속 측정 시작 |
| `STOP_TEST_MQ3` | MQ-3 연속 측정 종료 |
| `CHECK_WEIGHT` | 1초 주기 무게 스트림 시작 |
| `STOP_WEIGHT` | 무게 스트림 종료 |
| `BUZZ_ON` | 1초 ON / 1초 OFF 부저 경고 시작 |
| `BUZZ_OFF` | 부저 경고 종료 |
| `UNLOCK` | 릴레이 ON |
| `LOCK` | 부저와 릴레이 OFF, 무게 스트림 종료 |

시리얼 테스트 방법, 출력 형식, 로드셀 계산과 보정 설명은 [README_SERIAL_TEST.md](README_SERIAL_TEST.md)를 참고한다.

## 역할 분리

- STM32: 센서 측정, UART 전송, 릴레이 및 부저 제어
- Raspberry Pi: MQ-3 판정, 탑승 인원 판단, 앱 연동 및 STM32 명령 전송
