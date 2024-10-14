#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// TFT
#include <Adafruit_GFX.h>    // Core graphics library
#include <XTronical_ST7735.h> // Hardware-specific library
#include <SPI.h>

// SHT31
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

// font
#include "SymbolMono18pt7b.h"

/********************** CONFIG **********************/
// 1. WiFi and password
const char* ssid = "YOUR_WIFI_ID";
const char* password = "YOUR_WIFI_PASSWORD";

// 2. openweather API key, city, country code
String openWeatherMapApiKey = "YOUR_OPENWEATHER_API";
String city = "YOUR_CITY";
String countryCode = "YOUR_COUNTRY";

// 3. ntp server for time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 32400; // UTC+0900 https://en.wikipedia.org/wiki/UTC_offset

// Time Interval
const long intervalSec = 1000;    // 1초 (1000 밀리초)
const long intervalTempHumi = 60000;  // 1분 (60000밀리초)
const long intervalNTP = 300000;          // 5분 (300000 밀리초)
const long intervalWeather = 300000;      // 5분 (300000 밀리초)

// 4. TFT Screen
// set up pins we are going to use to talk to the screen
#define TFT_DC     2       // register select (stands for Data Control perhaps!)
#define TFT_RST   4         // Display reset pin, you can also connect this to the ESP32 reset
                            // in which case, set this #define pin to -1!
#define TFT_CS   5       // Display enable (Chip select), if not enabled will not talk on SPI bus

// 5. SHT31
/****************************************************/

// 2. openweather 
JSONVar myObject; // JSON Data from openweather API

// 3. ntp time
struct tm timeinfo;
struct tm *buffer;

// Time Interval
unsigned long previousMillisSec = 0;
unsigned long previousMillisNTP = 0;
unsigned long previousMillisWeather = 0;

// 4. TFT 
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);  

int prevDay = -1;
int prevMin = -1;
int prevSec = -1;

// 5. SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();
bool enableHeater = false;
float temp = 23.45;
float humi = 51.23;

/************************************/

void setup() {
  Serial.begin(115200);

  // 1. WiFi init
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // 2. openweather api
  checkWeatherInfo();

  // 3. Timer Init
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Timezone setting
  syncTimeWithNtp(); // send NTP request for init 

  // 4. TFT init
  tft.init();
  tft.setRotation(2);
  displayInit();

  // 5. SHT31
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  checkTemperature();
  drawTempAndHumi();
}

void loop() {
  unsigned long currentMillis = millis();

  // 날짜 갱신
  if(prevDay != timeinfo.tm_mday){
    prevDay = timeinfo.tm_mday;
    drawDailyInfo();
  }

  // 1초 간격 수행
  if (currentMillis - previousMillisSec >= intervalSec) {
    previousMillisSec = currentMillis;
    updateLocalTime();
    drawTime();
  }

  // 1분 간격 수행 - 온습도 갱신
  if(currentMillis - previousMillisSec >= intervalTempHumi){
    checkTemperature();
    drawTempAndHumi();
  }

  // NTP 서버에서 시간 보정
  if (currentMillis - previousMillisNTP >= intervalNTP) {
    previousMillisNTP = currentMillis;
    syncTimeWithNtp();
  }

  // 날씨 정보 
  if (currentMillis - previousMillisWeather >= intervalWeather) {
    previousMillisWeather = currentMillis;
    checkWeatherInfo();
  }
}

