# Sandbox ESP-IDF App

<small>Last updated: 2026-05-09T15:18:11+02:00</small>

ESP-IDF application that uses `M5Unified` without the PlatformIO layout.

## What it does

- Starts through native ESP-IDF entry points
- Uses `M5Unified` for display and button access
- Serves as a compatibility sandbox for ESP-IDF + M5Stack

## Build

```bash
source /Users/krzysztof/esp/esp-idf/export.sh
idf.py -C sandbox/espidf build
```

## Flash

```bash
source /Users/krzysztof/esp/esp-idf/export.sh
idf.py -C sandbox/espidf flash
```

