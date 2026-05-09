# 🧵 SpoolSense

Wersja angielska: [README.md](README.md)

<small>Last updated: 2026-05-09T14:37:57+02:00</small>

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

### M5Stack Core ESP32 - informacje pomocnicze

Projekt jest rozwijany z myślą o M5Stack Core ESP32, więc poniższe mapowanie pomaga przy okablowaniu i testach:

- wyświetlacz LCD 320x240 z kontrolerem `ILI9342C`
- karta TF podłączona przez SPI
- przyciski `BtnA`, `BtnB`, `BtnC` na pinach `GPIO39`, `GPIO38`, `GPIO37`
- głośnik na pinie `GPIO25`
- złącze `GROVE A` dla I2C: `GPIO22` jako `SCL`, `GPIO21` jako `SDA`
- układ zarządzania zasilaniem `IP5306` pod adresem I2C `0x75`

To są informacje referencyjne dla warstwy sprzętowej M5Stack i mogą pomóc przy debugowaniu LCD, przycisków oraz magistrali I2C.

## 🔧 Model runtime

Firmware działa teraz jako runtime oparty o taski FreeRTOS zamiast pojedynczej pętli aplikacji.
HX711 można wyłączyć, aby uruchomić lżejszy tryb developerski RFID/UI.
Ustaw `CONFIG_SPOOLSENSE_ENABLE_HX711=n`, aby runtime pozostał tylko przy RFID, LCD, przyciskach i FSM.

Start tworzy osobne taski dla:

- centralnego App Task, który posiada jawną maszynę stanów aplikacji
- odczytu RFID / PN532
- próbkowania HX711, gdy subsystem jest włączony
- renderowania wyświetlacza
- diagnostyki i logów stanu

Warstwa `app` odpowiada za lifecycle startupu, recovery i logikę przejść stanów. Komunikacja między taskami jest oparta o kolejki:

- RFID Task publikuje eventy RFID do App Task
- HX711 Task publikuje aktualizacje wagi do App Task
- App Task publikuje snapshoty stanu UI do UI Task

Aktualny tryb developerski odczytuje też trzy przyciski M5Stack w App Task:

- BtnA
- BtnB
- BtnC

`BtnA` zeruje bazę HX711 w czasie pracy, a interfejs pokazuje aktualne oszacowanie wagi w gramach po początkowym tarowaniu.

App Task jest jedynym właścicielem mutowalnego stanu aplikacji, wykonuje logikę FSM i agreguje dane sensorów przed renderowaniem UI. `UiState` przenosi aktualny `AppMode`, więc wyświetlacz może reagować na stany Boot, Idle, TagDetected, Measuring, Error i Calibration.
Gdy HX711 jest wyłączony, runtime pozostaje w trybie RFID/UI i brak czujnika wagi nie jest traktowany jako błąd krytyczny.

Po migracji do ESP-IDF ten sam podział na taski pozostaje modelem wykonania firmware.

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
- Ustaw moduł w tryb `I2C` za pomocą `DIP switch`

### Jak testować

1. Wgraj firmware
2. Uruchom monitor portu szeregowego z prędkością `115200`
3. Sprawdź logi startowe i odczyty

Oczekiwane komunikaty:

- `HX711 disabled`, gdy podsystem jest wyłączony w konfiguracji
- `HX711 init OK`
- `HX711 not found`
- odczyty wagi wypisywane cyklicznie przez `Serial` razem z oszacowaną wagą w gramach
- `BtnA clicked` oraz `HX711 zeroed`, gdy baza HX711 zostanie zresetowana w czasie pracy
- `PN532 init OK`
- `PN532 not found`
- UID karty RFID wypisywany po zbliżeniu tagu
- kliknięcia przycisków logowane jako `BtnA clicked`, `BtnB clicked` lub `BtnC clicked`
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

## 🧰 VSCode / ESP-IDF Setup

- Zainstaluj rozszerzenie `ESP-IDF` od Espressif.
- Zainstaluj rozszerzenie Microsoft `C/C++` dla IntelliSense.
- Opcjonalnie doinstaluj `CMake Tools` dla integracji z CMake w VS Code.
- Otworz katalog glownego workspace projektu.
- Aktywuj srodowisko ESP-IDF z `/Users/krzysztof/esp/esp-idf` przed uzyciem terminala.
- Projekt jest skonfigurowany dla `esp32` i korzysta z `sdkconfig.defaults` jako bazy konfiguracji.
- `build/compile_commands.json` sluzy do nawigacji po kodzie i autocomplete.
- Taski w `.vscode/tasks.json` udostepniaja: `Build`, `Flash`, `Monitor`, `Flash + Monitor`, `Full Clean` i `Reconfigure`.

## 🔨 Budowanie i flashowanie

### Taski VS Code

1. `Build` uruchamia `idf.py build`
2. `Flash` uruchamia `idf.py flash`
3. `Monitor` uruchamia `idf.py monitor`
4. `Flash + Monitor` uruchamia `idf.py flash monitor`
5. `Full Clean` uruchamia `idf.py fullclean`
6. `Reconfigure` uruchamia `idf.py reconfigure`

Konfiguracja ESP-IDF używa teraz flasha `16MB`, aby odpowiadala zainstalowanemu modulowi ESP32.

### Ręczny workflow CLI

1. Aktywuj srodowisko ESP-IDF przez `source /Users/krzysztof/esp/esp-idf/export.sh`
2. Uruchom `idf.py reconfigure` po zmianie zaleznosci lub targetu
3. Uruchom `idf.py build`
4. Uruchom `idf.py flash`
5. Uruchom `idf.py monitor`

Predkosc monitora to `115200`. Jesli port szeregowy sie zmieni, zaktualizuj ustawienia VS Code albo wybierz port z palety polecen rozszerzenia ESP-IDF.

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
