# 005_ZigbeeTemperatureReader

Zigbee-Temperatursensor auf ESP32-C6.

## Toolchain
- PlatformIO in VS Code, Platform: pioarduino-Fork (offizieller `espressif32` unterstuetzt C6 nicht)
- Framework: Arduino (arduino-esp32 3.x), Zigbee-Library ist Teil des Cores
- Build: `pio run` | Flash: `pio run -t upload` | Monitor: `pio device monitor`

## Konventionen
- Zigbee-Rolle: End Device (`-DZIGBEE_MODE_ED`), Partition `zigbee.csv`
- Quellcode in `src/`, ein Endpoint pro Messgroesse
- Kommentare und Commit-Messages auf Deutsch

## Setup-Hinweis (macOS)
PlatformIOs portables Python hat kein LZMA. Deshalb in den VS-Code-User-Settings:
`"platformio-ide.useBuiltinPython": false` und
`"platformio-ide.customPyPath": "/Library/Frameworks/Python.framework/Versions/3.13/bin/python3.13"`.
Bei Problemen: `rm -rf ~/.platformio/penv ~/.platformio/python3` und VS Code neu laden.
