// Autor: JoWizard
#include "Zigbee.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_SENSOR_ENDPOINT_VORLAUF   10
#define TEMP_SENSOR_ENDPOINT_RUECKLAUF 11
#define BUTTON_PIN                     9   // BOOT-Taster: lang druecken (>3s) = Factory Reset

// DS18B20: beide Fuehler haengen an einem gemeinsamen OneWire-Bus,
// Unterscheidung erfolgt ueber die eindeutige ROM-Adresse jedes Sensors.
// Versorgung ueber GPIO3 (geschaltet, fuer spaetere Batterieoptimierung),
// Pull-up-Widerstand der Datenleitung haengt ebenfalls an GPIO3 statt an
// einer festen 3,3V-Schiene.
#define ONE_WIRE_PIN     2
#define SENSOR_POWER_PIN 3
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature dsSensors(&oneWire);
DeviceAddress addrVorlauf, addrRuecklauf;

// OLED (JMD0.96D-1, SSD1306 128x64) ueber I2C. GPIO9 ist der BOOT-Taster,
// GPIO4-8/15 sind Strapping-Pins und GPIO12/13 fuer USB reserviert,
// daher GPIO18/19 als freie I2C-Pins verwenden.
#define OLED_SDA_PIN 18
#define OLED_SCL_PIN 19

#define LED_PIN 8  // onboard WS2812-RGB-LED (ESP32-C6-Zero)

#define DEVICE_MANUFACTURER "by JoWizard"
#define DEVICE_MODEL        "T-Heizungssensor"

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

  display.drawStr(0, 12, DEVICE_MODEL);
  display.drawStr(0, 24, DEVICE_MANUFACTURER);

  display.drawStr(0, 46, "Vorlauf:");
  snprintf(buf, sizeof(buf), "%.1f C", vorlauf);
  display.drawStr(70, 46, buf);

  display.drawStr(0, 58, "Ruecklauf:");
  snprintf(buf, sizeof(buf), "%.1f C", ruecklauf);
  display.drawStr(70, 58, buf);

  display.sendBuffer();
}

void printAddress(DeviceAddress addr) {
  for (uint8_t i = 0; i < 8; i++) {
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
  }
}

// Erkennt die beiden DS18B20 am Bus. Die Zuordnung Vorlauf/Ruecklauf erfolgt
// zunaechst nach Reihenfolge auf dem Bus; die ROM-Adressen werden ueber
// Serial ausgegeben, damit man sie bei Bedarf vertauschen kann.
void setupDs18b20() {
  dsSensors.begin();
  int count = dsSensors.getDeviceCount();
  Serial.printf("DS18B20: %d Sensor(en) gefunden\n", count);

  if (count < 2 || !dsSensors.getAddress(addrVorlauf, 0) || !dsSensors.getAddress(addrRuecklauf, 1)) {
    Serial.println("FEHLER: Es werden 2 DS18B20 am Bus erwartet!");
    showMessage("DS18B20 Fehler", "< 2 Sensoren");
    while (true) delay(1000);
  }

  Serial.print("Vorlauf-Adresse:   ");
  printAddress(addrVorlauf);
  Serial.println();
  Serial.print("Ruecklauf-Adresse: ");
  printAddress(addrRuecklauf);
  Serial.println();

  dsSensors.setResolution(addrVorlauf, 12);
  dsSensors.setResolution(addrRuecklauf, 12);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);  // auf USB-CDC-Host warten
  delay(200);
  Serial.println("\nBoot: ZigbeeTemperatureReader");
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);  // DS18B20 mit Strom versorgen
  delay(10);  // Anlaufzeit der Sensoren nach dem Einschalten

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  display.begin();
  showMessage("Boot-Selbsttest");
  delay(500);
  showMessage(DEVICE_MODEL, DEVICE_MANUFACTURER);
  delay(3000);  // Geraetename/Hersteller lesbar stehen lassen

  setupDs18b20();

  zbVorlauf.setManufacturerAndModel(DEVICE_MANUFACTURER, DEVICE_MODEL);
  zbVorlauf.setMinMaxValue(-40, 125);
  zbVorlauf.setTolerance(0.5);

  zbRuecklauf.setManufacturerAndModel(DEVICE_MANUFACTURER, DEVICE_MODEL);
  zbRuecklauf.setMinMaxValue(-40, 125);
  zbRuecklauf.setTolerance(0.5);

  Zigbee.addEndpoint(&zbVorlauf);
  Zigbee.addEndpoint(&zbRuecklauf);

  if (!Zigbee.begin()) {
    Serial.println("Zigbee-Start fehlgeschlagen, Neustart...");
    ESP.restart();
  }

  Serial.print("Verbinde mit Netzwerk");
  showMessage("Nicht verbunden", "Suche Netzwerk...");
  bool ledOn = false;
  while (!Zigbee.connected()) {
    Serial.print(".");
    ledOn = !ledOn;
    // Board-LED hat R/G vertauscht ggue. rgbLedWrite(): G-Kanal ansteuern fuer rot
    rgbLedWrite(LED_PIN, 0, ledOn ? 32 : 0, 0);  // blinkend waehrend Verbindungsaufbau
    delay(100);
  }
  rgbLedWrite(LED_PIN, 0, 0, 0);  // LED aus, sobald verbunden
  Serial.println(" ok");

  zbVorlauf.setReporting(1, 0, 1.0);     // min 1 s, max 0 s, delta 1.0 C
  zbRuecklauf.setReporting(1, 0, 1.0);
}

