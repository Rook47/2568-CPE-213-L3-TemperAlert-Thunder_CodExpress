ESP32 Smart Environment Monitor
📌 Project Overview

โครงการนี้เป็นระบบตรวจวัดสภาพแวดล้อมโดยใช้ ESP32 DOIT DEVKIT V1 ร่วมกับเซนเซอร์ DHT11 และจอแสดงผล OLED SSD1306
ระบบสามารถตรวจวัดอุณหภูมิและความชื้นแบบเรียลไทม์ พร้อมแสดงผลการแจ้งเตือนและโหมดการทำงานที่แตกต่างกัน

👥 Group Info

Group number: L3

Group name: Thunder-CodExpress

Members: 

นาย รัชพล ลองซุม – 2311310862

อภิโชค ม่วงกล่ำ – 2311310417

นรภัทร จันเอียบ – 2311340083

🎯 Project Detail / Requirement

ใช้ ESP32 DOIT Devkit V1 เป็นไมโครคอนโทรลเลอร์หลัก

เซนเซอร์ DHT11 สำหรับอ่านค่าอุณหภูมิและความชื้น

จอ OLED SSD1306 สำหรับแสดงผล

LED แดง/เขียว สำหรับบอกสถานะการทำงาน

ปุ่มกดสำหรับสลับโหมด

ต้องมีโหมดการทำงานอย่างน้อย 3 โหมด

⚙️ Project Specification

MCU: ESP32 DOIT Devkit V1

Sensor: DHT11 Temperature & Humidity

Display: OLED SSD1306 (I2C, 128x64)

LEDs: Red (GPIO 18), Green (GPIO 19)

Button: GPIO 13 (Pull-up)

Power: USB 5V

🏗️ System Architecture

Data Collection: DHT11 อ่านค่าอุณหภูมิ/ความชื้นทุก ๆ 1 วินาที

Processing: ESP32 ประมวลผลค่าที่อ่านได้ และตรวจสอบเงื่อนไข

Display & Alert: แสดงผลบนจอ OLED และควบคุม LED ตามโหมด

🖼️ Block Diagram

<img width="752" height="632" alt="image" src="https://github.com/user-attachments/assets/e6a5aadb-e2dc-47e7-87b5-89cd2feff6eb" />



🔌 Circuit Diagram
<img width="1565" height="517" alt="image" src="https://github.com/user-attachments/assets/3d8afe4d-8489-48fa-9f06-abfbd904819b" />

(ใส่วงจรที่ทำใน Fritzing / Tinkercad หรือวาดเอง)

💻 Codes

โค้ดทั้งหมดเก็บไว้ในโฟลเดอร์ /src

Mode 1 (Live): แสดงค่าแบบเรียลไทม์

Mode 2 (Alert): แจ้งเตือนเมื่ออุณหภูมิ/ความชื้นเกินกำหนด

Mode 3 (Quiz): เกมควิซคิดเลขเร็ว เก็บคะแนน + แสดงประวัติ

🎥 Demonstration VDO
https://drive.google.com/drive/folders/1EJ-RjSEnCcKYMFGtikMLgeuCsUhLjqVN

📘 Manual Report

กดปุ่มเพื่อสลับโหมด

สังเกตผลลัพธ์บนจอ OLED และไฟ LED

ใช้ Serial Monitor (115200) สำหรับโหมดควิซเพื่อตอบโจทย์

🛠️ Problem & Solution

ปัญหา: การเชื่อมต่อจอ OLED กับ ESP32 บางครั้งไม่ขึ้น → วิธีแก้: ตรวจสอบ Address I2C (0x3C) และการต่อสาย SDA/SCL

ปัญหา: ค่า DHT11 อ่านได้ NaN บ้าง → วิธีแก้: เพิ่ม delay และตรวจสอบ library ให้ตรงเวอร์ชัน

📅 Project Gantt Chart

<img width="941" height="461" alt="image" src="https://github.com/user-attachments/assets/fcafa56f-0ce2-46a4-9eda-0f4c2ea92793" />
