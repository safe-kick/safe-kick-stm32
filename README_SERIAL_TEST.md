# STM32 시리얼 통신 테스트

Raspberry Pi 없이 PC 시리얼 터미널에서 STM32의 센서, 릴레이, 부저를 테스트하는 방법이다.

## 시리얼 설정

| 항목 | 설정값 |
|---|---|
| UART | USART2 |
| Baud rate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |
| Line ending | CR+LF 또는 Enter |

보드가 정상적으로 시작되면 다음 메시지가 출력된다.

```text
Tare...
READY
```

## 핀 설정

| 장치 | STM32 핀 | 동작 |
|---|---|---|
| 릴레이 | PA7 | GPIO Output |
| 부저 | PA6 | GPIO Output |

## 명령어

### `CHECK_MQ3_BASELINE`

부저를 연속으로 켠 상태에서 주변 공기 baseline을 8회, 500ms 간격으로
측정한다. 첫 측정부터 마지막 측정까지 총 3.5초이며, 완료 직후 부저를 끈다.

```text
[CHECK_MQ3_BASELINE]
MQ3_BASELINE:627
[END_MQ3_BASELINE]
```

### `CHECK_MQ3_MEASURE`

명령을 받으면 추가 안내음 지연 없이 `MEASURE_BEGIN`을 보내고, MQ-3 실측값
8개를 500ms 간격으로 전송한다.

```text
[CHECK_MQ3_MEASURE]
MEASURE_BEGIN
MQ3:756
MQ3:674
MQ3:608
MQ3:656
MQ3:677
MQ3:685
MQ3:710
MQ3:652
MEASURE_END
[END_MQ3_MEASURE]
```

### `CHECK_MQ3` (호환용)

기존 시리얼 도구를 위해 유지한다. baseline과 실측을 연속 실행하지만 새 운용
코드에서는 분리된 두 명령을 사용한다.

### `TEST_MQ3` (시리얼 테스트 전용)

PC 시리얼 터미널에서 MQ3 센서를 점검할 때 사용한다. `MQ3_STREAM_ON`을 출력한 뒤 MQ3 값을 500ms 간격으로 계속 전송한다. 라즈베리파이 실제 운용 로직에서는 사용하지 않는다.

### `STOP_TEST_MQ3` (시리얼 테스트 전용)

PC 시리얼 터미널에서 시작한 MQ3 연속 측정을 종료할 때 사용한다. `MQ3_STREAM_OFF`를 출력하고 MQ3 연속 전송을 중단한다. 라즈베리파이 실제 운용 로직에서는 사용하지 않는다.

### `CHECK_WEIGHT`

`[CHECK_WEIGHT]`를 출력하고 무게 측정을 시작한다. 이후 1000ms 간격으로 계속 전송한다.

```text
[CHECK_WEIGHT]
FL:-0.00 FR:0.01 RL:-0.00 RR:-0.00 TOTAL:0.01
FL:-0.00 FR:0.01 RL:-0.00 RR:-0.00 TOTAL:0.01
FL:-0.00 FR:0.01 RL:-0.00 RR:-0.00 TOTAL:0.00
```

### `STOP_WEIGHT` (시리얼 테스트 전용)

PC 시리얼 터미널에서 무게 스트림을 종료할 때 사용한다. 무게 연속 전송을 중단하고 `[END_WEIGHT]` 다음 `WEIGHT_STREAM_OFF`를 출력한다. 라즈베리파이 실제 운용 로직에서는 사용하지 않는다.

```text
[END_WEIGHT]
WEIGHT_STREAM_OFF
```

### `BUZZ_ON`

부저 경고를 시작한다. 부저는 1초 동안 울리고 1초 동안 멈추는 동작을 반복한다. 모터 출력은 현재보다 증가하지 않으며 최대 30%로 제한된다.

부저가 동작하는 동안에도 무게 측정과 UART 명령 처리는 계속된다.

### `BUZZ_OFF`

부저 경고를 즉시 중단하고 30% 출력 제한을 해제한다. 출력은 자동으로 70%로 복구되지 않고 발판 하중 제어를 다시 따른다.

### `LOCK`

부저를 끄고, 릴레이 출력을 OFF로 변경한 뒤 무게 스트림이 켜져 있으면 `[END_WEIGHT]`를 출력하고 `LOCK_OK`를 출력한다.

```text
[END_WEIGHT]
LOCK_OK
```

무게 스트림이 실행 중이 아니면 `[END_WEIGHT]` 없이 `LOCK_OK`만 출력된다.

### `UNLOCK`

릴레이 출력을 ON으로 변경하되 모터 PWM은 0%로 유지한다. 무게 스트림과 부저 상태는 변경하지 않는다. 이후 앞쪽 하중이 70% 이상으로 확인되면 모터가 30%에서 기동한다.

```text
UNLOCK_OK
```

## 권장 테스트 순서

