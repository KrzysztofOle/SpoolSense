# Sandbox Native ESP-IDF App

<small>Last updated: 2026-05-10T11:34:00+02:00</small>

Native ESP-IDF sandbox without Arduino dependencies.

## What it does

- Uses `driver`, `esp_lcd`, and FreeRTOS directly
- Shows switchable BGR/GRAY test screens with large swatches
- Uses A/B/C to cycle the screen and toggle LCD RGB/BGR order
- Drives the LCD in 18-bit RGB666 mode to match the panel datasheet
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
