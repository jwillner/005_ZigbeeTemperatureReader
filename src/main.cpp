// Autor: JoWizard
#include "Zigbee.h"
#include <Wire.h>
#include <U8g2lib.h>

#define TEMP_SENSOR_ENDPOINT_VORLAUF   10
#define TEMP_SENSOR_ENDPOINT_RUECKLAUF 11
#define BUTTON_PIN                     9   // BOOT-Taster: lang druecken = Factory Reset

// OLED (JMD0.96D-1, SSD1306 128x64) ueber I2C. GPIO9 ist der BOOT-Taster,
// GPIO4-8/15 sind Strapping-Pins und GPIO12/13 fuer USB reserviert,
// daher GPIO18/19 als freie I2C-Pins verwenden.
#define OLED_SDA_PIN 18
#define OLED_SCL_PIN 19

#define LED_PIN 8  // onboard WS2812-RGB-LED (ESP32-C6-Zero)

ZigbeeTempSensor zbVorlauf(TEMP_SENSOR_ENDPOINT_VORLAUF);
ZigbeeTempSensor zbRuecklauf(TEMP_SENSOR_ENDPOINT_RUECKLAUF);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

void showMessage(const char *line1, const char *line2 = nullptr) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 12, line1);
  if (line2 != nullptr) {
    display.drawStr(0, 26, line2);
  }
  display.sendBuffer();
}

void showTemperatures(float vorlauf, float ruecklauf) {
  char buf[16];

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(0, 12, "Vorlauf:");
  snprintf(buf, sizeof(buf), "%.1f C", vorlauf);
  display.drawStr(70, 12, buf);

  display.drawStr(0, 26, "Ruecklauf:");
  snprintf(buf, sizeof(buf), "%.1f C", ruecklauf);
  display.drawStr(70, 26, buf);

  display.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);  // auf USB-CDC-Host warten
  delay(200);
  Serial.println("\nBoot: ZigbeeTemperatureReader");
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  display.begin();
  showMessage("Boot...");

  zbVorlauf.setManufacturerAndModel("DIY", "TempReader-Vorlauf");
  zbVorlauf.setMinMaxValue(-40, 125);
  zbVorlauf.setTolerance(0.5);

  zbRuecklauf.setManufacturerAndModel("DIY", "TempReader-Ruecklauf");
  zbRuecklauf.setMinMaxValue(-40, 125);
  zbRuecklauf.setTolerance(0.5);

  Zigbee.addEndpoint(&zbVorlauf);
  Zigbee.addEndpoint(&zbRuecklauf);

  if (!Zigbee.begin()) {
    Serial.println("Zigbee-Start fehlgeschlagen, Neustart...");
    ESP.restart();
  }

  Serial.print("Verbinde mit Netzwerk");
  bool ledOn = false;
  while (!Zigbee.connected()) {
    Serial.print(".");
    ledOn = !ledOn;
    rgbLedWrite(LED_PIN, ledOn ? 32 : 0, 0, 0);  // blinkend waehrend Verbindungsaufbau
    delay(100);
  }
  rgbLedWrite(LED_PIN, 0, 0, 0);  // LED aus, sobald verbunden
  Serial.println(" ok");

  zbVorlauf.setReporting(1, 0, 1.0);     // min 1 s, max 0 s, delta 1.0 C
  zbRuecklauf.setReporting(1, 0, 1.0);
}

void loop() {
  // Factory Reset: Taster > 3 s halten
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(100);
    unsigned long t = millis();
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
      if (millis() - t > 3000) {
        Serial.println("Factory Reset...");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }

  // DS18B20-Fühler noch nicht angeschlossen, daher feste Platzhalterwerte
  float vorlauf = 45.0;
  float ruecklauf = 35.0;

  zbVorlauf.setTemperature(vorlauf);
  zbRuecklauf.setTemperature(ruecklauf);
  showTemperatures(vorlauf, ruecklauf);
  Serial.printf("Vorlauf: %.2f C, Ruecklauf: %.2f C\n", vorlauf, ruecklauf);

  delay(1000);
}
