//I forgot the Firebase version — I’m sorry if Firebase doesn’t work, so please this repo for reference only.

#include <FirebaseESP32.h>
#include <WiFi.h>
#include <DHT.h>

// Wifi
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase
#define FIREBASE_HOST "YOUR_FIREBASE_HOST_URL"
#define FIREBASE_AUTH "YOUR_FIREBASE_AUTH"

// DHT
#define DHTPIN 4
#define DHTTYPE DHT11

// LED
#define LED_PIN 2
#define LED2_PIN 13
#define LED3_PIN 12
#define LED4_PIN 14

DHT dht(DHTPIN, DHTTYPE);

// Config Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastTempUpdate = 0;


void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi đã kết nối!");

  // Start config Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Init Firebase
  Firebase.begin(&config, &auth);

  // Check connection Firebase
  if (Firebase.ready()) {
    Serial.println("Firebase ready!");
  } else {
    Serial.println("Firebase not connected!");
  }

  // Init DHT 
  dht.begin();

  // Config LED IN/OUT
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);


  // Init LED Low
  digitalWrite(LED_PIN, LOW); 
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);  
  digitalWrite(LED4_PIN, LOW); 
}


void loop() {
  // Refresh temperature & humidity every 5s
  if (millis() - lastTempUpdate >= 5000) {
    lastTempUpdate = millis(); // Reset counter

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      String tempStr = String(t) + " °C";
      String humStr = String(h) + " %";

      Serial.printf("Temperature: %s, Humidity: %s\n", tempStr.c_str(), humStr.c_str());

      // Send data to Firebase
      FirebaseJson json;
      json.set("/Temperature", tempStr);
      json.set("/Humidity", humStr);

      if (Firebase.ready()) {
        if (Firebase.updateNode(fbdo, "/SensorData", json)) {
          Serial.println("Data has been updated to Firebase!");
        } else {
          Serial.printf("Error send data to Firebase: %s\n", fbdo.errorReason().c_str());
        }
      } else {
        Serial.println("Firebase not ready!");
      }
    } else {
      Serial.println("Cant read data from DHT11!");
    }
  }

  // Read control LED state from Firebase
  // bool ledStates[4] = {false, false, false, false};

  // if (Firebase.getBool(fbdo, "/LEDControl/LED") && fbdo.dataType() == "boolean") ledStates[0] = fbdo.boolData();
  // if (Firebase.getBool(fbdo, "/LEDControl/LED2") && fbdo.dataType() == "boolean") ledStates[1] = fbdo.boolData();
  // if (Firebase.getBool(fbdo, "/LEDControl/LED3") && fbdo.dataType() == "boolean") ledStates[2] = fbdo.boolData();
  // if (Firebase.getBool(fbdo, "/LEDControl/LED4") && fbdo.dataType() == "boolean") ledStates[3] = fbdo.boolData();

  int ledStates[4] = {0, 0, 0, 0}; // Default

  if (Firebase.getInt(fbdo, "/LEDControl/LED1")) {
    if (fbdo.dataType() == "int") {
      ledStates[0] = fbdo.intData();
      Serial.printf("LED1 state: %d\n", ledStates[0]);
    }
  } else {
    Serial.printf("Error read LED1: %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.getInt(fbdo, "/LEDControl/LED2")) {
    if (fbdo.dataType() == "int") {
      ledStates[1] = fbdo.intData();
      Serial.printf("LED2 state: %d\n", ledStates[1]);
    }
  } else {
    Serial.printf("Error read LED2: %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.getInt(fbdo, "/LEDControl/LED3")) {
    if (fbdo.dataType() == "int") {
      ledStates[2] = fbdo.intData();
      Serial.printf("LED3 state: %d\n", ledStates[2]);
    }
  } else {
    Serial.printf("Error read LED3: %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.getInt(fbdo, "/LEDControl/LED4")) {
    if (fbdo.dataType() == "int") {
      ledStates[3] = fbdo.intData();
      Serial.printf("LED4 state: %d\n", ledStates[3]);
    }
  } else {
    Serial.printf("Error read LED4: %s\n", fbdo.errorReason().c_str());
  }


  // Update LED state
  digitalWrite(LED_PIN, ledStates[0] ? HIGH : LOW);
  digitalWrite(LED2_PIN, ledStates[1] ? HIGH : LOW);
  digitalWrite(LED3_PIN, ledStates[2] ? HIGH : LOW);
  digitalWrite(LED4_PIN, ledStates[3] ? HIGH : LOW);

  delay(100); 
}
