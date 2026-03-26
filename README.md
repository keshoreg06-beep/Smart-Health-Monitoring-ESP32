# 🏥 IoT-Based Smart Health Monitoring System Using ESP32

> VIT Invention Disclosure Format (IDF-B) | Document No: 02-IPR-R003

## 📌 Overview
A low-cost IoT-based health monitoring system using the ESP32 microcontroller
that simultaneously monitors:
- ❤️ Heart Rate (BPM) & SpO₂ — via MAX30102
- 📈 ECG Signal — via AD8232
- 🌡️ Temperature & Humidity — via DHT11

All data streams in real-time to a **Blynk cloud dashboard** via Wi-Fi,
with automatic buzzer + cloud alerts for abnormal vitals.

---

## 🔧 Hardware Components
| Component | Purpose | Pin |
|-----------|---------|-----|
| ESP32 | Microcontroller + WiFi | — |
| MAX30102 | Heart Rate & SpO₂ | GPIO 21/22 (I2C) |
| AD8232 | ECG Signal | GPIO 34, 26, 27 |
| DHT11 | Temperature & Humidity | GPIO 4 |
| Buzzer | Alert | GPIO 18 |

---

## 📊 Alert Thresholds
- BPM < 50 or > 120 → Buzzer ON + Cloud Alert
- SpO₂ < 92% → Buzzer ON + Cloud Alert
- 30-second cooldown to prevent repeated alerts

---

## 📱 Blynk Virtual Pins
| Pin | Data |
|-----|------|
| V2 | Temperature |
| V3 | Humidity |
| V4 | ECG Waveform |

---

## 🚀 How to Flash
1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support
3. Install libraries: `Blynk`, `MAX30105`, `DHT sensor library`
4. Open `firmware/main.ino`
5. Update WiFi credentials and Blynk Auth Token
6. Upload to ESP32

---

## 📈 Sample Output
```
------ VITAL SIGNS ------
BPM: 72.40
SpO₂: 97 %
Temperature: 36.70 °C
Humidity: 54.50 %
```

---

## 🧪 Validation Results
- ✅ 25 SAFE state test cases — all PASSED
- ✅ 25 ACTIVE alert test cases — all PASSED
- TRL Level: 1–5 + 7 (System prototype demonstrated)

---

## ⚠️ Important
Before uploading, replace credentials in `main.ino`:
```cpp
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";
#define BLYNK_AUTH_TOKEN "YOUR_TOKEN"
```

---

## 📄 License
This project is submitted as an Invention Disclosure at VIT. 
All rights reserved © VIT IPR & TT Cell.
