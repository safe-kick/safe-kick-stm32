# safe-kick-stm32

Safe Kick의 STM32F411RE 펌웨어 프로젝트다. STM32는 MQ-3 알코올 센서와 4개의 로드셀을 측정하고, Raspberry Pi와 USART2로 통신하며 릴레이·부저·L298N 모터 PWM을 제어한다.

## 주요 기능

- MQ-3 baseline 3.5초 부저 안내와 버튼 입력 후 실측 8회 분리
- HX711 4채널 무게 측정, 앞뒤 하중 기반 PWM 제어 및 1초 주기 스트림
- UART 명령 수신과 키보드 입력 echo
- 릴레이 잠금 및 잠금 해제 제어
- 무게 측정과 동시에 동작하는 비차단 부저 경고

## 하드웨어

### 구성 부품과 역할

| 장치 | 수량 | 역할 | STM32 인터페이스 |
|---|---:|---|---|
| NUCLEO-F411RE | 1 | 센서 수집, UART 통신, 안전 출력 제어 | 3.3V GPIO, ADC1, TIM1, USART2 |
| MQ-3 센서 모듈 | 1 | 호흡 전후 알코올 반응을 아날로그 원시값으로 출력 | ADC1 IN1 |
| 로드셀 + HX711 | 4세트 | 발판 네 모서리 하중을 kg 단위로 측정 | 채널별 DT 1개, SCK 1개 |
| 릴레이 모듈 | 1 | 구동계 전원 허용·차단 | Active-High GPIO 출력 |
| 부저 모듈 | 1 | MQ-3 안내음과 추가 탑승 경고음 | Active-High GPIO 출력 |
| L298N 모터 드라이버 | 1 | DC 모터 방향 및 PWM 출력 제어 | ENA PWM, IN1/IN2 GPIO |
| DC 모터 | 1 | 킥보드 구동 시험 | L298N OUT 단자 |

수량과 역할은 현재 펌웨어 구성을 기준으로 한다. 실제 센서 모듈의 정격전압과
단자 배열은 제품마다 다를 수 있으므로 실물 모듈 표기와 데이터시트를 우선한다.

### STM32 신호 핀

| 장치 | 핀 |
|---|---|
| USART2 TX | PA2 |
| USART2 RX | PA3 |
| MQ-3 ADC | PA1 |
| BUZZER | PA6 |
| RELAY | PA7 |
| L298N ENA (PWM) | PA8 / TIM1 CH1 |
| L298N IN1 / IN2 | PB4 / PB5 |
| HX711 FL DT / SCK | PB0 / PB1 |
| HX711 FR DT / SCK | PB2 / PB10 |
| HX711 RL DT / SCK | PB12 / PB13 |
| HX711 RR DT / SCK | PB14 / PB15 |

### 신호 연결 개요

```text
Raspberry Pi TX  ───────────────> PA3  STM32 USART2 RX
Raspberry Pi RX  <─────────────── PA2  STM32 USART2 TX
Raspberry Pi GND ──────────────── GND  STM32

MQ-3 AO ── 입력전압 보호 ───────> PA1  ADC1 IN1

로드셀 FL ─> HX711 DT/SCK ──────> PB0 / PB1
로드셀 FR ─> HX711 DT/SCK ──────> PB2 / PB10
로드셀 RL ─> HX711 DT/SCK ──────> PB12 / PB13
로드셀 RR ─> HX711 DT/SCK ──────> PB14 / PB15

PA6 ────────────────────────────> 부저 모듈 입력
PA7 ────────────────────────────> 릴레이 모듈 입력
PA8 / PB4 / PB5 ────────────────> L298N ENA / IN1 / IN2 ─> DC 모터
```

이 그림은 펌웨어 신호 연결을 나타낸다. 센서·릴레이·L298N의 전원 단자와 릴레이
COM/NO/NC 접점은 사용 중인 실제 모듈의 정격 및 시스템 전원 회로에 맞춰 연결한다.

### 연결 원칙

- Raspberry Pi TX는 STM32 `PA3(USART2 RX)`, Raspberry Pi RX는
  `PA2(USART2 TX)`에 교차 연결하고 두 장치의 GND를 연결한다.