// Daily Info : 날짜(YYYY-MM-DD), 요일, 요일, 현재 위치, 일출, 일몰
// 날짜 변경 시 자동으로 수행됨
void drawDailyInfo(){
  tft.fillRect(10, 10, 65, 15, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(12, 12);
  char dateStr[11];
  snprintf(dateStr, sizeof(dateStr), "%04d %02d %02d",
    timeinfo.tm_year + 1900, // tm_year는 1900년 이후의 년도이므로 1900을 더함
    timeinfo.tm_mon + 1, // tm_mon은 0부터 시작하므로 1을 더함
    timeinfo.tm_mday
  );
  tft.printf(dateStr);

  // 요일 
  tft.fillRect(72, 10, 23, 15, ST7735_BLACK);
  tft.setCursor(77, 12);
  const char* weekdays[] = {"SUN", "MON", "TUE", "WED", "FRI", "SAT"};
  int wday_idx = timeinfo.tm_wday;
  const char* wday_str = weekdays[wday_idx];
  char wdayStr[4];
  strncpy(wdayStr, wday_str, sizeof(wdayStr) - 1);
  wdayStr[sizeof(wdayStr) - 1] = '\0';
  tft.printf(wdayStr);

  // 위치 - 수정 필요
  tft.fillRect(10, 25, 75, 15, ST7735_BLACK);
  tft.setCursor(12, 28);
  tft.printf("Seoul Korea");  // openweatherAPI로 받아올것 

  // 날씨 아이콘 넣기
  drawWeatherSymbol();

  // 일출
  tft.fillRect(10, 108, 50, 12, ST7735_BLACK);

  drawSymbol(11, 123, MY_SUNRISE, ST7735_WHITE, ST7735_BLACK, 1);

  tft.setCursor(35, 110);

  long sun_jsoninfo = myObject["sys"]["sunrise"];
  time_t sun_time = (time_t)sun_jsoninfo;
  buffer = localtime(&sun_time);
  int sun_hour = buffer->tm_hour;
  int sun_min = buffer->tm_min;
    
  char sunStr[5];
  snprintf(sunStr, sizeof(sunStr), "%02d%02d", sun_hour, sun_min);

  tft.printf(sunStr);
  
  // 일몰
  tft.fillRect(68, 108, 50, 12, ST7735_BLACK);

  drawSymbol(95, 123, MY_SUNSET, ST7735_WHITE, ST7735_BLACK, 1);

  tft.setCursor(70,110);

  sun_jsoninfo = myObject["sys"]["sunset"];
  sun_time = (time_t)sun_jsoninfo;
  buffer = localtime(&sun_time);
  sun_hour = buffer->tm_hour;
  sun_min = buffer->tm_min;
  
  snprintf(sunStr, sizeof(sunStr), "%02d%02d", sun_hour, sun_min);
  
  tft.printf(sunStr);  
}

void drawTime(){
  if(prevMin != timeinfo.tm_min){
    tft.fillRect(14,54, 70, 25, ST7735_BLACK);
    tft.setTextSize(3);
    tft.setCursor(14, 54);
    char timeStr[5];
    snprintf(timeStr, sizeof(timeStr), "%02d%02d", timeinfo.tm_hour, timeinfo.tm_min);
    tft.print(timeStr);
    prevMin = timeinfo.tm_min;
  }

  if(prevSec != timeinfo.tm_sec){
    tft.fillRect(88,58, 30, 25, ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor(92, 61);
    char secStr[3];
    snprintf(secStr, sizeof(secStr), "%02d", timeinfo.tm_sec);
    tft.print(secStr);
    prevSec = timeinfo.tm_sec;
  }
}

void drawTempAndHumi(){
  // 온도 관련
  tft.fillRect(13,90, 45, 12, ST7735_BLACK);    

  // 온도에 맞는 그림 출력
  drawSymbol(12, 104, MY_TEMP_OUTLINE, ST7735_WHITE, ST7735_BLACK, 1);
  if(temp > 25){
    drawSymbol(12, 104, MY_TEMP_HIGH, ST7735_RED, ST7735_BLACK, 1);
  } 
  else if (temp < 15){
    drawSymbol(12, 104, MY_TEMP_LOW, ST7735_CYAN, ST7735_BLACK, 1);
  }
  else{
    drawSymbol(12, 104, MY_TEMP_MODERATE, ST7735_YELLOW, ST7735_BLACK, 1);
  }

  // 온도 텍스트 출력
  tft.setTextSize(1);
  tft.setCursor(35, 92);
  char tempStr[5];
  snprintf(tempStr, sizeof(tempStr), "%0.1f", temp); // 소수점 한자리까지 출력
  tft.print(tempStr);

  // 습도 관련
  tft.fillRect(68,90, 45, 12, ST7735_BLACK);

  // 습도 아이콘 출력
  drawSymbol(95, 105, MY_HUMI, ST7735_CYAN, ST7735_BLACK, 1);

  // 습도 텍스트 출력
  char humiStr[5];
  snprintf(humiStr, sizeof(humiStr), "%0.1f", humi);
  tft.setCursor(70,92);
  tft.print(humiStr);
}

// 화면 초기 세팅
void displayInit(){
  tft.fillScreen(ST7735_BLACK);

  // Lines
  tft.drawRect(5,5, 118, 118, ST7735_WHITE);
  tft.drawFastHLine(10, 43, 108, ST7735_WHITE);
  tft.drawFastHLine(10, 44, 108, ST7735_WHITE);

  tft.drawFastHLine(10, 86, 108, ST7735_WHITE);
  tft.drawFastHLine(10, 85, 108, ST7735_WHITE);

  tft.drawFastHLine(15, 104, 98, ST7735_WHITE);
  tft.drawFastVLine(64, 89, 13, ST7735_WHITE);
  tft.drawFastVLine(64, 107, 13, ST7735_WHITE);
}

// SHT31 관련 함수
void checkTemperature(){
  temp = sht31.readTemperature();
  humi = sht31.readHumidity();
  
  /*
  if (! isnan(temp)) {  // check if 'is not a number'
    Serial.print("checkTemperature :: Temp *C = "); Serial.print(temp); Serial.print("\t\t");
  } else { 
    Serial.println("checkTemperature :: Failed to read temperature");
  }
  
  if (! isnan(humi)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(humi);
  } else { 
    Serial.println("Failed to read humidity");
  }
  */

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  int loopCnt = 0;
  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
  loopCnt++; // 함수가 호출될 때 마다 loopCnt가 증가함
}

// NTP Time 관련 함수
void syncTimeWithNtp() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    Serial.println("Time synchronized with NTP server");
  }
}

