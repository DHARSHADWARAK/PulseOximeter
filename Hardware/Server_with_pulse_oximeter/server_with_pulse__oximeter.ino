#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "MAX30100_PulseOximeter.h"

// ==== Wi-Fi config ====
const char* ssid     = "vivo T1 5G";     
const char* password = "1234567890";

// ==== HTTP server ====
WebServer server(80);

// ==== MAX30100 ====
#define REPORTING_PERIOD_MS 2000
PulseOximeter pox;

float a, b;
int i = 10;       // countdown
float c = 0;      // running avg heart rate
uint32_t tsLastReport = 0;

// ==== Data to send ====
String hrData = "0";
String spo2Data = "0";
int countdownVal = 10;

void onBeatDetected() {
  // Serial.println("STAY STILL Measuring...");
}

// ==== CORS helper ====
void sendCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  sendCORS();
  server.send(204); // No Content
}

// ==== Return data as JSON ====
void handleDataGet() {
  sendCORS();

  DynamicJsonDocument doc(256);
  doc["countdown"] = countdownVal;
  doc["heartRate"] = hrData;
  doc["spo2"] = spo2Data;

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);

  // --- MAX30100 init ---
  Serial.println("Initializing MAX30100...");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // --- Wi-Fi ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // --- HTTP routes ---
  server.on("/data", HTTP_OPTIONS, handleOptions);
  server.on("/data", HTTP_GET, handleDataGet);

  // simple root check
  server.on("/", HTTP_GET, []() {
    sendCORS();
    server.send(200, "text/plain", "ESP32 MAX30100 server running");
  });

  server.begin();
}

void loop() {
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    a = pox.getHeartRate();
    b = pox.getSpO2();

    if (a == 0 || b == 0) {
      server.handleClient();
      return;
    }

    if (i > 0) {
      c = ((c + a) / 2);
      i = i - 1;
      Serial.println(i);
      countdownVal = i;
    }

    if (i == 0) {
      i = 10;
      hrData = String(c, 1);
      spo2Data = String(b, 1);
      countdownVal = i;

      Serial.print("Heart rate: ");
      Serial.print(hrData);
      Serial.print(" bpm & SpO2: ");
      Serial.print(spo2Data);
      Serial.println("%");

      c = 0;
    }

    tsLastReport = millis();
  }

  server.handleClient();
}
