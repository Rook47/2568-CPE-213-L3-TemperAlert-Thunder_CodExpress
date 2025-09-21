#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "main.h"

// ---------- สร้างโจทย์ใหม่ ----------
void newQuestion() {
  aQ = random(0, 10);
  bQ = random(0, 10);
  ansQ = aQ + bQ;
  inputBuf = "";
  Serial.printf("\n[Quiz] %d + %d = ?\nAnswer: ", aQ, bQ);
}

// ---------- หน้าจอ QUIZ Ready ----------
void showQuizReady() {
  drawHeader("QUIZ (Ready)");
  display.setTextSize(2);
  display.setCursor(0, 20); display.print("Hold to");
  display.setCursor(0, 40); display.print("Start");
  drawFooterHint("Hold button ~0.8s");
  display.display();
}

// ---------- หน้าจอ QUIZ Running ----------
void showQuizRunning(unsigned long remainMs) {
  drawHeader("QUIZ");
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(aQ); display.print("+"); display.print(bQ); display.print("=?");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Remain: "); display.print(remainMs/1000); display.print(" s");

  display.setCursor(0, 50);
  display.print("Score: "); display.print(score);

  drawMascotAnimated(true);  // การ์ตูนยิ้มตลอดเวลา
  drawFooterHint("Type in Serial (115200)");
  display.display();
}

// ---------- หน้าจอ QUIZ Final ----------
void showQuizFinal() {
  drawHeader("QUIZ FINISH");
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print("SCORE:");
  display.setCursor(0, 36);
  display.print(score);

  display.setTextSize(1);
  display.setCursor(64, 0);
  display.print("Last3:");
  display.setCursor(64, 10);
  display.print(lastPlayersStr());   // เช่น "5 3 7"

  drawMascotAnimated(true);
  drawFooterHint("Hold: restart | Press: change mode");
  display.display();
}

// ---------- ฟังก์ชันรันโหมด QUIZ ----------
void runMode3() {
  // หลังจบรอบ
  if (roundFinished) {
    showQuizFinal();
    return;
  }

  // ยังไม่เริ่ม
  if (!roundRunning) {
    showQuizReady();
    return;
  }

  // ระหว่างรอบ
  unsigned long elapsed = millis() - roundStart;
  if (elapsed < ROUND_MS) {
    unsigned long remain = ROUND_MS - elapsed;
    showQuizRunning(remain);
  } else {
    // จบรอบ
    roundRunning = false;
    roundFinished = true;
    pushPlayerHistory(score);
    Serial.printf("\n[Quiz] ROUND FINISH  SCORE=%d  Last3=%s\n",
                  score, lastPlayersStr().c_str());
    showQuizFinal();
  }
}