// 로컬 시간 업데이트 함수
void updateLocalTime() {
  timeinfo.tm_sec++;
  mktime(&timeinfo); // 시간 구조체 정규화
  /*
  Serial.println("updateLocalTime :: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
  */
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void checkWeatherInfo(){
  // send HTTP GET Request
  String jsonBuffer;
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";

  jsonBuffer = httpGETRequest(serverPath.c_str());

  // Serial.println(jsonBuffer);
  myObject = JSON.parse(jsonBuffer);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  /*
  Serial.println("checkWeatherInfo :: ");
  Serial.print("JSON object = ");
  Serial.println(myObject);
  Serial.print("Temperature: ");
  Serial.println(myObject["main"]["temp"]);
  Serial.print("Pressure: ");
  Serial.println(myObject["main"]["pressure"]);
  Serial.print("Humidity: ");
  Serial.println(myObject["main"]["humidity"]);
  Serial.print("Wind Speed: ");
  Serial.println(myObject["wind"]["speed"]);
  */
  return;
}

// 심볼을 그리는 함수
void drawSymbol(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t Size){
  if( (c>=32) && (c<=126) ){ //If it's 33-126 then use standard mono 18 font
      tft.setFont(NULL);
  } else {
    tft.setFont(&SymbolMono18pt7b);//Otherwise use special symbol font
    if (c>126) {      //Remap anything above 126 to be in the range 32 and upwards
      c-=(127-32);
    }
  }
  tft.drawChar(x,y,c,color,bg,Size);
  tft.setFont(NULL); // 출력 후 다시 폰트는 디폴트로 설정
}

void drawWeatherSymbol(){

  int weatherId = (int)myObject["weather"][0]["id"];
  
  int xpos = 97;
  int ypos = 28;
  int size = 1;

  tft.fillRect(95, 10, 27, 32, ST7735_BLACK);

  // Thunderstorm
  if(weatherId / 100 == 2){ 
    drawSymbol(xpos, ypos, MY_THUNDERSTORM, ST7735_YELLOW, ST7735_BLACK, size);
  }
  // Drizzle
  else if (weatherId / 100 == 3){
    drawSymbol(xpos, ypos, MY_DRIZZLE, ST7735_CYAN, ST7735_BLACK, size);
  }
  // Heavy RAIN
  else if(weatherId == 503){
    drawSymbol(xpos, ypos, MY_UMBRELLA, ST7735_BLUE, ST7735_BLACK, size);
  }
  // Rain
  else if (weatherId / 100 == 5){
    drawSymbol(xpos, ypos, MY_RAIN, ST7735_BLUE, ST7735_BLACK, size);
  }
  // Snow
  else if (weatherId / 100 == 6){
    drawSymbol(xpos, ypos, MY_SNOW, ST7735_CYAN, ST7735_BLACK, size);
  }
  // Clear
  else if (weatherId == 800){
    drawSymbol(xpos, ypos, MY_CLEAR, ST7735_WHITE, ST7735_BLACK, size);
  }
  // Clouds
  else{
    drawSymbol(xpos, ypos, MY_CLOUD, ST7735_WHITE, ST7735_BLACK, size);
  }
  
}