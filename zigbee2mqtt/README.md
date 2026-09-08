# Zigbee2MQTT External Converter

Ohne diesen Converter zeigt Zigbee2MQTT nur einen der beiden Temperaturwerte
an, da mehrere Endpoints mit demselben Cluster-Typ standardmäßig nicht
korrekt unterschieden werden ([bekanntes Z2M-Problem](https://github.com/Koenkk/zigbee2mqtt/issues/31172)).

## Installation (Home Assistant Add-on, ohne Dateisystemzugriff)

1. Zigbee2MQTT-Oberfläche öffnen (Sidebar in Home Assistant → "Zigbee2MQTT").
2. **Settings → Dev Console → External converters.**
3. Neuen Converter anlegen, Name `zigbee_temperature_reader.mjs`, Inhalt aus
   `external_converters/zigbee_temperature_reader.mjs` einfügen und speichern.
   Wird sofort übernommen, kein Neustart nötig.
4. Prüfen, dass in der Add-on-Konfiguration (configuration.yaml)
   `advanced: enable_external_js: true` gesetzt ist (i. d. R. Standard).
5. Gerät in Zigbee2MQTT entfernen und per Factory Reset (BOOT-Taster > 3 s
   halten) erneut anlernen, damit beide Endpoints frisch erkannt werden.
6. Prüfen, ob Zigbee2MQTT das Gerät als `ZigbeeTemperatureReader` erkennt.
   Falls es weiterhin als "nicht unterstützt" gemeldet wird: den im
   Z2M-Log/Frontend angezeigten `modelID` mit dem Wert in `zigbeeModel`
   in der `.mjs`-Datei abgleichen und anpassen.

### Alternative: Dateisystemzugriff (Standalone-Installation)

`external_converters/zigbee_temperature_reader.mjs` in das
`external_converters`-Verzeichnis von Zigbee2MQTT kopieren
(`<data_directory>/external_converters/`), danach wie oben ab Schritt 4
fortfahren und Zigbee2MQTT neu starten.

Nach erfolgreichem Setup erscheinen zwei getrennte Entitäten in Home
Assistant mit eigenem Namen: **Vorlauf** und **Ruecklauf**.
