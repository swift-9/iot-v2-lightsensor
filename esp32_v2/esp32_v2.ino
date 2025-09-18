#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Must match sender struct
typedef struct struct_message {
  float temperature;
  float humidity;
  float lightLevel;   // lux as float
} struct_message;

struct_message incomingData;

// --- Small 16x16 icons ---
// ==========================
// 16x16 WEATHER ICONS
// ==========================

// SUNNY ☀️


// CLOUDY ☁️
const unsigned char PROGMEM sun_bmp[] = {1,0,65,4,32,8,3,128,15,224,15,224,31,240,223,246,31,240,15,224,15,224,3,128,32,8,65,4,1,0,0,0}; 

const unsigned char PROGMEM cloud_bmp[]  = {
  // 'cloudy', 60x53px
 0,0,0,0,146,0,84,128,1,32,124,64,254,0,254,0,
255,192,248,32,112,16,128,14,128,1,128,1,192,1,127,254

};

// DIM 💡 (Bulb)
const unsigned char PROGMEM bulb_bmp[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,192,4,32,8,16,112,14,192,1,128,1,128,1,127,254
};

// DARK 🌑 (Night / Filled Circle)
const unsigned char PROGMEM moon_bmp[] = {
  7,224,31,192,63,128,127,0,127,0,254,0,254,0,254,0,
254,0,254,0,127,0,127,0,63,128,31,192,7,192,0,0

};


// Map lux to simple conditions
String luxCondition(float lux) {
  if (lux >= 50000.0) return "Sunny";
  if (lux >= 5000.0)  return "Cloudy";
  if (lux >= 100.0)   return "Dim";
  return "Dark";
}

// Pick icon pointer
const unsigned char* luxIcon(String cond) {
  if (cond == "Sunny")  return sun_bmp;
  if (cond == "Cloudy") return cloud_bmp;
  if (cond == "Dim")    return bulb_bmp;
  return moon_bmp; // Dark
}

void onReceive(const uint8_t *mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  String condition = luxCondition(incomingData.lightLevel);

  // Debug to Serial
  Serial.printf("Received -> Temp: %.2f °C | Hum: %.2f %% | Light: %s\n",
                incomingData.temperature,
                incomingData.humidity,
                condition.c_str());

  // Show on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 10);
  display.printf("Temp: %.2f C", incomingData.temperature);

  display.setCursor(0, 30);
  display.printf("Hum : %.2f %%", incomingData.humidity);

  display.setCursor(0, 50);
  display.printf("SunLight: %s", condition.c_str());

  // draw little icon at right side
  display.drawBitmap(100, 45, luxIcon(condition), 16, 16, SSD1306_WHITE);

  display.display();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Waiting for data...");
  display.display();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  delay(50);
}
