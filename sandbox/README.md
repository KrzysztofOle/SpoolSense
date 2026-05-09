# Sandbox Apps

<small>Last updated: 2026-05-09T15:18:11+02:00</small>

This directory contains three separate sandbox applications, each isolated in its own folder:

- `sandbox/platformio/` - PlatformIO + Arduino + `M5Unified`
- `sandbox/espidf/` - ESP-IDF + `M5Unified`
- `sandbox/espidf_native/` - native ESP-IDF without Arduino

Each app can be built and flashed independently.

## Build

- PlatformIO: `cd sandbox/platformio && /Users/krzysztof/.platformio/penv/bin/platformio run`
- ESP-IDF + M5Unified: `source /Users/krzysztof/esp/esp-idf/export.sh && idf.py -C sandbox/espidf build`
- Native ESP-IDF: `source /Users/krzysztof/esp/esp-idf/export.sh && idf.py -C sandbox/espidf_native build`

## Flash

- PlatformIO: `cd sandbox/platformio && /Users/krzysztof/.platformio/penv/bin/platformio run --target upload`
- ESP-IDF + M5Unified: `source /Users/krzysztof/esp/esp-idf/export.sh && idf.py -C sandbox/espidf flash`
- Native ESP-IDF: `source /Users/krzysztof/esp/esp-idf/export.sh && idf.py -C sandbox/espidf_native flash`