- USART2는 3.3V TTL 신호다. RS-232 전압이나 5V UART 신호를 직접 연결하지 않는다.
- `PA1`은 12비트 ADC 입력이며 펌웨어가 사용하는 값은 `0~4095` 범위의 원시값이다.
  ppm이나 혈중알코올농도로 직접 환산한 값이 아니다.
- MQ-3 모듈의 `AO`가 STM32의 허용 ADC 입력전압을 넘을 수 있으면 분압 또는
  레벨 보호 회로를 사용한다. 센서 히터 전원은 사용 중인 모듈의 정격을 따른다.
- HX711 네 개는 각각 로드셀의 `E+`, `E-`, `A+`, `A-`를 입력받는다. DT와 SCK만
  STM32에 연결되며, 로드셀 선 색상만으로 단자를 단정하지 않는다.
- 릴레이 코일, 부저, 모터를 STM32 GPIO에서 직접 구동하지 않는다. 정격에 맞는
  모듈 또는 드라이버를 사용하고, 비절연 제어 회로는 STM32와 공통 GND를 구성한다.
- 모터 전원은 NUCLEO 보드의 3.3V/5V 핀에서 공급하지 않는다. L298N과 모터 정격에
  맞는 별도 전원을 사용한다.

### 장치별 펌웨어 동작

#### MQ-3

- `PA1 / ADC1_IN1`, 12비트, software trigger, right alignment로 1회 변환한다.
- baseline과 실측은 각각 8개 표본을 500ms 간격으로 수집한다.
- STM32는 원시값을 전송하고, baseline 대비 증가량과 절대 임계값을 이용한 최종
  음주 판정은 Raspberry Pi가 수행한다.
- 센서 예열과 안정화 시간은 장착한 MQ-3 모듈의 데이터시트 및 실제 시험 조건에
  맞춰 운용한다.

#### 로드셀과 HX711