1. 보드 리셋 후 `READY` 출력을 확인한다.
2. `CHECK_MQ3_BASELINE`을 입력하고 부저가 3.5초간 켜진 뒤 baseline 응답과 함께 꺼지는지 확인한다.
3. `CHECK_MQ3_MEASURE`를 입력하고 즉시 `MEASURE_BEGIN`과 MQ3 표본 8개가 오는지 확인한다.
4. `CHECK_WEIGHT`를 입력하고 무게값이 1000ms 간격으로 출력되는지 확인한다.
5. 무게 스트림이 실행되는 동안 `BUZZ_ON`을 입력한다.
6. 무게값 출력과 부저 경고가 동시에 동작하는지 확인한다.
7. `BUZZ_OFF`를 입력해 부저가 즉시 멈추는지 확인한다.
8. `UNLOCK`을 입력해 릴레이가 ON으로 전환되는지 확인한다.
9. `LOCK`을 입력해 부저와 릴레이가 OFF되고 무게 스트림이 종료되는지 확인한다.

## 릴레이 주의사항

현재 펌웨어는 시작할 때 릴레이를 OFF 상태로 초기화한다. `UNLOCK`과 `LOCK`을 차례대로 입력해 릴레이 접점 변화를 확인할 수 있다.

릴레이 모듈이 LOW 신호에서 켜지는 Active-Low 제품이면 `relay.c`의 ON/OFF GPIO 상태를 반대로 설정해야 한다.

## 로드셀 측정 주기 및 보정

- 각 로드셀은 내부 제어를 위해 200ms 간격으로 1회씩 측정한다.
- 가장 최근 무게값은 기존과 동일하게 1000ms 간격으로 전송한다.
- 앞쪽 합계가 `TOTAL`의 70% 이상이면 정지 상태에서는 목표 PWM을 30%로 설정하고, 주행 중에는 5% 올린다. 뒤쪽 합계가 70% 이상이면 최저 30%까지 5% 내린다. 변경은 최대 1초에 한 번 적용한다.
- `TOTAL`이 1kg 미만이면 릴레이를 유지한 채 PWM만 즉시 0%로 만든다.
- 현재 적용된 HX711 scale 값은 아래와 같다.

| 로드셀 | scale |
|---|---:|
| FL | 83387 |
| FR | 77144 |
| RL | 82693 |
| RR | 85663 |

실제 장착 상태나 하중 위치가 바뀌면 다시 보정해야 한다.

## 로드셀 무게 측정 동작 설명

### 구성

발판의 네 모서리에 로드셀과 HX711 모듈이 하나씩 연결되어 있다.

| 출력 이름 | 위치 | HX711 데이터 핀 | HX711 클럭 핀 |
|---|---|---|---|
| FL | 앞쪽 왼쪽(Front Left) | PB0 | PB1 |
| FR | 앞쪽 오른쪽(Front Right) | PB2 | PB10 |
| RL | 뒤쪽 왼쪽(Rear Left) | PB12 | PB13 |
| RR | 뒤쪽 오른쪽(Rear Right) | PB14 | PB15 |

각 로드셀의 측정값은 하중이 놓인 위치에 따라 서로 다를 수 있다. 사람이나 물체의 전체 무게는 네 값을 더한 `TOTAL`을 사용한다.

```text
TOTAL = FL + FR + RL + RR
```

### 부팅 시 영점 조정

STM32가 시작되면 각 로드셀을 10회씩 측정해 영점값을 저장한다.

```text
Tare...
READY
```

`Tare...`가 출력되는 동안에는 발판 위에 아무것도 올리지 않아야 한다. 하중이 올라간 상태에서 전원을 켜거나 리셋하면 해당 무게가 영점으로 저장되어 이후 측정값이 잘못된다. `READY`가 출력된 후 측정을 시작한다.

### 무게 계산

HX711에서 읽은 원시값에서 부팅 시 저장한 영점값을 뺀 뒤, 로드셀별 scale 값으로 나누어 kg 단위로 변환한다.

```text
무게(kg) = (HX711 원시값 - 영점값) / scale
```

로드셀마다 감도와 설치 상태가 다르기 때문에 FL, FR, RL, RR은 서로 다른 scale 값을 사용한다.

### `CHECK_WEIGHT` 처리 순서

1. STM32가 UART로 `CHECK_WEIGHT` 명령을 받는다.
2. `[CHECK_WEIGHT]`를 전송하고 무게 스트림을 활성화한다.
3. 200ms마다 FL, FR, RL, RR을 각각 1회 측정하고 하중 기반 모터 제어에 사용한다.
4. 네 측정값을 더해 `TOTAL`을 계산한다.
5. 가장 최근 값을 1000ms마다 한 줄의 고정된 형식으로 Raspberry Pi 또는 시리얼 터미널에 전송한다.
6. `STOP_WEIGHT` 또는 `LOCK` 명령을 받을 때까지 반복한다.

```text
FL:12.30 FR:13.10 RL:11.80 RR:12.90 TOTAL:50.10
```

Raspberry Pi에서는 공백으로 필드를 나눈 뒤 각 `이름:값` 형식을 파싱할 수 있다. 모든 숫자의 단위는 kg이다.

### 측정값 해석

