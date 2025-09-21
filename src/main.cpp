// === 3 Modes (ESP32 DOIT DEVKIT V1) ===
// Mode1 : LIVE  — แสดง DHT11 แบบเรียลไทม์ + การ์ตูนตัวเดียว (ส่ายเมื่อไม่สบายตัว)
// Mode2 : ALERT — เกินเกณฑ์ → LED แดงกระพริบ 200ms + การ์ตูนขยับ (ไม่สบาย)
// Mode3 : QUIZ  — รอบละ 30 วินาที/คน ตอบคิดเลขเร็ว เก็บคะแนน + ประวัติ 3 คนล่าสุด (พักรอเริ่มใหม่)
//
// Pins
//   LED_RED   -> GPIO 18
//   LED_GREEN -> GPIO 19
//   BUTTON    -> GPIO 13  (กดลง GND, INPUT_PULLUP)
//   DHT11     -> GPIO 15
//   OLED I2C  -> SDA=21, SCL=22 (0x3C)
//
// Libraries: Adafruit GFX, Adafruit SSD1306, DHT sensor library (by Adafruit)

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- Pins ----------------
#define LED_RED    18
#define LED_GREEN  19
#define BUTTON_PIN 13
#define DHTPIN     15
#define DHTTYPE    DHT11

// -------------- OLED setup ------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------------- DHT setup -------------
DHT dht(DHTPIN, DHTTYPE);
float tempC = NAN, humi = NAN;

// -------------- Modes -----------------
enum Mode { MODE_1, MODE_2, MODE_3 };
Mode mode = MODE_1;

// --------- timing / blink -------------
unsigned long tRead  = 0;          // อ่าน DHT ทุก 1 วิ (โหมด 1/2)
unsigned long tBlink = 0;          // กระพริบ LED / animation
bool ledFlip = false;              // ใช้ทั้ง animation และ LED แดง

// --------------- Button ---------------
const unsigned long DEBOUNCE_MS = 30;
const unsigned long LONG_MS     = 800;   // กดค้าง ~0.8s
bool btnPrev = HIGH;
unsigned long btnStamp = 0;

// --------- ALERT thresholds -----------
const float TEMP_HIGH = 30.0;
const float HUMI_HIGH = 80.0;

// --------- Bitmaps ---------
// การ์ตูน 16x16 (ตัวเดียว ใช้ทุกโหมด)
const uint8_t PROGMEM BMP_MASCOT[] = {
  0x00,0x00,0x3C,0x3C,0x42,0x42,0xA1,0x81,
  0xA1,0x81,0xA1,0x81,0x42,0x42,0x3C,0x3C,
  0x00,0x00,0x00,0x00,0x18,0x18,0x24,0x24,
  0x42,0x42,0x42,0x42,0x24,0x24,0x18,0x18
};
// ไอคอนติ๊กถูก 8x8 (โชว์ตอนตอบถูก)
const uint8_t PROGMEM BMP_TICK[] = {
  0x00,0x18,0x1C,0x8E,0xC7,0x62,0x20,0x00
};

// -------------------- QUIZ (Mode 3: รอบ 30 วิ/คน) --------------------
const uint32_t ROUND_MS = 30000;          // รอบละ 30 วิ
bool roundRunning = false;                 // กำลังเล่นรอบอยู่
bool roundFinished = false;                // เพิ่งจบรอบ และกำลังโชว์ผล
unsigned long roundStart = 0;              // เวลาเริ่มรอบ

int aQ = 0, bQ = 0, ansQ = 0;             // โจทย์ปัจจุบัน
String inputBuf;                           // buffer คำตอบจาก Serial
int score = 0;                             // คะแนนของรอบนี้
unsigned long tickShowUntil = 0;           // แสดงติ๊กถูกหลังตอบถูก

// ประวัติ 3 คนล่าสุด (รอบ 30 วิก่อนหน้า) ค่าเริ่มต้น -1 = ยังไม่มี
int lastPlayers[3] = {-1, -1, -1};
void pushPlayerHistory(int sc) { lastPlayers[2]=lastPlayers[1]; lastPlayers[1]=lastPlayers[0]; lastPlayers[0]=sc; }
String lastPlayersStr() {
  auto mapv = [](int x)->String { return (x<0 ? String("-") : String(x)); };
  return mapv(lastPlayers[0]) + " " + mapv(lastPlayers[1]) + " " + mapv(lastPlayers[2]);
}

// ---------- helpers ----------
bool comfortable(float t, float h) {
  if (isnan(t) || isnan(h)) return false;
  return (t >= 22 && t <= 30 && h >= 30 && h <= 70);
}

void drawFooterHint(const char* txt="Press: change mode") {
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(txt);
}

void drawHeader(const char* title) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(title);
}

// ------ DHT ------
void readDHT() {
  tempC = dht.readTemperature();
  humi  = dht.readHumidity();
}

// ------ Mascot draw (animate on uncomfortable) ------
void drawMascotAnimated(bool comfy) {
  // ถ้าไม่สบายตัว ให้ส่ายซ้าย-ขวาเล็ก ๆ สลับทุก ~300ms
  int baseX = 96, y = 16;
  int dx = 0;
  if (!comfy) {
    if (millis() - tBlink >= 300) { tBlink = millis(); ledFlip = !ledFlip; }
    dx = ledFlip ? -1 : 1;
  }
  display.drawBitmap(baseX + dx, y, BMP_MASCOT, 16, 16, SSD1306_WHITE);

  // ตอบถูก แสดงติ๊กถูกข้าง ๆ ชั่วครู่
  if (millis() < tickShowUntil) {
    display.drawBitmap(baseX + 18, y + 2, BMP_TICK, 8, 8, SSD1306_WHITE);
  }
}

