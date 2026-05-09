# Sandbox PlatformIO App

<small>Last updated: 2026-05-09T15:25:07+02:00</small>

PlatformIO test app for M5Stack Core ESP32.

## What it does

- Initializes `M5Unified`
- Clears the display
- Shows the application folder name on the display
- Counts presses on `BtnA`, `BtnB`, and `BtnC`

## Build

```bash
cd sandbox/platformio
/Users/krzysztof/.platformio/penv/bin/platformio run
```

## Flash

```bash
cd sandbox/platformio
/Users/krzysztof/.platformio/penv/bin/platformio run --target upload
```
