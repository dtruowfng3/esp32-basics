#define BLYNK_TEMPLATE_ID "TMPL64Ynoovqx"
#define BLYNK_TEMPLATE_NAME "dTruong3"
#define BLYNK_AUTH_TOKEN "0RxDbuzpTlNJF5joHnAyQNv0TDycHVt5"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// definition PIN DHT11
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Config wifi
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Định nghĩa chân LED
#define LED 26
#define LED2 16

// Tạo đối tượng BlynkTimer
BlynkTimer timer;

// Hàm điều khiển LED từ nút nhấn trong ứng dụng Blynk
BLYNK_WRITE(V0) {
  int value = param.asInt(); // Đọc giá trị từ Blynk (1 hoặc 0)
  value ? digitalWrite(LED, HIGH) : digitalWrite(LED, LOW); // Điều khiển LED
}

BLYNK_WRITE(V6) {
  int value = param.asInt(); // Đọc giá trị từ Blynk (1 hoặc 0)
  value ? digitalWrite(LED2, HIGH) : digitalWrite(LED2, LOW); // Điều khiển LED2
}

// Hàm gửi dữ liệu cảm biến lên Blynk
void sendSensor() {
  float h = dht.readHumidity();    // Đọc độ ẩm
  float t = dht.readTemperature(); // Đọc nhiệt độ

  // Kiểm tra nếu không đọc được dữ liệu từ cảm biến
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // In ra dữ liệu nhiệt độ và độ ẩm lên Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" °C  Humidity: ");
    Serial.print(h);
    Serial.println(" %");

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V3, h); // Gửi độ ẩm lên V3
  Blynk.virtualWrite(V2, t); // Gửi nhiệt độ lên V2
}

// Hàm setup
void setup() {
  // Mở Serial Monitor
  Serial.begin(9600);
  delay(1000);

  // Kết nối với Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Khởi tạo các chân
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  dht.begin();

  // Thiết lập gửi dữ liệu cảm biến mỗi giây
  timer.setInterval(1000L, sendSensor);
}

// Hàm loop
void loop() {
  Blynk.run(); // Chạy Blynk
  timer.run(); // Chạy bộ định thời
}
