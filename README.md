# 🧵 SpoolSense

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

Projekt zawiera proste testy sprzętowe HX711 i PN532 uruchamiane bezpośrednio z firmware. To są testy do szybkiej weryfikacji okablowania i działania modułów, nie osobna architektura aplikacji.

### Włączenie testów

- Otwórz [`src/main.cpp`](src/main.cpp)
- Znajdź flagę `ENABLE_HW_TESTS`
- Ustaw ją na `1`:

```cpp
#define ENABLE_HW_TESTS 1
```

- Po wgraniu firmware testy będą wykonywane z `main.cpp`

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
- Ustaw moduł w tryb `I2C` za pomocą `DIP switch`

### Jak testować

1. Wgraj firmware z włączonym `ENABLE_HW_TESTS`
2. Uruchom monitor portu szeregowego z prędkością `115200`
3. Sprawdź logi startowe i odczyty

Oczekiwane komunikaty:

- `HX711 init OK`
- `HX711 not found`
- odczyty wagi wypisywane cyklicznie przez `Serial`
- `PN532 init OK`
- `PN532 not found`
- UID karty RFID wypisywany po zbliżeniu tagu

### Troubleshooting

- `PN532 not found` -> sprawdź tryb komunikacji `I2C`, ustawienie `DIP switch` i zasilanie modułu
- brak odczytu z `HX711` -> sprawdź piny `DOUT` / `SCK` oraz zasilanie `3.3V`

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
