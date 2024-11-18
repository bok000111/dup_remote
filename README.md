# dup_remote

ESP32-C3와 FreeRTOS 환경에서 RMT 모듈을 사용하여 가정용 선풍기 리모컨 신호를 분석하고 복제하는 프로젝트입니다.

## 주요 기능

- IR 신호 수신 및 분석: ESP32-C3의 RMT 모듈을 사용하여 리모컨의 IR 신호를 수신하고, 신호 패턴을 분석합니다.
- 프로토콜 분석: 팬 리모컨에서 사용하는 신호 패턴을 분석하여 각 기능을 구분하고 정의했습니다.
- 신호 분석 및 출력: 수신한 신호를 분석하여 LCD 디스플레이를 통해 출력합니다.
- FreeRTOS 멀티태스킹: LVGL, 수신, 분석 작업을 각각 독립적인 FreeRTOS 태스크로 분리하여 안정적인 동작을 보장합니다.
- TODO: 신호를 복제하여 선풍기를 제어하는 기능을 추가할 예정입니다.

## 프로토콜 정보

널리 사용되는 NEC, Sony 등의 IR 프로토콜을 예상하고 분석하였으나, 분석하기로 한 리모컨은 자체적인 프로토콜을 사용하고 있었습니다.

따라서 해당 리모컨의 프로토콜을 분석하고 정의하였습니다.

### 신호 구조

- RMT 신호 패턴:
  - `0` 신호: 400µs HIGH, 1300µs LOW
  - `1` 신호: 1300µs HIGH, 400µs LOW

- 프레임 구조:
  - 모든 신호는 `FRAME`으로 시작하며, `FRAME`은 `0b11011` (5비트)로 구성됩니다.
  - 이후 각 신호는 1비트로 기능을 나타냅니다.

### 분석한 명령어

| 명령어          | 신호 값      | 설명                          |
| --------------- | ------------ | ----------------------------- |
| `RMT_SIGNAL_OFF` | `0b1101100001` | 선풍기 끄기 |
| `RMT_SIGNAL_ON_SPEED` | `0b1101100010` | 선풍기 켜기 또는 속도 조절 |
| `RMT_SIGNAL_MODE` | `0b1101100100` | 선풍기 모드 변경(자연풍 등) |
| `RMT_SIGNAL_TIMER` | `0b1101101000` | 타이머 설정 |
| `RMT_SIGNAL_ROTATE` | `0b1101110000` | 팬 회전 기능 |

## 요구 사항

- ESP32-C3 모듈
- ESP-IDF v5.3.1 이상

## 설치 및 빌드

1. ESP-IDF 환경을 설정합니다. 자세한 내용은 [ESP-IDF 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)를 참고하세요.
2. 이 저장소를 클론합니다:

   ```bash
   git clone https://github.com/bok000111/esp_fan_remote.git
   ```

3. 프로젝트 디렉토리로 이동합니다:

   ```bash
   cd esp_fan_remote
   ```

4. 필요한 서브모듈을 초기화하고 업데이트합니다:

   ```bash
   git submodule update --init --recursive
   ```

5. 프로젝트를 빌드하고 플래시합니다:

   ```bash
   idf.py build
   idf.py flash
   ```

6. 시리얼 모니터를 통해 장치의 출력을 확인합니다:

   ```bash
   idf.py monitor
   ```
