# 무인 스마트 스토어 시스템 (Smart Store System)

무인 매장 관리를 위한 IoT 기반 스마트 스토어 시스템입니다. STM32와 Arduino를 기반으로 다양한 센서들을 활용하여 매장의 조명, 환기, 출입 등을 원격 자동 제어합니다.

해당 프로젝트의 목적은 무인 매장의 효율성을 높이고 사업주의 편의성을 높이는 것입니다.

## 📋 프로젝트 개요

- **프로젝트 유형**: 미니 프로젝트
- **주요 기술**: STM32, Arduino, C언어, TCP/IP 통신, Bluetooth 통신
- **주요 기능**: 자동 출입문, 조명 제어, 온습도 관리, 환기 시스템, 원격 모니터링

## 🎥 데모 영상

- [A지점 구현 영상 - STM32](asset/A지점%20구현%20영상%20-%20stm32.mp4)
- [경비실 구현 영상 - Arduino Uno](asset/경비실%20구현%20영상%20-%20아두이노%20우노.mp4)
- [앱 원격제어 구현 영상](asset/어플%20원격제어%20구현%20영상.mp4)

## 🏗️ 시스템 구성

```
┌─────────────┐      TCP/IP      ┌─────────────┐      Bluetooth    ┌─────────────┐
│   STM32     │ ←────────────────→│   Server    │ ←──────────────→ │  Arduino    │
│  (매장 제어)  │   WiFi(ESP-01)   │  (중앙 서버)  │      (HC-06)     │  (경비실)    │
└─────────────┘                  └─────────────┘                  └─────────────┘
```

### 1. STM32 매장 제어 시스템

**하드웨어**
- MCU: STM32F411 (Nucleo-F411RE)
- WiFi 모듈: ESP-01 (UART6, 38400 baud)
- 센서:
  - DHT11: 온습도 센서
  - 적외선 거리 센서 (GP2Y0A21YK0F) x2: 출입 감지용
- 액추에이터:
  - DC 모터 (TIM4 PWM): 환기 팬 제어 (0~1000 속도)
  - LED (TIM1 PWM): 조명 제어 (0~100% 밝기)
  - 서보 모터 (TIM3 PWM): 자동문 제어

**주요 기능**
- 온습도 자동 측정 (5초 간격)
- 온도에 따른 자동 환기 (23°C 이상 팬 가동)
- 출입 감지 및 자동문 개폐 (17cm 이내 감지)
- 고객 수 자동 카운팅
- 조명 자동 제어 (고객 감지 시 100% 점등)
- 서버와 실시간 TCP/IP 통신 (10초 간격 연결 확인)

**파일 구조**
```
smart_strore_stm32/smart_store_stm32/Core/
├── Src/
│   ├── main.c        # 메인 제어 로직
│   ├── dht.c         # DHT11 센서 드라이버
│   └── esp.c         # ESP-01 WiFi 통신
└── Inc/
    ├── main.h
    ├── dht.h
    └── esp.h
```

### 2. C 기반 TCP/IP 서버

**개발 환경**: Linux/Unix (POSIX 소켓)

**주요 기능**
- 멀티 클라이언트 처리 (최대 32개 동시 접속)
- 스레드 기반 비동기 통신 (pthread)
- ID/Password 기반 인증 (idpasswd.txt)
- 클라이언트 간 메시지 라우팅
- 로그 기록 및 모니터링
- 시간 동기화 서비스

**실행 방법**
```bash
# 컴파일
gcc -o store_server store_server.c -lpthread

# 실행
./store_server <포트번호>
```

**설정 파일** (`idpasswd.txt`)
```
A PASSWD
B PASSWD
BOSS PASSWD
```

### 3. Arduino 경비실 제어 시스템 (Office)

**파일**: `smart_store_office_arduino.ino`

**하드웨어**
- MCU: Arduino Uno/Nano
- Bluetooth: HC-06 (Software Serial, 9600 baud)
- LCD: 16x2 I2C LCD (0x27)
- Keypad: 4x4 매트릭스 키패드
- 타이머: MsTimer2 라이브러리