- `FL`, `FR`, `RL`, `RR` 값이 서로 다른 것은 하중 위치에 따른 정상적인 현상이다.
- 사람의 전체 무게나 탑승 인원 판정에는 개별 값보다 `TOTAL`을 사용한다.
- 빈 발판에서 `-0.01`, `0.00`, `0.01`처럼 작은 값이 나타나는 것은 센서 노이즈와 영점 오차 때문이다.
- 필요하면 Raspberry Pi에서 `TOTAL`의 절댓값이 작은 구간을 0kg으로 처리하는 deadband를 적용할 수 있다.
- STM32는 무게를 측정해 전송하며, 1명 또는 2명 탑승 여부의 최종 판정은 Raspberry Pi에서 수행한다.

### 측정 중 명령 동작

무게 스트림이 실행되는 동안에도 UART 수신과 부저 제어는 계속 동작한다.

| 수신 명령 | 무게 스트림 동작 |
|---|---|
| `BUZZ_ON` | 무게 측정을 유지하며 부저 경고 시작 |
| `BUZZ_OFF` | 무게 측정을 유지하며 부저 정지 |
| `UNLOCK` | 무게 측정을 유지하며 릴레이 ON |
| `STOP_WEIGHT` | 무게 스트림 종료 |
| `LOCK` | 부저와 릴레이를 OFF하고 무게 스트림 종료 |

## 라즈베리파이 로직 참고

STM32는 센서값과 상태 메시지만 보내고, 라즈베리파이가 최종 판단과 제어 명령을 보낸다.

### 통신 요약

| RPi -> STM32 | STM32 -> RPi | 용도 |
|---|---|---|
| `CHECK_MQ3_BASELINE` | `[CHECK_MQ3_BASELINE]`, `MQ3_BASELINE:...`, `[END_MQ3_BASELINE]` | 3.5초 연속 부저 MQ-3 baseline 세션 |
| `CHECK_MQ3_MEASURE` | `[CHECK_MQ3_MEASURE]`, `MEASURE_BEGIN`, `MQ3:...`, `MEASURE_END` | MQ-3 실측 세션 |
| `CHECK_MQ3` | 기존 통합 응답 | 호환용 통합 세션 |
| `CHECK_WEIGHT` | `[CHECK_WEIGHT]`, `FL:... FR:... RL:... RR:... TOTAL:...` | 무게 스트림 시작 |
| `BUZZ_ON` | - | 부저 경고 시작 |
| `BUZZ_OFF` | - | 부저 경고 정지 |
| `UNLOCK` | `UNLOCK_OK` | 릴레이 ON |
| `LOCK` | `[END_WEIGHT]`(무게 스트림 중일 때만), `LOCK_OK` | 릴레이 OFF, 부저 OFF, 무게 스트림 종료 |

### 권장 상태 처리

- 무게 스트림을 먼저 시작하려면 `CHECK_WEIGHT`를 보낸다.
- RPi는 `UNLOCK_OK` 5초 후 첫 유효 `TOTAL`을 탑승자 기준 무게로 저장한다.
- 기준보다 30kg 증가한 값이 2회 연속 수신되면 `BUZZ_ON`을 보낸다.
- `BUZZ_ON` 이후에도 5초 동안 2인 상태가 유지되면 `LOCK`을 보낸다.
- 무게가 정상 범위로 돌아오면 `BUZZ_OFF`를 보낸다.
- STM32도 같은 기준 증가량을 감지하면 앞뒤 하중 기반 PWM 증감을 중단한다.
- 1kg 미만에서는 PWM만 0%로 만들며 릴레이, 세션, 무게 스트림은 유지한다.
- `LOCK`을 보내면 STM32는 릴레이와 부저를 끄고 무게 스트림도 종료한다.
- 앱에서 회원 인증이 완료되면 Raspberry Pi가 STM32에 `UNLOCK`을 보내고, STM32는 릴레이를 ON으로 전환해 킥보드를 주행 가능한 상태로 만든다.

### 구현할 때 기억할 점

- 새 운용 흐름은 `CHECK_MQ3_BASELINE`과 `CHECK_MQ3_MEASURE`를 분리해 사용한다.
- `MEASURE_BEGIN`은 `CHECK_MQ3_MEASURE` 수신 직후 전송한다.
- `CHECK_WEIGHT`는 1000ms 간격으로 `FL`, `FR`, `RL`, `RR`, `TOTAL`이 나온다.
- `LOCK`은 스트림을 유지하는 명령이 아니라, 현재 코드에서는 무게 스트림까지 같이 종료한다.
- `BUZZ_ON`과 `BUZZ_OFF`는 라즈베리파이가 판단해서 보내는 제어 명령이다.
- `TEST_MQ3`, `STOP_TEST_MQ3`, `STOP_WEIGHT`는 PC 시리얼 테스트 전용이므로 라즈베리파이 운용 로직에 넣지 않는다.
- 판단 기준은 `TOTAL` 하나로 처리해도 되고, 앱 정책에 맞게 개별 로드셀까지 같이 볼 수 있다.
