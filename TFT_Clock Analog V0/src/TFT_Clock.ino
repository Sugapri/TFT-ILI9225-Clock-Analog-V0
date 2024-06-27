/*
 An example analogue clock using a TFT LCD screen to show the time
 use of some of the drawing commands with the ST7735 library.

 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo. 

 Uses compile time to set the time so a reset will start with the compile time again
 
 Gilchrist 6/2/2014 1.0
 Updated by Bodmer
 */

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

RTC_DS3231 rtc;

#define TFT_GREY 0xBDF7

float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
uint16_t osx=32, osy=32, omx=32, omy=32, ohx=32, ohy=32;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint32_t targetTime = 0;                    // for next 1 second timeout
float           floatnow=0;

char daysOfTheWeek[7][10] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
const char Buln[12][4]= {"Jan","Feb","Mar","Apr","Mei","Jun","Jul","Ags","Sep","Okt","Nop","Des"};

bool initial = 1;

void setup(void) {

  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_GREY);
  tft.setTextColor(TFT_GREEN, TFT_GREY);  // Adding a black background colour erases previous text automatically
  
  // Draw clock face
  tft.fillCircle(32, 32, 31, TFT_BLUE);
  tft.fillCircle(32, 32, 29, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*26+32;
    yy0 = sy*26+32;
    x1 = sx*30+32;
    yy1 = sy*30+32;

    tft.drawLine(x0, yy0, x1, yy1, TFT_YELLOW);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*27+32;
    yy0 = sy*27+32;
    
    tft.drawPixel(x0, yy0, TFT_BLUE);
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==0 || i==180) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
  }

  tft.fillCircle(32, 32, 2, TFT_RED);

  // Draw text at position 64,125 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
  tft.drawCentreString("Time flies",36,70,2);

  targetTime = millis() + 1000; 

  // rtc only
    if(! rtc.begin()) {
      Serial.println("RTC tidak terhubung, Cek kembai wiring! ");
      Serial.flush();
      while (1);
      }
    if (! rtc.lostPower()) {
      Serial.println("RTC tidak bekerja, Setel ulang Waktu!");
      //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      //rtc.adjust(DateTime(2020, 4, 22, 19, 42, 0));
      }
     Serial.println("Test modul RTC DS 3231!");
     delay (500);
     //print_intro(); 
}

void loop() {

  updateTime();
  DateTime now = rtc.now();
  uint8_t hh=now.hour(), mm=now.minute(), ss=now.second();  // Get H, M, S from compile time  

  

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss*6;                  // 0-59 -> 0-354
    mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
    hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg-90)*0.0174532925);    
    hy = sin((hdeg-90)*0.0174532925);
    mx = cos((mdeg-90)*0.0174532925);    
    my = sin((mdeg-90)*0.0174532925);
    sx = cos((sdeg-90)*0.0174532925);    
    sy = sin((sdeg-90)*0.0174532925);

    if (ss==0 || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 32, 32, TFT_BLACK);
      ohx = hx*18+32;    
      ohy = hy*18+32;
      tft.drawLine(omx, omy, 32, 32, TFT_BLACK);
      omx = mx*23+32;    
      omy = my*23+32;
    }

      // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
      tft.drawLine(osx, osy, 32, 32, TFT_BLACK);
      tft.drawLine(ohx, ohy, 32, 32, TFT_WHITE);
      tft.drawLine(omx, omy, 32, 32, TFT_WHITE);
      osx = sx*23+32;    
      osy = sy*23+32;
      tft.drawLine(osx, osy, 32, 32, TFT_RED);

    tft.fillCircle(32, 32, 2, TFT_RED);

    print_intro(); 
  
}

void updateTime()
    {
      DateTime now = rtc.now();
      floatnow = float(now.hour()) + float(now.minute())/60 + float(now.second())/3600;
      //Serial.println(floatnow);
    }
void print_intro(){
  Serial.println("### Konfigurasi DS3231 Via Serial Command ###");
  Serial.println("    format -> Y,M,D,h,m,s");
  Serial.println("    contoh -> 2020,5,10,7,0,0");
  Serial.println("              10 mei 2020 jam 7:00");
  Serial.println();

    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    Serial.println();
  delay(1000);
}