**주요 기능**
- 비밀번호 인증 시스템 (기본값: 1234)
- 매장 선택 (A/B/C/D)
- 실시간 매장 상태 모니터링:
  - 온도, 습도
  - LED 밝기
  - 도어 상태 (잠김/열림)
  - 고객 수
  - 팬 속도
- 긴급 정지 기능 (ALLSTOP)
- LCD 실시간 시각 표시
- 서버 시간 동기화

**LCD 표시 예시**
```
25.3`C led:100
lock h:5 f:600
```

### 4. Arduino Bluetooth 브리지 (선택 사항)

**파일**: `smart_store_arduino.c`

라즈베리파이 또는 Linux 시스템에서 Bluetooth를 통해 Arduino와 서버 간 통신을 중계하는 브리지 프로그램입니다.

**실행 방법**
```bash
# 컴파일
gcc -o smart_store_arduino smart_store_arduino.c -lbluetooth -lpthread

# 실행
./smart_store_arduino <서버IP> <포트> <클라이언트ID>
```

## 📡 통신 프로토콜 (API 명세)

### 메시지 형식
```
[수신자ID]명령@파라미터1@파라미터2@...\n
```
- **구분자**: `[@]`로 파싱
- **종료**: `\n` (개행 문자 필수)
- **수신자ID**: A, B, C, D (매장), BOSS (경비실), A_DOOR (이벤트 전용)

---

### 1. STM32 매장 → 서버

#### 1.1 자동문 개방 알림
```
[A_DOOR]DOOR@OPEN
```
- **전송 시점**: 적외선 센서가 17cm 이내 객체 감지 시
- **동작**: 자동문 개방 이벤트를 서버에 알림 (응답 없음)

---

### 2. 클라이언트 → 서버 → STM32 매장

#### 2.1 긴급 정지/해제
**요청 (정지)**:
```
[A]ALLSTOP@ON
```
**응답**:
```
[A]ALLSTOP@ON
```
- **동작**: 팬=0, 조명=0, 문잠김(lockFlag=1)

**요청 (해제)**:
```
[A]ALLSTOP@OFF
```
**응답**:
```
[A]ALLSTOP@OFF
```
- **동작**: 팬=500, 조명=50%, 문열림(lockFlag=0)

#### 2.2 매장 상태 조회
**요청**:
```
[A]STATE
```
**응답**:
```
[A]STATE@온도@습도@LED밝기@문상태@고객수@팬속도
```
**응답 예시**:
```
[A]STATE@25.3@65@100@0@5@600
```
- **온도**: "25.3" (문자열, 소수점 포함)
- **습도**: 65 (정수, %)
- **LED밝기**: 100 (0~100)
- **문상태**: 0=열림, 1=잠김
- **고객수**: 5 (누적)
- **팬속도**: 600 (0~1000)

#### 2.3 팬 제어
**요청**:
```
[A]FAN@600
```
**응답**:
```
[A]FAN@600
```
- **파라미터**: 0~1000 (PWM 듀티)
- **동작**: TIM4 PWM으로 팬 속도 변경

#### 2.4 온습도 조회
**요청**:
```
[A]DHTSTATE
```
**응답**:
```
[A]DHTSTATE@25.3@65
```
- **온도**: "25.3" (문자열)
- **습도**: 65 (정수)

#### 2.5 조명 상태 조회
**요청**:
```
[A]LED@STATE
```
**응답**:
```
[A]LED@STATE@80
```
- **파라미터**: 현재 밝기 (0~100)

#### 2.6 조명 밝기 제어
**요청**:
```
[A]LED@80
```
**응답**:
```
[A]LED@80
```
- **파라미터**: 0~100 (%)
- **동작**: TIM1 PWM으로 조명 밝기 변경 (내부적으로 0~1000 변환)

---

### 3. 서버 내장 명령 (공통)

#### 3.1 전체 메시지 브로드캐스트
**요청**:
```
[ALLMSG]메시지내용
```
- **동작**: 접속된 모든 클라이언트에게 전송

#### 3.2 접속 ID 목록 조회
**요청**:
```
[IDLIST]
```
**응답**:
```
[송신자ID]IDLIST A BOSS
```
- **동작**: 현재 접속 중인 클라이언트 ID 목록 반환

#### 3.3 서버 시간 동기화
**요청**:
```
[GETTIME]
```
**응답**:
```
[GETTIME]YY.MM.DD HH:MM:SS DAY
```
**응답 예시**:
```
[GETTIME]25.09.30 14:35:20 Mon
```
- **형식**: `YY.MM.DD HH:MM:SS DAY`
- **DAY**: Sun/Mon/Tue/Wed/Thu/Fri/Sat
- **사용처**: Arduino 경비실 시각 동기화

---

### 4. 서버 시스템 메시지

#### 4.1 연결 성공
```
[클라이언트ID] New connected! (ip:192.168.0.100,fd:5,sockcnt:2)
```

#### 4.2 중복 로그인
```
[클라이언트ID] Already logged!
```
- **STM32 동작**: 기존 연결 종료 후 재연결 시도 (MCU용 예외 처리)

#### 4.3 인증 실패
```
[클라이언트ID] Authentication Error!
```
- **원인**: ID/PW 불일치 또는 idpasswd.txt에 미등록

---

### 5. Arduino 경비실 수신 파싱

Arduino는 서버로부터 받은 응답을 다음과 같이 파싱합니다:

#### STATE 응답 파싱
```c
// 수신: [A]STATE@25.3@65@100@0@5@600
pArray[0] = "A"          // 매장 ID
pArray[1] = "STATE"      // 명령
pArray[2] = "25.3"       // 온도
pArray[3] = "65"         // 습도
pArray[4] = "100"        // LED 밝기
pArray[5] = "0"          // 문 상태
pArray[6] = "5"          // 고객 수
pArray[7] = "600"        // 팬 속도
```

#### GETTIME 응답 파싱
```c
// 수신: [GETTIME]25.09.30 14:35:20 Mon
pArray[0] = "GETTIME"
pArray[1] = "25.09.30 14:35:20 Mon"  // 시간 문자열
// 문자열 인덱스로 각 값 추출:
// [1][0-1]: 년(25), [1][3-4]: 월(09), [1][6-7]: 일(30)
// [1][9-10]: 시(14), [1][12-13]: 분(35), [1][15-16]: 초(20)
// [1][18-20]: 요일(Mon)
```

## ⚙️ 주요 기능 상세

### 1. 자동 출입 관리
- 매장 외부 센서: 고객 진입 감지 및 카운팅
- 매장 내부 센서: 퇴장 감지
- 거리 17cm 이내 감지 시 자동문 개방
- 서버로 개방 이벤트 전송

### 2. 스마트 조명 제어
- 평상시: 설정 밝기 유지 (기본 20%)
- 고객 감지: 자동 100% 점등
- 60초 후 원래 밝기로 복귀
- 원격 제어 가능 (0~100%)

### 3. 자동 환기 시스템
- 온도 기반 자동 제어:
  - 23°C 이상: 400 RPM
  - 25°C 이상: 600 RPM
  - 27°C 이상: 800 RPM
  - 30°C 이상: 1000 RPM (최대)
- 원격 수동 제어 가능

### 4. 경비실 모니터링
- 비밀번호 인증 (4자리)
- 매장 선택 (A/B/C/D 중 택 1)
- 실시간 상태 표시:
  ```
  25.3`C led:100
  lock h:5 f:600
  ```
