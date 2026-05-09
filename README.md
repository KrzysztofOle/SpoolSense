# 🧵 SpoolSense

Polish version: [README_PL.md](README_PL.md)

<small>Last updated: 2026-05-09T14:37:57+02:00</small>

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

### M5Stack Core ESP32 - reference notes

The project is being developed with M5Stack Core ESP32 hardware in mind, so this mapping is useful for wiring and debug work:

- 320x240 LCD with the `ILI9342C` controller
- TF card connected over SPI
- `BtnA`, `BtnB`, `BtnC` on `GPIO39`, `GPIO38`, `GPIO37`
- speaker on `GPIO25`
- `GROVE A` I2C pins: `GPIO22` as `SCL`, `GPIO21` as `SDA`
- `IP5306` power-management chip at I2C address `0x75`

These are hardware reference details for the M5Stack layer and can help when debugging the LCD, buttons, and I2C bus.

## 🔧 Runtime Model

The firmware now runs as a FreeRTOS task-based runtime instead of a single application loop.
HX711 can be disabled for a lightweight RFID/UI developer mode.
Set `CONFIG_SPOOLSENSE_ENABLE_HX711=n` to keep the runtime on RFID, LCD, buttons, and FSM only.

Startup creates separate tasks for:

- central App Task that owns the explicit application state machine
- RFID / PN532 polling
- HX711 sampling when enabled
- display rendering
- diagnostics and status logging

The app layer owns lifecycle startup, recovery, and transition logic. Task-to-task communication is queue-based:

- RFID Task publishes RFID events to the App Task
- HX711 Task publishes weight updates to the App Task
- App Task publishes UI state snapshots to the UI Task

The current developer flow also reads the three M5Stack buttons in the App Task:

- BtnA
- BtnB
- BtnC

`BtnA` zeroes the HX711 baseline at runtime and the UI shows the current weight estimate in grams after the initial tare.

The App Task is the single owner of mutable application state, evaluates FSM transitions, and aggregates sensor data before the UI renders it. `UiState` carries the current `AppMode`, so the display can react to Boot, Idle, TagDetected, Measuring, Error, and Calibration states.
When HX711 is disabled, the runtime stays in RFID/UI mode and does not treat the missing weight sensor as a fatal fault.

After the ESP-IDF migration, the same task split remains the execution model for the firmware.

### Okablowanie

#### HX711

- `DOUT` -> `GPIO16`
- `SCK` -> `GPIO17`
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

- `HX711 disabled` when the subsystem is turned off in config
- `HX711 init OK`
- `HX711 not found`
- weight readings printed periodically over `Serial` together with the estimated grams
- `BtnA clicked` followed by `HX711 zeroed` when the HX711 baseline is reset at runtime
- `PN532 init OK`
- `PN532 not found`
- RFID UID printed when a tag is presented
- button clicks logged as `BtnA clicked`, `BtnB clicked`, or `BtnC clicked`
- for NTAG213 tags, page `0x24` is read without writing to the tag
- `Serial` shows: `Page 24: ...`, `Usage: ... s`, `Life: ...%`
- the counter is interpreted as little-endian, and the time conversion is a heuristic for Philips Sonicare brush heads

## 🧱 Code Layout

- `main/src/main.cpp` - ESP-IDF entry point with `app_main()`
- `components/app` - firmware orchestration and FreeRTOS task lifecycle
- `components/board` - pin assignments and board-level bus setup
- `components/pn532` - RFID logic plus local Arduino PN532 compatibility sources
- `components/hx711` - HX711 polling plus local Arduino HX711 compatibility source
- `components/display` - M5 display handling
- `components/diagnostics` - logging and formatting helpers
- `sdkconfig.defaults` - base ESP-IDF configuration

## 🧰 VSCode / ESP-IDF Setup

- Install the `ESP-IDF` extension from Espressif.
- Install the Microsoft `C/C++` extension for IntelliSense.
- Optional but useful: `CMake Tools` for CMake integration in VS Code.
- Open the repository root as the workspace.
- Activate the ESP-IDF setup at `/Users/krzysztof/esp/esp-idf` before using the terminal.
- The project is configured for `esp32` and uses `sdkconfig.defaults` as the base configuration.
- `build/compile_commands.json` is used for code navigation and autocomplete.
- The VS Code task set in `.vscode/tasks.json` provides `Build`, `Flash`, `Monitor`, `Flash + Monitor`, `Full Clean`, and `Reconfigure`.

## 🔨 Build and Flash

### VS Code tasks

1. `Build` runs `idf.py build`
2. `Flash` runs `idf.py flash`
3. `Monitor` runs `idf.py monitor`
4. `Flash + Monitor` runs `idf.py flash monitor`
5. `Full Clean` runs `idf.py fullclean`
6. `Reconfigure` runs `idf.py reconfigure`

The ESP-IDF configuration targets `16MB` flash to match the installed ESP32 module.

### Manual CLI workflow

1. Activate the ESP-IDF environment with `source /Users/krzysztof/esp/esp-idf/export.sh`
2. Run `idf.py reconfigure` after changing dependencies or target settings
3. Run `idf.py build`
4. Run `idf.py flash`
5. Run `idf.py monitor`

The monitor baud rate is `115200`. If the serial port changes, update the VS Code settings or select the port from the ESP-IDF extension command palette.

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
