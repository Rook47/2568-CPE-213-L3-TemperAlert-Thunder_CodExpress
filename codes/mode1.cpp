#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "globals.h"   // จะทำไฟล์นี้เก็บตัวแปรกลางทีหลัง

void runMode1() {
  if (millis() - tRead >= 1000) {
    tRead = millis();
    readDHT();
    showLive();
  }
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
}

void showLive() {
  drawHeader("LIVE");
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print("T:");
  if (isnan(tempC)) display.print("--");
  else display.print((int)round(tempC));
  display.print((char)247); display.print("C");

  display.setCursor(0, 40);
  display.print("H:");
  if (isnan(humi)) display.print("--");
  else display.print((int)round(humi));
  display.print("%");

  drawMascotAnimated(comfortable(tempC, humi));
  drawFooterHint();
  display.display();
}
