/* Demo for XTronical 7735 driver library for generic 128x128 pixel
   TFT displays, based on the one from ADAFruit, their original
   header comments are below.
 */

/***************************************************
  This is a library for the genric 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <XTronical_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "time.h"

// set up pins we are going to use to talk to the screen
#define TFT_DC     2       // register select (stands for Data Control perhaps!)
#define TFT_RST   4         // Display reset pin, you can also connect this to the ESP32 reset
                            // in which case, set this #define pin to -1!
#define TFT_CS   5       // Display enable (Chip select), if not enabled will not talk on SPI bus

// initialise the routine to talk to this display with these pin connections (as we've missed off
// TFT_SCLK and TFT_MOSI the routine presumes we are using hardware SPI and internally uses 13 and 11
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);  

float p = 3.1415926;

// dummy texts
#define LEFTMARGIN 5
#define TOPMARGIN 10

char tempStr[10] = "36.78℃";
char humiStr[10] = "55.55%";
struct tm tm;

void setup(void) {
  tft.init();   // initialize a ST7735S chip,
  tm.tm_sec = 30;
  tm.tm_min = 45;
  tm.tm_hour = 12;
  tm.tm_mday = 14;
  tm.tm_mon = 6;
  tm.tm_year = 2024;
  tm.tm_wday = 6;
  tm.tm_yday = 165;
  tm.tm_isdst = -1;
}

void loop() {  
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(2);

  // Lines
  tft.drawRect(5,5, 118, 118, ST7735_WHITE);
  tft.drawFastHLine(10, 43, 108, ST7735_WHITE);
  tft.drawFastHLine(10, 44, 108, ST7735_WHITE);

  tft.drawFastHLine(10, 86, 108, ST7735_WHITE);
  tft.drawFastHLine(10, 85, 108, ST7735_WHITE);

  tft.drawFastHLine(15, 104, 98, ST7735_WHITE);
  tft.drawFastVLine(64, 88, 13, ST7735_WHITE);
  tft.drawFastVLine(64, 106, 13, ST7735_WHITE);

  // 상단 
  tft.setTextSize(1);
  tft.setCursor(12, 12);
  tft.setTextColor(ST7735_WHITE);
  char dateStr[16];
  snprintf(dateStr, sizeof(dateStr), "%04d %02d %02d", tm.tm_year, tm.tm_mon, tm.tm_mday);
  tft.printf(dateStr);

  tft.setCursor(12, 28);
  tft.printf("Seoul, Korea");
  // tft.print(dateStr);

  // HH:MM:SS 크기 2 가운데 버전
  /*
  tft.setTextSize(2);
  tft.setCursor(LEFTMARGIN + 10, tft.height()/3 + 15);
  tft.print(timeStr);
  */
  
  // 중단 HHMM 크게, ss는 작게 띄우는 버전
  tft.setTextSize(3);
  tft.setCursor(14, 52);
  char timeStr[5];
  snprintf(timeStr, sizeof(timeStr), "%02d%02d", tm.tm_hour, tm.tm_min);
  tft.print(timeStr);
  
  char secStr[3];
  snprintf(secStr, sizeof(secStr), "%2d", tm.tm_sec);
  tft.setTextSize(2);
  tft.setCursor(92, 56);
  tft.print(secStr);
  // 하단

  tft.setTextSize(1);
  tft.setCursor(16, 90);
  tft.printf("24.7");

  tft.setCursor(70,90);
  tft.printf("53.1");

  tft.setCursor(23, 106);
  tft.printf("0623");

  tft.setCursor(70,106);
  tft.printf("1923");
  /*
  for(uint16_t rotation=0; rotation<4;rotation++)
    Demo(rotation);
  */
 
  delay(30000);
}

void Demo(uint16_t rotation)
{

  tft.setRotation(rotation);
  uint16_t time = millis();
  tft.fillScreen(ST7735_BLACK);

  time = millis() - time;

  delay(500);

  // large block of text
  tft.fillScreen(ST7735_BLACK);
 
  testdrawtext("This is a test of some very log text that I written for the sole purpose of checking if the display displays very long text correctly.", ST7735_WHITE);
  delay(10000);

  // tft print function!
  tftPrintTest();
  delay(4000);

  // a single pixel
  tft.drawPixel(tft.width()/2, tft.height()/2, ST7735_GREEN);
  delay(500);

  testdrawrects(ST7735_GREEN);
  delay(500);

  testfillrects(ST7735_YELLOW, ST7735_MAGENTA);
  delay(500);

  tft.fillScreen(ST7735_BLACK);
  testfillcircles(10, ST7735_BLUE);
  testdrawcircles(10, ST7735_WHITE);
  delay(500);

  testroundrects();
  delay(500);

  testtriangles();
  delay(1000);
  for(int i=2;i>0;i--)
  {
    tft.invertDisplay(true);
    delay(500);
    tft.invertDisplay(false);
    delay(500);
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0,0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);
  tft.println("I'm running");
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(3);
  tft.println("on a");
  tft.setTextColor(ST7735_BLUE);
  tft.println("ESP32");
  tft.setTextSize(4);
  tft.setTextColor(ST7735_CYAN);
  tft.print("Ha ha!");
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST7735_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST7735_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST7735_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST7735_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST7735_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST7735_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST7735_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_GREEN);
}
void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
}
void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST7735_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST7735_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}
