# 🧵 SpoolSense

Polish version: [README_PL.md](README_PL.md)

<small>Last updated: 2026-05-07T18:00:22+02:00</small>

SpoolSense is a smart filament spool scale designed for 3D printing environments.  
It measures filament weight in real-time, tracks usage, and optionally monitors the drying process when placed under a filament dryer.

## 🚀 Features

- ⚖️ Real-time filament weight measurement
- 📉 Filament usage tracking and estimation
- 🔥 Drying process monitoring (temperature/time integration – optional)
- 🏷️ RFID-based spool identification (automatic spool recognition)
- 📊 Data logging and analysis
- 🔌 Designed for integration with IoT / automation systems

## 🧠 Concept

The device is placed under a filament dryer or spool holder.  
It continuously measures the spool weight and correlates it with:

- filament consumption during printing
- drying cycles (time/temperature)
- spool identity via RFID tags

This allows:

- accurate estimation of remaining filament
- tracking material usage over time
- validating drying conditions for sensitive filaments (e.g. Nylon, PETG)

## 🛠️ Hardware (planned)

- Load cell + HX711 (or similar ADC)
- Microcontroller (ESP32 / similar)
- RFID reader (e.g. PN532)
- Optional temperature sensor (dryer integration)

## 🔧 Hardware Tests

The project includes simple HX711 and PN532 hardware checks that run directly in firmware. They are intended for quick validation of wiring and module behavior, not as a separate application architecture.

After the ESP-IDF migration, the checks run as part of the normal firmware startup.

### Okablowanie

#### HX711

- `DOUT` -> `GPIO16`
- `SCK` -> `GPIO4`
- `VCC` -> `3.3V`
- `GND` -> `GND`

#### PN532 (I2C)

- `SDA` -> `GPIO21`
- `SCL` -> `GPIO22`
- `VCC` -> `3.3V`
- `GND` -> `GND`
- Set the module to `I2C` mode using the `DIP switch`

### How to Test

1. Flash the firmware
2. Open the serial monitor at `115200`
3. Check the startup logs and readings

Expected messages:

- `HX711 init OK`
- `HX711 not found`
- weight readings printed periodically over `Serial`
- `PN532 init OK`
- `PN532 not found`
- RFID UID printed when a tag is presented
- for NTAG213 tags, page `0x24` is read without writing to the tag
- `Serial` shows: `Page 24: ...`, `Usage: ... s`, `Life: ...%`
- the counter is interpreted as little-endian, and the time conversion is a heuristic for Philips Sonicare brush heads

## 🧱 Code Layout

- `main/src/main.cpp` - ESP-IDF entry point with `app_main()`
- `components/app` - firmware orchestration
- `components/board` - pin assignments and board-level bus setup
- `components/pn532` - RFID logic plus local Arduino PN532 compatibility sources
- `components/hx711` - HX711 polling plus local Arduino HX711 compatibility source
- `components/display` - M5 display handling
- `components/diagnostics` - logging and formatting helpers
- `sdkconfig.defaults` - base ESP-IDF configuration

### Building

1. Activate the ESP-IDF environment.
2. Run `idf.py set-target esp32` once for this project.
3. Run `idf.py build`.
4. Flash and monitor with `idf.py flash monitor`.

### Troubleshooting

- `PN532 not found` -> check the `I2C` mode, `DIP switch` setting, and module power
- no HX711 reading -> check the `DOUT` / `SCK` pins and `3.3V` power

## 💻 Software (planned)

- Python-based data processing / API
- Embedded firmware for data acquisition
- Optional visualization (dashboard / logs)

## 📦 Use cases

- 3D printing farms
- filament management systems
- material research / testing
- hobbyist and professional setups

## ⚠️ Status

Project in early development stage.

## 📜 License

MIT
