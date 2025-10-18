#include <WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"
#include <ThingSpeak.h>

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PWD "YOUR_WIFI_PASSWORD"

#define WRITEAPIKEY "YOUR_THINGSPEAK_KEY" 
#define CHANNEL_ID 2781313 // YOUR_CHANNEL_ID

#define DHT_DAT_PIN 4
#define DHTTYPE DHT11

DHT dht(DHT_DAT_PIN, DHTTYPE);

WiFiClient client;

void setup() {
  Serial.begin(9600);
  Serial.println("\nESP32 starting...");

  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  ThingSpeak.begin(client);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" °C, Humidity: ");
    Serial.print(h);
    Serial.println(" %");

    ThingSpeak.setField(1, t); 
    ThingSpeak.setField(2, h);

    int responseCode = ThingSpeak.writeFields(CHANNEL_ID, WRITEAPIKEY);
    if (responseCode == 200) {
      Serial.println("Data sent to ThingSpeak successfully.");
    } else {
      Serial.print("Failed to send data to ThingSpeak. Error code: ");
      Serial.println(responseCode);
    }
  }
  delay(15000);
}
