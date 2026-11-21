# SpoolSense — smart filament scale (prototype)

**Verified state of `main` as of 2026-11-21:** PlatformIO/Arduino firmware skeleton for **M5Stack Core ESP32**. The only `src/main.cpp` initializes M5Unified and prints a periodic message to serial; **weight measurement, HX711, RFID and the multi-screen UI are not implemented on `main`**.

## Purpose
The project explores a connected scale for monitoring the remaining mass of filament on a 3D-printer spool. Intended future capabilities include load-cell measurements, spool identification by RFID and a local screen. These are roadmap items, not tested product functions.

## Current technology
- ESP32 / `m5stack-core-esp32`, framework **Arduino**;
- PlatformIO `espressif32@6.7.0`;
- M5Unified library from the M5Stack repository;
- serial monitor at 115200 baud.

## Build and run (M5Stack Core)
Install [PlatformIO](https://platformio.org/) and connect the supported board.

```sh
pio run -e m5stack-core
pio run -e m5stack-core -t upload
pio device monitor -b 115200
```

Firmware upload changes the device. Confirm the hardware and port first. Only the build configuration is visible in GitHub; the commands and physical runtime have **not** been executed during this documentation audit.

## Repository structure
- `platformio.ini` — board, framework and dependency.
- `src/main.cpp` — initialization and serial heartbeat.
- [`BRANCHES_AUDIT.md`](BRANCHES_AUDIT.md) — four side branches with significant, unmerged firmware development (home screen, HX711/RFID, multi-screen UI).

## Maturity
Early prototype. Do **not** treat feature branches as production-ready or automatically merge their `build/` artifacts. Select a candidate, review differences, then run hardware tests.
