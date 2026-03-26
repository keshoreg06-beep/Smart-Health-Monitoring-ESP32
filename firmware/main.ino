#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL3tmhpVbmy"
#define BLYNK_TEMPLATE_NAME "iot with max"
#define BLYNK_AUTH_TOKEN "rQcg4r_ev7YqCCFbsLUWrLWCl3uIxbLb"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "DHT.h"

// ---------------- WiFi ----------------
char ssid[] = "Keerthiop";
char pass[] = "podabaadu";

// ---------------- Pins ----------------
#define BUZZER_PIN 18

// AD8232 ECG
#define ECG_PIN 34
#define LO_PLUS 26
#define LO_MINUS 27

// DHT11
#define DHTPIN 4
#define DHTTYPE DHT11

// MAX30102
#define IR_THRESHOLD 5000

// ---------------- Objects ----------------
MAX30105 particleSensor;
BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);

// ---------------- Variables ----------------
float bpm = 0;
long lastBeat = 0;

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int bufferIndex = 0;

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

float temperature = 0;
float humidity = 0;

bool alertSent = false;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 30000;

// -------- ECG FILTER VARIABLES (Hospital Style) --------
const int ECG_OFFSET = 512;      // Center value
const int FILTER_SIZE = 8;       // Moving average window

int ecgFilterBuffer[FILTER_SIZE];
int filterIndex = 0;
long filterSum = 0;

// ---------------- FUNCTIONS ----------------

// 🏥 Hospital-style ECG waveform
void readECG() {
  if (digitalRead(LO_PLUS) == 1 || digitalRead(LO_MINUS) == 1) {
    Serial.println(ECG_OFFSET);   // flat centered line
    Blynk.virtualWrite(V4, 0);
    return;
  }

  // Raw ADC read
  int rawECG = analogRead(ECG_PIN);

  // Scale 12-bit ADC → 10-bit
  rawECG = rawECG / 4;   // 0–1023

  // -------- Moving Average Filter --------
  filterSum -= ecgFilterBuffer[filterIndex];
  ecgFilterBuffer[filterIndex] = rawECG;
  filterSum += rawECG;

  filterIndex++;
  if (filterIndex >= FILTER_SIZE) filterIndex = 0;

  int filteredECG = filterSum / FILTER_SIZE;

  // -------- Baseline Centering --------
  int ecgSignal = filteredECG - ECG_OFFSET;

  // -------- Gain (adjustable) --------
  ecgSignal = ecgSignal * 2;

  // -------- Output --------
  Serial.println(ecgSignal);       // ONLY ECG → Serial Plotter
  Blynk.virtualWrite(V4, ecgSignal);
}

// 🔵 MAX30102
void readMAX30102() {
  long irValue = particleSensor.getIR();

  if (irValue < IR_THRESHOLD) {
    bpm = 0;
    validSPO2 = 0;
    digitalWrite(BUZZER_PIN, LOW);
    alertSent = false;
    return;
  }

  redBuffer[bufferIndex] = particleSensor.getRed();
  irBuffer[bufferIndex] = irValue;
  bufferIndex++;

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    bpm = 60.0 / (delta / 1000.0);
  }

  if (bufferIndex >= 100) {
    maxim_heart_rate_and_oxygen_saturation(
      irBuffer, 100, redBuffer,
      &spo2, &validSPO2,
      &heartRate, &validHeartRate
    );

    bufferIndex = 0;

    if ((bpm < 50 || bpm > 120 || (validSPO2 && spo2 < 92)) &&
        !alertSent &&
        millis() - lastAlertTime > ALERT_COOLDOWN) {

      digitalWrite(BUZZER_PIN, HIGH);
      Blynk.logEvent("heart_alert",
        String("BPM: ") + bpm + " | SpO2: " + spo2);

      alertSent = true;
      lastAlertTime = millis();
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      alertSent = false;
    }
  }
}

// 🟢 DHT11
void readDHT11() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) return;

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

// 📝 Serial Monitor (text only)
void printOtherValues() {
  Serial.println("------ VITAL SIGNS ------");
  Serial.print("BPM: "); Serial.println(bpm);

  if (validSPO2) {
    Serial.print("SpO2: "); Serial.print(spo2); Serial.println(" %");
  } else {
    Serial.println("SpO2: --");
  }

  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.println("-------------------------\n");
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LO_PLUS, INPUT);
  pinMode(LO_MINUS, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  dht.begin();

  // Timers
  timer.setInterval(5L, readECG);        // 200 Hz ECG (hospital-like)
  timer.setInterval(20L, readMAX30102);
  timer.setInterval(3000L, readDHT11);
  timer.setInterval(5000L, printOtherValues);
}

// ---------------- LOOP ----------------
void loop() {
  Blynk.run();
  timer.run();
}
