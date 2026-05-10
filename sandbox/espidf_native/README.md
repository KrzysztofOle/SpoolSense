# Sandbox Native ESP-IDF App

<small>Last updated: 2026-05-10T09:49:32+02:00</small>

Native ESP-IDF sandbox without Arduino dependencies.

## What it does

- Uses `driver`, `esp_lcd`, and FreeRTOS directly
- Shows a color test screen with large LCD swatches for RGB/CMY/white/gray checks
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