- 긴급 정지 명령 전송

## 🛠️ 개발 환경 및 도구

- **STM32**: STM32CubeIDE, HAL Library
- **Server**: GCC, Linux/Unix, pthread
- **Arduino**: Arduino IDE
- **라이브러리**:
  - SoftwareSerial
  - Wire (I2C)
  - MsTimer2
  - Keypad
  - LiquidCrystal_I2C
  - bluetooth (Linux/Unix)

## 📦 설치 및 실행

### 1. 서버 설정
```bash
# idpasswd.txt 작성
echo "A PASSWD" > idpasswd.txt
echo "BOSS PASSWD" >> idpasswd.txt

# 컴파일 및 실행
gcc -o store_server store_server.c -lpthread
./store_server 5000
```

### 2. STM32 펌웨어 업로드
```bash
# STM32CubeIDE에서 프로젝트 열기
cd smart_strore_stm32/smart_store_stm32

# 빌드 및 업로드
# Project → Build Project
# Run → Debug (또는 F11)
```

### 3. Arduino 경비실 펌웨어 업로드
```bash
# Arduino IDE에서 열기
# smart_store_office_arduino.ino

# 필요한 라이브러리 설치:
# - MsTimer2
# - Keypad
# - LiquidCrystal I2C

# 보드 선택 후 업로드
```

