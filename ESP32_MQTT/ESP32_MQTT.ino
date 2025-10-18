 #include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include <DHT.h> 

#define IO_USERNAME  "YOUR_IO_NAME"
#define IO_KEY       "YOUR_IO_KEY"

#define WIFI_SSID "YOUR_WIFI_NAME" 
#define WIFI_PASS "YOUR_WIFI_PASSWORD"  

#define LED_PIN 26      // LED  GPIO 26
#define LED2_PIN 27     // LED2 GPIO 27

#define DHTPIN 4      // GPIO 4
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

// Adafruit IO & Feed
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *digital = io.feed("digital");
AdafruitIO_Feed *digital2 = io.feed("digital2"); 
AdafruitIO_Feed *temperatureFeed = io.feed("temperature"); 
AdafruitIO_Feed *humidityFeed = io.feed("humidity");  

unsigned long lastUpdateTime = 0;  
const unsigned long dhtInterval = 5000;  

unsigned long previousMillis_LED = 0;
const long interval_LED = 500; 
bool ledState = false;     

void setup() {
  pinMode(LED_PIN, OUTPUT); 
  pinMode(LED2_PIN, OUTPUT);
  dht.begin(); 

  Serial.begin(115200);
  while (!Serial);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  Serial.print("Connecting to Adafruit IO...");
  io.connect();
  while (io.status() < AIO_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Adafruit IO connected!");

  digital->get();
  digital->onMessage(handleMessage);  
  
  digital2->get();
  digital2->onMessage(handleMessage2); 
}

 void handleMessage(AdafruitIO_Data *data) {
   Serial.print("Received <- digital feed: ");
  if (data->toPinLevel() == HIGH)
    Serial.println("HIGH");
   else
    Serial.println("LOW");

   digitalWrite(LED_PIN, data->toPinLevel());
 }

 void handleMessage2(AdafruitIO_Data *data) {
   Serial.print("Received <- digital2 feed: ");
   if (data->toPinLevel() == HIGH)
     Serial.println("HIGH");
   else
     Serial.println("LOW");

  digitalWrite(LED2_PIN, data->toPinLevel());
}

void loop() {
  io.run();  

  if (millis() - lastUpdateTime >= dhtInterval) {
    lastUpdateTime = millis(); 

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

    temperatureFeed->save(t);  
    humidityFeed->save(h);  

  }
}
