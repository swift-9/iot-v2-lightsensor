#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter;

// Replace with ESP32 MAC
uint8_t peer_esp32[] = {0x68, 0x25, 0xDD, 0x32, 0x0F, 0x7C};

typedef struct struct_message {
  float temperature;
  float humidity;
  float lux;
} struct_message;

struct_message payload;

void OnDataSent(uint8_t *mac, uint8_t sendStatus) {
  Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  Wire.begin(D6, D5);   // SDA=D6, SCL=D5
  lightMeter.begin();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(peer_esp32, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  payload.temperature = dht.readTemperature();
  payload.humidity    = dht.readHumidity();
  payload.lux         = lightMeter.readLightLevel();

  Serial.print("T: "); Serial.print(payload.temperature);
  Serial.print(" °C  H: "); Serial.print(payload.humidity);
  Serial.print(" %  Lux: "); Serial.println(payload.lux);

  esp_now_send(peer_esp32, (uint8_t *) &payload, sizeof(payload));

  delay(1000);  // every second
}
