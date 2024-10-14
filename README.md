## Satellite

<center>
    <img 
    src="https://github.com/user-attachments/assets/45200387-3d6f-4814-b07b-25bf058bfbbb" 
    />
</center>

ESP32와 TFT 1.4인치 스크린을 이용한 환경정보 디스플레이 프로젝트

[MOHIT BHOITE의 Boron Lander](https://www.bhoite.com/sculptures/boron-lander/) 프로젝트의 소프트웨어 구현을 목표로 한 프로젝트

상세 개발 후기는 [블로그](https://jinhg0214.github.io/posts/satelite/) 참조

### 주요 기능
---
- TFT 1.4 인치 스크린을 통해 환경 정보 확인
- 날씨 예보 API 활용
- 실시간 데이터 업데이트

## 프로젝트 정보
- 이진행([@Github](https://github.com/jinhg0214/))
- 개발 기한 : 2024-06 ~ 2024-09

### 사용 기술 스택

Hardware 

<img src="https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=ESP32&logoColor=white">
<img src="https://img.shields.io/badge/ST7735-000C1F?style=for-the-badge&logo=ST7735&logoColor=white">
<img src="https://img.shields.io/badge/SHT31-2CB9F1?style=for-the-badge&logo=SHT31&logoColor=white">

Software

<img src="https://img.shields.io/badge/arduino-00878F?style=for-the-badge&logo=arduino&logoColor=white">
<img src="https://img.shields.io/badge/espressif-E7352C?style=for-the-badge&logo=espressif&logoColor=white">
<img src="https://img.shields.io/badge/c-A8B9CC?style=for-the-badge&logo=c&logoColor=white">
<img src="https://img.shields.io/badge/github-181717?style=for-the-badge&logo=github&logoColor=white">
<img src="https://img.shields.io/badge/obsidian-7C3AED?style=for-the-badge&logo=obsidian&logoColor=white">

### 아키텍쳐 
---
### 프로젝트 아키텍처 

![architecture](https://github.com/user-attachments/assets/f59cee70-9b7d-4e44-b7dc-44fa353a91a2)

ESP32를 이용하여

WiFi를 이용하여 OpenWeatherAPI로 부터 날씨 정보 수신

SHT31와 I2C 통신을 이용한 실시간 온습도 정보 센싱 

TFT 1.44인치 디스플레이 모듈과 SPI 통신을 이용한 데이터 디스플레이

### 디렉토리 구조
```
📦 Satellite
 ┣ 📂Docs
 ┃ ┣ 📜creating-custom-symbol-font-for-adafruit-gfx-library // 커스텀 폰트 제작방법 adafruit 문서
 ┣ 📂src
 ┃ ┣ 📂currenttime // ntp 서버로부터 시간 정보 얻는 테스트 프로그램
 ┃ ┃ ┣ 📜currenttime.ino 
 ┃ ┣ 📂GraphicsTestESP32 // ST7735를 이용한 디스플레이 출력 테스트 프로그램
 ┃ ┃ ┣ 📜GraphicsTestESP32.ino
 ┃ ┣ 📂SHT31 // SHT31 센서 테스트 프로그램
 ┃ ┃ ┣ 📜SHT31test.ino
 ┃ ┣ 📂Wifi_and_WeatherAPI_Test // 와이파이 및 오픈웨더API 연결 테스트 프로그램
 ┃ ┃ ┣ 📂Wifi_and_WeatherAPI_Test.ino
 ┣ 📂main
 ┃ ┣ 📜main.io // 프로젝트 메인 파일
 ┃ ┣ 📜SymbolMono18pt7b.h // 아이콘이 추가된 커스텀 폰트 파일
 ┣ 📜README.md
```

## 3. 시작 가이드

### 하드웨어 구성
필요 하드웨어는 다음과 같다
- ESP32-Wroom-32
- TFT 1.44 inch LCD 모듈
- SHT 31

다음과 같이 핀맵을 구성한다

![pinmap](https://github.com/user-attachments/assets/2c871f02-c884-4aca-b90e-785fab657896)

### 실행 방법
1. Arduino IDE 설치 및 실행
2. ESP32 관련 라이브러리 설치 [링크 참조](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
3. `Arduino_JSON` 라이브러리 설치
4. `XTronical-ST7735-Library` [링크 참조](https://www.xtronical.com/basics/displays/lcd-tft-colourcolor-display-128x128-pixelst7735-driver/128x128-colour-lcd-esp32/), `Adafruit GFX Library` 설치
5. 필요 시 XT라이브러리 내 rowstart, colstart를 수정하여 오프셋 수정
6. main.ino 파일 내 WiFi ID, PW, OpenWeather API Key, 국가, 지역, Ntp 서버등을 설정 후 ESP32에 적재
7. 실행