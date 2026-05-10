# Sandbox Native ESP-IDF App

<small>Last updated: 2026-05-10T10:04:51+02:00</small>

Native ESP-IDF sandbox without Arduino dependencies.

## What it does

- Uses `driver`, `esp_lcd`, and FreeRTOS directly
- Shows a color test screen with all 8 RGB combinations as large swatches
- Uses button B to toggle RGB/BGR order while A and C keep input counters
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
