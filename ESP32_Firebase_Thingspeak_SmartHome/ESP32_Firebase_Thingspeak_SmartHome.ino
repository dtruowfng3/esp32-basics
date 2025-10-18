#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define FIREBASE_HOST "YOUR_FIREBASE_HOST_URL"
#define FIREBASE_AUTH "YOUR_FIREBASE_AUTH"

// config ThingSpeak
#define THINGSPEAK_API_KEY "YOUR_THINGSPEAK_API_KEY"
#define THINGSPEAK_CHANNEL_ID xxxxxx // YOUR_THINGSPEAK_CHANNEL_ID

#define DHT_PIN 4
#define DHT_TYPE DHT11

// Config PIR PIN
#define PIR_PIN 13
#define BUZZER 25

#define LED_PIN 26

DHT dht(DHT_PIN, DHT_TYPE);

WiFiClient client;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long previousMillis = 0;
const unsigned long thingSpeakInterval = 15000; 

void setup() {
  Serial.begin(115200);

  dht.begin();

  // Kết nối Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Init ThingSpeak
  ThingSpeak.begin(client);

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);

  if (Firebase.ready()) {
    Serial.println("Firebase is ready!");
  } else {
    Serial.println("Failed to connect to Firebase!");
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); 
}

void loop() {
  int pirState = digitalRead(PIR_PIN);

  String motionStatus; 

  if (pirState == HIGH) { 
    motionStatus = "Detected";
    digitalWrite(BUZZER, HIGH);
    Serial.println("Motion detected!");
  } else {
    motionStatus = "No detected";
    digitalWrite(BUZZER, LOW);
    Serial.println("No motion detected.");
  }

  if (Firebase.ready()) {
    if (Firebase.setString(fbdo, "/MotionStatus", motionStatus)) {
      Serial.printf("Motion status updated to Firebase: %s\n", motionStatus.c_str());
    } else {
      Serial.printf("Failed to update motion status: %s\n", fbdo.errorReason().c_str());
    }
  }

  // Send sensor state to ThingSpeak
  ThingSpeak.setField(3, (pirState == HIGH) ? 1 : 0); // Field 3: 1 = Detected, 0 = No detected

  if (Firebase.getBool(fbdo, "/LEDControl")) {
    if (fbdo.dataType() == "boolean") {
      bool ledState = fbdo.boolData();
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      Serial.printf("LED State: %s\n", ledState ? "ON" : "OFF");
    }
  } else {
    Serial.printf("Error reading LED state from Firebase: %s\n", fbdo.errorReason().c_str());
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= thingSpeakInterval) {
    previousMillis = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Cant read DHT11!");
    } else {
      Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%\n", t, h);

      // Gửi dữ liệu lên Firebase
      if (Firebase.ready()) {
        FirebaseJson json;
        json.set("/Temperature", t);
        json.set("/Humidity", h);

        if (Firebase.updateNode(fbdo, "/SensorData", json)) {
          Serial.println("Data has been send to Firebase!");
        } else {
          Serial.printf("Error send to Firebase: %s\n", fbdo.errorReason().c_str());
        }
      }

      // Gửi dữ liệu lên ThingSpeak
      ThingSpeak.setField(1, t);
      ThingSpeak.setField(2, h);
      int response = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_KEY);

      if (response == 200) {
        Serial.println("Data has been send to ThingSpeak succesfull!");
      } else {
        Serial.printf("Error send data to ThingSpeak: %d\n", response);
      }
    }
  }
  delay(2000);
}
