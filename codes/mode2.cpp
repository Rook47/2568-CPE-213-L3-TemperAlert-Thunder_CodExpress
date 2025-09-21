#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "main.h"   // ใช้เพื่ออ้างอิงตัวแปร global เช่น tempC, humi, display ฯลฯ

// ----------- ฟังก์ชันแสดงผลโหมด ALERT -----------
void showAlert(bool highT, bool highH) {
  drawHeader("ALERT");
  display.setTextSize(2);
  display.setCursor(0, 16);
  if (highT) display.print("HIGH TEMP!");
  else if (highH) display.print("HIGH HUMID!");
  else display.print("OK");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("T="); if (isnan(tempC)) display.print("--"); else display.print(tempC,1);
  display.print("C  H="); if (isnan(humi)) display.print("--"); else display.print(humi,0);
  display.print("%");

  drawMascotAnimated(comfortable(tempC, humi));  // ถ้าไม่สบายจะขยับเอง
  drawFooterHint();
  display.display();
}

// ----------- ฟังก์ชันรันโหมด ALERT -----------
void runMode2() {
  if (millis() - tRead >= 1000) {
    tRead = millis();
    readDHT();
  }
  bool highT = (!isnan(tempC) && tempC > TEMP_HIGH);
  bool highH = (!isnan(humi)  && humi  > HUMI_HIGH);
  bool alert = highT || highH;

  if (alert) {
    if (millis() - tBlink >= 200) {           // LED แดงกระพริบเร็ว
      tBlink = millis(); ledFlip = !ledFlip;
      digitalWrite(LED_RED, ledFlip);
    }
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  }
  showAlert(highT, highH);
}


