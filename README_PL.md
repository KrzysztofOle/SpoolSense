# 🧵 SpoolSense

Wersja angielska: [README.md](README.md)

<small>Last updated: 2026-05-07T19:20:22+02:00</small>

SpoolSense to inteligentna waga do szpuli filamentu przeznaczona do środowisk druku 3D.  
Mierzy wagę filamentu w czasie rzeczywistym, śledzi zużycie i opcjonalnie monitoruje proces suszenia, gdy jest umieszczona pod suszarką do filamentu.

## 🚀 Funkcje

- ⚖️ Pomiar wagi filamentu w czasie rzeczywistym
- 📉 Śledzenie i szacowanie zużycia filamentu
- 🔥 Monitorowanie procesu suszenia (integracja temperatury/czasu - opcjonalnie)
- 🏷️ Identyfikacja szpuli za pomocą RFID (automatyczne rozpoznawanie szpuli)
- 📊 Rejestrowanie i analiza danych
- 🔌 Projektowany do integracji z systemami IoT / automatyki

## 🧠 Koncepcja

Urządzenie jest umieszczane pod suszarką do filamentu lub uchwytem na szpulę.  
Nieprzerwanie mierzy wagę szpuli i koreluje ją z:

- zużyciem filamentu podczas druku
- cyklami suszenia (czas/temperatura)
- identyfikacją szpuli przez tagi RFID

Dzięki temu możliwe jest:

- dokładniejsze szacowanie ilości pozostałego filamentu
- śledzenie zużycia materiału w czasie
- weryfikacja warunków suszenia dla wrażliwych filamentów (np. Nylon, PETG)

## 🛠️ Sprzęt (planowany)

- Cewka tensometryczna + HX711 (lub podobny ADC)
- Mikrokontroler (ESP32 / podobny)
- Czytnik RFID (np. PN532)
- Opcjonalny czujnik temperatury (integracja z suszarką)

## 🔧 Model runtime

Firmware działa teraz jako runtime oparty o taski FreeRTOS zamiast pojedynczej pętli aplikacji.

Start tworzy osobne taski dla:

- centralnego App Task, który posiada jawną maszynę stanów aplikacji
- odczytu RFID / PN532
- próbkowania HX711
- renderowania wyświetlacza
- diagnostyki i logów stanu

Warstwa `app` odpowiada za lifecycle startupu, recovery i logikę przejść stanów. Komunikacja między taskami jest oparta o kolejki:

- RFID Task publikuje eventy RFID do App Task
- HX711 Task publikuje aktualizacje wagi do App Task
- App Task publikuje snapshoty stanu UI do UI Task

App Task jest jedynym właścicielem mutowalnego stanu aplikacji, wykonuje logikę FSM i agreguje dane sensorów przed renderowaniem UI. `UiState` przenosi aktualny `AppMode`, więc wyświetlacz może reagować na stany Boot, Idle, TagDetected, Measuring, Error i Calibration.

Po migracji do ESP-IDF ten sam podział na taski pozostaje modelem wykonania firmware.

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

1. Wgraj firmware
2. Uruchom monitor portu szeregowego z prędkością `115200`
3. Sprawdź logi startowe i odczyty

Oczekiwane komunikaty:

- `HX711 init OK`
- `HX711 not found`
- odczyty wagi wypisywane cyklicznie przez `Serial`
- `PN532 init OK`
- `PN532 not found`
- UID karty RFID wypisywany po zbliżeniu tagu
- dla tagów NTAG213 odczytywana jest strona `0x24` bez zapisu do taga
- w `Serial` pojawiają się: `Page 24: ...`, `Usage: ... s`, `Life: ...%`
- licznik jest interpretowany jako little-endian, a przeliczenie czasu jest heurystyczne dla końcówek Philips Sonicare

## 🧱 Układ kodu

- `main/src/main.cpp` - punkt wejścia ESP-IDF z `app_main()`
- `components/app` - orkiestracja firmware i cykl zycia taskow FreeRTOS
- `components/board` - przypisania pinów i uruchomienie magistrali
- `components/pn532` - logika RFID oraz lokalne źródła zgodności Arduino dla PN532
- `components/hx711` - odpytywanie HX711 oraz lokalne źródło zgodności Arduino dla HX711
- `components/display` - obsługa wyświetlacza M5
- `components/diagnostics` - logowanie i formatowanie danych
- `sdkconfig.defaults` - bazowa konfiguracja ESP-IDF

### Budowanie

1. Aktywuj środowisko ESP-IDF.
2. Uruchom `idf.py set-target esp32` raz dla tego projektu.
3. Uruchom `idf.py build`.
4. Wgraj i podglądaj logi przez `idf.py flash monitor`.

### Rozwiązywanie problemów

- `PN532 not found` -> sprawdź tryb komunikacji `I2C`, ustawienie `DIP switch` i zasilanie modułu
- brak odczytu z `HX711` -> sprawdź piny `DOUT` / `SCK` oraz zasilanie `3.3V`

## 💻 Oprogramowanie (planowane)

- Przetwarzanie danych / API w Pythonie
- Firmware wbudowany do zbierania danych
- Opcjonalna wizualizacja (dashboard / logi)

## 📦 Zastosowania

- farmy drukarek 3D
- systemy zarządzania filamentem
- badania i testy materiałowe
- zestawy hobbystyczne i profesjonalne

## ⚠️ Status

Projekt znajduje się na wczesnym etapie rozwoju.

## 📜 Licencja

MIT