### 4. Bluetooth 브리지 실행 (선택 사항)
```bash
# 컴파일
gcc -o smart_store_arduino smart_store_arduino.c -lbluetooth -lpthread

# 실행 (HC-06 MAC 주소 수정 필요)
./smart_store_arduino 192.168.0.100 5000 BOSS
```

## 🌐 네트워크 설정

### ESP-01 WiFi 설정
```
AT+CWMODE=1                    # Station 모드
AT+CWJAP="SSID","PASSWORD"     # WiFi 접속
AT+CIPMUX=0                    # 단일 연결 모드
AT+CIPSTART="TCP","서버IP",5000  # 서버 연결
```

### 포트 설정
- Server: 5000 (TCP)
- STM32 UART6 (ESP-01): 38400 baud
- Arduino Bluetooth: 9600 baud
- STM32 Debug (UART2): 115200 baud

### Bluetooth 설정
- HC-06 MAC 주소: `98:DA:60:02:6B:43` (코드 내 수정 가능)
- Baud Rate: 9600
- RFCOMM Channel: 1

## 🔄 시스템 동작 흐름

```
1. 시스템 초기화
   ↓
2. WiFi/Bluetooth 연결
   ↓
3. 서버 인증 및 연결
   ↓
4. 주기적 센서 데이터 수집
   │ └→ DHT11 (5초)
   │ └→ 적외선 센서 (1초)
   ↓
5. 이벤트 기반 제어
   │ └→ 고객 감지 → 자동문 개방 + 조명 ON
   │ └→ 온도 상승 → 팬 가동
   │ └→ 서버 명령 수신 → 장치 제어
   ↓
6. 상태 보고 (요청 시)
   ↓
7. 긴급 상황 처리 (ALLSTOP)
```

## 📁 프로젝트 파일 구조

```
mini-project_smart-store/
├── README.md                              # 프로젝트 문서
├── store_server.c                         # TCP/IP 중앙 서버
├── smart_store_office_arduino.ino         # Arduino 경비실 제어 (메인)
├── smart_store_arduino.c                  # Bluetooth 브리지 (선택)
├── smart_strore_stm32/                    # STM32 프로젝트
│   └── smart_store_stm32/
│       └── Core/
│           ├── Src/
│           │   ├── main.c                 # 매장 제어 메인
│           │   ├── dht.c                  # DHT11 드라이버
│           │   └── esp.c                  # ESP-01 통신
│           └── Inc/
│               ├── main.h
│               ├── dht.h
│               └── esp.h
├── asset/                                 # 데모 영상
│   ├── A지점 구현 영상 - stm32.mp4
│   ├── 경비실 구현 영상 - 아두이노 우노.mp4
│   └── 어플 원격제어 구현 영상.mp4
└── 무인 매장을 위한 자동화 IoT 시스템.pptx  # 발표 자료
```

## 🔧 트러블슈팅

### STM32 연결 문제
- ESP-01 AT 명령어 응답 확인
- WiFi SSID/Password 확인
- 서버 IP 주소 및 포트 확인
- UART 보드레이트 확인 (38400)

### Arduino Bluetooth 연결 문제
- HC-06 페어링 확인 (기본 PIN: 1234)
- MAC 주소 확인 및 코드 수정
- SoftwareSerial 핀 연결 확인 (RX:10, TX:11)

### 서버 연결 문제
- 방화벽 설정 확인
- idpasswd.txt 파일 존재 및 형식 확인
- 포트 사용 가능 여부 확인 (`netstat -an | grep 5000`)

## 📄 라이선스

이 프로젝트는 교육 목적의 미니 프로젝트입니다.

## 👥 개발자

- [@minjuyeong](https://github.com/minjuyeong)
- [@uniljetstream](https://github.com/uniljetstream)

---

**프로젝트 기간**: 2025년
**개발 환경**: STM32CubeIDE 1.18.1, Arduino IDE, Linux GCC