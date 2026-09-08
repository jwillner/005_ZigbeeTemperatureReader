#include "Zigbee.h"
#include <Wire.h>
#include <U8g2lib.h>

#define TEMP_SENSOR_ENDPOINT 10
#define BUTTON_PIN           9   // BOOT-Taster: lang druecken = Factory Reset

// OLED (JMD0.96D-1, SSD1306 128x64) ueber I2C. GPIO9 ist der BOOT-Taster,
// GPIO4-8/15 sind Strapping-Pins und GPIO12/13 fuer USB reserviert,
// daher GPIO18/19 als freie I2C-Pins verwenden.
#define OLED_SDA_PIN 18
#define OLED_SCL_PIN 19

ZigbeeTempSensor zbTempSensor(TEMP_SENSOR_ENDPOINT);
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

void showTemperature(float temp) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f C", temp);

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 12, "Temperatur:");
  display.setFont(u8g2_font_logisoso24_tf);
  display.drawStr(0, 48, buf);
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

  zbTempSensor.setManufacturerAndModel("DIY", "TempReader");
  zbTempSensor.setMinMaxValue(-40, 125);
  zbTempSensor.setTolerance(0.5);

  Zigbee.addEndpoint(&zbTempSensor);

  if (!Zigbee.begin()) {
    Serial.println("Zigbee-Start fehlgeschlagen, Neustart...");
    ESP.restart();
  }

  Serial.print("Verbinde mit Netzwerk");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" ok");

  zbTempSensor.setReporting(1, 0, 1.0);  // min 1 s, max 0 s, delta 1.0 C
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

  float temp = temperatureRead();  // interner Sensor als Platzhalter
  zbTempSensor.setTemperature(temp);
  Serial.printf("Temp: %.2f C\n", temp);

  delay(1000);
}
