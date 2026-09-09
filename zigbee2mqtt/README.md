# Zigbee2MQTT: zwei Temperaturwerte in Home Assistant

Kein External Converter nötig. Zigbee2MQTT erkennt das Gerät zwar als
"nicht unterstützt" (kein passendes offizielles Geräteprofil), erzeugt aber
automatisch eine "Automatically generated definition" und veröffentlicht
darüber beide Endpoints sauber getrennt:

```json
{
  "linkquality": 140,
  "temperature_10": 23.8,
  "temperature_11": 23.6
}
```

`temperature_10` = Vorlauf (Endpoint 10), `temperature_11` = Ruecklauf
(Endpoint 11), siehe `TEMP_SENSOR_ENDPOINT_VORLAUF`/`_RUECKLAUF` in
`src/main.cpp`.

## Home Assistant

Zigbee2MQTT meldet für diese generische Definition automatisch zwei
Home-Assistant-Entitäten per MQTT-Discovery an (`homeassistant/sensor/...`),
es muss nichts manuell in Home Assistant angelegt werden. Die Entitäten
tauchen unter dem Gerät mit generischem Namen auf und können in Home
Assistant ganz normal umbenannt werden (Einstellungen → Entität bearbeiten).

Falls die Entitäten nach dem Pairing nicht sofort auftauchen: In
**MQTT Explorer** unter `zigbee2mqtt/<friendly_name>` prüfen, ob die Werte
tatsächlich ankommen, und ggf. die **MQTT-Integration** in Home Assistant
neu laden (Einstellungen → Geräte & Dienste → MQTT → Neu laden), damit die
retained Discovery-Nachrichten neu verarbeitet werden.
