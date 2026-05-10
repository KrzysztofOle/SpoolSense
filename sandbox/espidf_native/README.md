# Sandbox Native ESP-IDF App

<small>Last updated: 2026-05-10T10:00:57+02:00</small>

Native ESP-IDF sandbox without Arduino dependencies.

## What it does

- Uses `driver`, `esp_lcd`, and FreeRTOS directly
- Shows a switchable color test screen with large LCD swatches for RGB/CMY/white/gray checks
- Uses buttons A/B/C to cycle the pattern, toggle RGB/BGR order, and reset
- Keeps LCD, input, renderer, and screen logic split into small modules

## Build

```bash
source /Users/krzysztof/esp/esp-idf/export.sh
idf.py -C sandbox/espidf_native build
```

## Flash

```bash
source /Users/krzysztof/esp/esp-idf/export.sh
idf.py -C sandbox/espidf_native flash
```
