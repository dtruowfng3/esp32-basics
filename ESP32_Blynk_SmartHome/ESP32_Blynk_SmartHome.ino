// #define BLYNK_TEMPLATE_ID 
// #define BLYNK_TEMPLATE_NAME 
// #define BLYNK_AUTH_TOKEN 

#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define PIRPIN 13 

// Config wifi
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

#define LED 26
#define BUZZER 25
#define RELAY 27
#define LEDRED_PIN 16
#define LEDBLUE_PIN 17

extern float t_thresholdWARNING = 25;

extern float h_thresholdHIGH = 80;
extern float t_limitLOW = 10;
extern float t_limitHIGH = 50;

BlynkTimer timer;

bool pirState = false;
bool pirEnabled = true;

// Variables to control LED blinking
unsigned long previousMillis_LED = 0;
const long interval_LED = 500; 
bool ledState = false;     // Tracks current LED state

BLYNK_WRITE(V5) {
  pirEnabled = param.asInt(); 
  if (pirEnabled) {
    Serial.println("PIR Sensor Enabled");
  } else {
    digitalWrite(BUZZER, LOW);
    Serial.println("PIR Sensor Disabled");
  }
}

BLYNK_WRITE(V0) {
  int value = param.asInt(); 
  value ? digitalWrite(LED, HIGH) : digitalWrite(LED, LOW); 
}

// Hàm điều khiển Relay từ ứng dụng Blynk
BLYNK_WRITE(V1) {
  int value = param.asInt(); 
  value ? digitalWrite(RELAY, HIGH) : digitalWrite(RELAY, LOW); 
}

void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C  Humidity: ");
  Serial.print(h);
  Serial.println(" %");

  Blynk.virtualWrite(V3, h); 
  Blynk.virtualWrite(V2, t); 

  unsigned long currentMillis_LED = millis();
  if (currentMillis_LED - previousMillis_LED >= interval_LED) {
    previousMillis_LED = currentMillis_LED;
    ledState = !ledState;

    if (t > t_thresholdWARNING) {
      digitalWrite(LEDRED_PIN, ledState);
      Serial.println("Red LED blynk ");
    } else {
      digitalWrite(LEDRED_PIN, LOW);
    }

    if ((t > t_limitLOW || t < t_limitHIGH) && (h < h_thresholdHIGH)) {
      digitalWrite(LEDBLUE_PIN, HIGH);
      Serial.println("Blue LED ON"); 
    } else {
      digitalWrite(LEDBLUE_PIN, LOW);
    }
  }
}

void checkMotion() {
  if (pirEnabled) { 
    pirState = digitalRead(PIRPIN); 

    if (pirState == HIGH) {
      digitalWrite(BUZZER, HIGH);
      Blynk.virtualWrite(V4, "Motion Detected!"); 
    } else {
      digitalWrite(BUZZER, LOW);
      Blynk.virtualWrite(V4, "No motion detected");
    }
  }
  else
    Blynk.virtualWrite(V4, "PIR Disabled");
}

// Hàm setup
void setup() {
  Serial.begin(115200);
  delay(1000);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(PIRPIN, INPUT);
  pinMode (LEDRED_PIN, OUTPUT);
  pinMode (LEDBLUE_PIN, OUTPUT);
  dht.begin();

  timer.setInterval(1000L, sendSensor);
  timer.setInterval(500L, checkMotion); 
}

// Hàm loop
void loop() {
  Blynk.run();
  timer.run(); 
}