// -------------------- OLED screens --------------------
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

// ---- QUIZ screens ----
void newQuestion() {
  aQ = random(0, 10);
  bQ = random(0, 10);
  ansQ = aQ + bQ;
  inputBuf = "";
  Serial.printf("\n[Quiz] %d + %d = ?\nAnswer: ", aQ, bQ);
}

void showQuizReady() {
  drawHeader("QUIZ (Ready)");
  display.setTextSize(2);
  display.setCursor(0, 20); display.print("Hold to");
  display.setCursor(0, 40); display.print("Start");
  drawFooterHint("Hold button ~0.8s");
  display.display();
}

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

  drawMascotAnimated(true);  // ระหว่างควิซให้การ์ตูนคงที่/ยิ้ม
  drawFooterHint("Type in Serial (115200)");
  display.display();
}

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
  drawFooterHint("Hold: restart  |  Press: change mode");
  display.display();
}

// -------------------- Mode runners --------------------
void runMode1() {
  if (millis() - tRead >= 1000) {
    tRead = millis();
    readDHT();
    showLive();
  }
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
}

void runMode2() {
  if (millis() - tRead >= 1000) {
    tRead = millis();
    readDHT();
  }
  bool highT = (!isnan(tempC) && tempC > TEMP_HIGH);
  bool highH = (!isnan(humi)  && humi  > HUMI_HIGH);
  bool alert = highT || highH;

  if (alert) {
    if (millis() - tBlink >= 200) {           // แดงกระพริบเร็ว
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

void runMode3() {
  // พักแสดงผลหลังจบรอบ จนกว่าจะกดเริ่มใหม่เอง
  if (roundFinished) {
    showQuizFinal();
    return;
  }

  // ยังไม่เริ่มรอบ
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
    pushPlayerHistory(score);     // เก็บคะแนนลงประวัติ 3 คนล่าสุด
    Serial.printf("\n[Quiz] ROUND FINISH  SCORE=%d  Last3=%s\n",
                  score, lastPlayersStr().c_str());
    showQuizFinal();
  }
}

// -------------------- Button handler --------------------
// กดสั้น  = เปลี่ยนโหมด
// กดค้าง ~0.8s (เฉพาะโหมด 3) = เริ่ม "รอบใหม่" (รีเซ็ตคะแนน/เวลา)
void handleButton() {
  bool now = digitalRead(BUTTON_PIN);
  if (now != btnPrev && (millis() - btnStamp) > DEBOUNCE_MS) {
    btnStamp = millis();
    btnPrev = now;

    if (now == LOW) {
      unsigned long p0 = millis();
      while (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long hold = millis() - p0;
        if (mode == MODE_3 && hold >= LONG_MS) {
          while (digitalRead(BUTTON_PIN) == LOW) delay(1); // รอปล่อย
          // เริ่มรอบใหม่เสมอ (ไม่ว่าจะอยู่สถานะไหน)
          score = 0;
          roundStart = millis();
          roundRunning = true;
          roundFinished = false;
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_GREEN, HIGH);
          newQuestion();
          return;
        }
        delay(1);
      }

      // กดสั้น → เปลี่ยนโหมด
      mode = (Mode)((mode + 1) % 3);

      // ออกจากโหมด 3: รีเซ็ตสถานะควิซ
      if (mode != MODE_3) {
        roundRunning = false;
        roundFinished = false;
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
      }
    }
  }
}

// -------------------- Setup / Loop --------------------
void setup() {
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(21, 22);                 // I2C: SDA=21, SCL=22
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("BOOTING...");
  display.display();

  Serial.begin(115200);
  delay(100);
  Serial.println("\n[Mode3 Quiz] เปิด Serial Monitor ที่ 115200");
  Serial.println("กดปุ่มค้าง ~0.8s เพื่อเริ่ม 'รอบใหม่ 30 วิ' | ตอบในบรรทัด Answer: แล้วกด Enter");

  dht.begin();
  randomSeed(esp_random());
  delay(400);
}

void loop() {
  // อ่านคำตอบเฉพาะตอน "รอบกำลังเล่น"
  if (mode == MODE_3 && roundRunning) {
    while (Serial.available()) {
      char c = (char)Serial.read();
      if (c == '\r') continue;            // ข้าม CR
      if (c == '\n') {                    // ส่งคำตอบแล้ว
        int user = inputBuf.toInt();
        bool correct = (user == ansQ);
        if (correct) {
          score += 1;
          tickShowUntil = millis() + 800; // ติ๊กถูกเล็ก ๆ ข้างการ์ตูน
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(LED_RED, LOW);
          Serial.printf("[OK] %d + %d = %d | your=%d | score=%d\n", aQ, bQ, ansQ, user, score);
        } else {
          for (int i=0; i<3; ++i) { digitalWrite(LED_RED, HIGH); delay(60); digitalWrite(LED_RED, LOW); delay(60); }
          Serial.printf("[WRONG] %d + %d = %d | your=%d | score=%d\n", aQ, bQ, ansQ, user, score);
        }
        inputBuf = "";
        newQuestion();                    // ไปข้อถัดไปทันที (ยังนับเวลารอบเดิมต่อ)
      } else {
        if (isDigit(c) || c=='-') { inputBuf += c; Serial.write(c); } // echo
      }
    }
  }

  handleButton();

  switch (mode) {
    case MODE_1: runMode1(); break;
    case MODE_2: runMode2(); break;
    case MODE_3: runMode3(); break;
  }
}