void loop() {
  // Factory Reset: Taster > 3 s halten. LED blinkt sofort ab Tastendruck
  // (Bestaetigung "wird erkannt, weiter halten"), leuchtet dauerhaft rot
  // kurz vor dem eigentlichen Reset.
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(100);
    unsigned long t = millis();
    bool ledOnDuringHold = false;
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
      ledOnDuringHold = !ledOnDuringHold;
      rgbLedWrite(LED_PIN, 0, ledOnDuringHold ? 32 : 0, 0);  // schnelles rotes Blinken waehrend des Haltens
      if (millis() - t > 3000) {
        Serial.println("Factory Reset...");
        rgbLedWrite(LED_PIN, 0, 32, 0);  // rot (Board-LED: G-Kanal ansteuern, siehe oben)
        delay(1000);
        // false = NVRAM sofort zuruecksetzen ohne auf Leave-Handshake mit dem
        // Koordinator zu warten (der kann haengen/verzoegern); Neustart selbst
        // steuern, damit der Reset zuverlaessig beim ersten Tastendruck greift.
        Zigbee.factoryReset(false);
        ESP.restart();
      }
    }
    rgbLedWrite(LED_PIN, 0, 0, 0);  // Taster losgelassen, ohne Reset ausgeloest zu haben
  }

  dsSensors.requestTemperatures();
  float vorlauf = dsSensors.getTempC(addrVorlauf);
  float ruecklauf = dsSensors.getTempC(addrRuecklauf);

  // Stiller Sofort-Retry: der Zigbee-Funk-Task kann das zeitkritische
  // OneWire-Timing vereinzelt kurz stoeren (z. B. direkt nach dem
  // Verbindungsaufbau), ein zweiter Versuch behebt das zuverlaessig.
  if (vorlauf == DEVICE_DISCONNECTED_C || ruecklauf == DEVICE_DISCONNECTED_C) {
    dsSensors.requestTemperatures();
    vorlauf = dsSensors.getTempC(addrVorlauf);
    ruecklauf = dsSensors.getTempC(addrRuecklauf);
  }

  if (vorlauf == DEVICE_DISCONNECTED_C || ruecklauf == DEVICE_DISCONNECTED_C) {
    Serial.println("FEHLER: DS18B20 nicht erreichbar");
    showMessage("Sensor-Fehler", "DS18B20 pruefen");
    delay(3000);  // laenger stehen lassen, damit die Meldung lesbar ist
    return;
  }

  zbVorlauf.setTemperature(vorlauf);
  zbRuecklauf.setTemperature(ruecklauf);
  showTemperatures(vorlauf, ruecklauf);
  Serial.printf("Vorlauf: %.2f C, Ruecklauf: %.2f C\n", vorlauf, ruecklauf);

  delay(1000);
}