- FL, FR, RL, RR 네 위치를 독립적으로 측정하고 합계 `TOTAL`을 계산한다.
- 부팅 때 무부하 상태를 채널별로 10회 측정해 tare 값을 저장한다.
- 내부 하중 제어는 200ms 주기, UART 무게 출력은 1000ms 주기다.
- 설치 위치가 바뀌면 각 채널의 scale 값을 다시 보정해야 한다. 자세한 계산식과
  현재 scale 값은 [README_SERIAL_TEST.md](README_SERIAL_TEST.md#로드셀-측정-주기-및-보정)를 참고한다.

#### 릴레이와 부저

- `PA7` 릴레이는 Active-High로 구현되어 있으며 부팅과 `LOCK`에서 OFF가 된다.
  사용한 모듈이 Active-Low이면 `relay.c`의 ON/OFF 상태를 반대로 설정해야 한다.
- `PA6` 부저도 Active-High다. MQ-3 baseline 중에는 연속으로 켜지고, 추가 탑승
  경고에서는 1초 ON/1초 OFF를 비차단 방식으로 반복한다.

#### L298N과 DC 모터

- `PA8 / TIM1 CH1`이 ENA PWM, `PB4=HIGH`와 `PB5=LOW`가 전진 방향이다.
  두 방향 입력이 모두 LOW이면 coast 상태가 된다.
- TIM1 입력 84MHz, prescaler 71, period 999 설정으로 PWM 주파수는 약 1.17kHz다.
- L298N 모듈에 ENA 점퍼가 있으면 외부 PWM을 사용하도록 점퍼 상태를 확인한다.
- `UNLOCK` 직후에는 릴레이만 켜고 PWM 0%를 유지한다. 전방 하중 조건을 만족하면
  30%에서 시작해 30~70% 범위에서 제어한다.
- 최초 전방 하중 확정 시 PWM 30%를 즉시 적용한다. 이후 실제 출력은 25ms마다
  1%씩 목표값을 따라가며, 앞·뒤 하중에 따른 목표값 변경은 최대 500ms에 한 번
  5%씩 수행한다. `BUZZ_ON` 중에는 목표 PWM을 최대 30%로 제한한다.

### 전원 및 안전 주의사항

- 배선을 변경할 때는 NUCLEO, 센서 전원, 모터 전원을 모두 차단한다.
- 모터 시험 전 킥보드를 고정하고 구동 바퀴를 지면에서 띄운다.
- 센서·로직 전원과 모터 전원 경로를 분리하고, 모터 전원선에는 시스템 정격에 맞는
  퓨즈·비상 차단 수단을 둔다.
- 최초 시험은 모터를 분리한 상태에서 UART, 센서, 부저, 릴레이 논리를 확인한 뒤
  L298N과 모터를 연결하는 순서로 진행한다.
- `LOCK`은 PWM 0%, L298N coast, 릴레이 OFF, 부저 OFF를 즉시 적용한다.

## 폴더 구조

```text
safe-kick/
├── Core/
│   ├── Inc/                    # CubeMX 생성 헤더
│   ├── Src/                    # 초기화 및 인터럽트 코드
│   └── Startup/                # STM32 시작 코드
├── Drivers/                    # STM32 HAL 및 CMSIS 드라이버
├── docs/images/                # 실제 하드웨어 개별 시험 캡처
├── MyApp/
│   ├── ap/
│   │   └── ap.c                # UART 명령 처리와 메인 루프
│   ├── bsp/                    # delay 같은 보드 공통 함수
│   ├── common/                 # 공통 타입과 hw_def.h
│   └── hw/driver/
│       ├── hx711.c             # HX711 로드셀 읽기와 영점 보정
│       ├── mq3.c               # MQ-3 ADC 읽기와 평균값 계산
│       ├── uart.c              # UART2 인터럽트 송수신
│       ├── relay.c             # 릴레이 ON/OFF
│       ├── motor_control.c     # 릴레이, 방향, PWM 및 속도 제한 제어
│       └── buzzer.c            # 부저 ON/OFF와 토글 제어
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
| `CHECK_MQ3_BASELINE` | 부저를 연속으로 켜고 baseline을 500ms 간격으로 8회 측정한 뒤 부저 종료(총 3.5초) |
| `CHECK_MQ3_MEASURE` | `MEASURE_BEGIN` 후 첫 표본을 즉시 보내고 500ms 간격으로 총 8회 측정 |
| `CHECK_MQ3` | 기존 도구 호환용 통합 세션 |
| `TEST_MQ3` | `MQ3_STREAM_ON` 출력 후 500ms 주기 MQ-3 연속 측정 시작 |
| `STOP_TEST_MQ3` | `MQ3_STREAM_OFF` 출력 후 MQ-3 연속 측정 종료 |
| `CHECK_WEIGHT` | `[CHECK_WEIGHT]` 출력 후 1초 주기 무게 스트림 시작 |
| `STOP_WEIGHT` | 운행 중이면 즉시 잠근 뒤 `[END_WEIGHT]`, `WEIGHT_STREAM_OFF` 출력 |
| `BUZZ_ON` | 부저 경고 시작, 현재 출력을 올리지 않고 최대 30%로 제한 |
| `BUZZ_OFF` | 부저 경고 종료, 30% 출력 제한 해제 |
| `UNLOCK` | 무게 제어 자동 시작, 릴레이 ON, PWM 0% 유지(앞쪽 하중 70% 감지 시 30%로 기동) |
| `LOCK` | 모터 PWM 0%, 릴레이와 부저 OFF, 무게 스트림 종료 |
| `MOTOR_STATE` | 현재 잠금 상태와 PWM 속도(%) 출력 |

시리얼 테스트 방법, 출력 형식, 로드셀 계산과 보정 설명은 [README_SERIAL_TEST.md](README_SERIAL_TEST.md)를 참고한다.

## 역할 분리

- STM32: 센서 측정, UART 전송, 릴레이·부저·모터 PWM 제어
- Raspberry Pi: MQ-3 판정, 탑승 인원 판단, 앱 연동 및 STM32 명령 전송

## 빌드

STM32Cube가 설치한 CMake, Ninja, ARM GCC가 일반 터미널의 `PATH`에 없어도
macOS에서는 빌드 스크립트가 Cube 번들 경로를 자동으로 찾는다.

```bash
./scripts/build.sh          # Debug
./scripts/build.sh Release
```

빌드 결과는 각각 `build/Debug/safe-kick.elf`와
`build/Release/safe-kick.elf`에 생성된다. 다른 환경에서는 `cmake`, `ninja`,
`arm-none-eabi-gcc`를 `PATH`에 추가한 뒤 같은 스크립트를 실행한다.
