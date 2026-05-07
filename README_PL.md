# 🧵 SpoolSense

Wersja angielska: [README.md](README.md)

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

## 🔧 Testy sprzętowe

Projekt zawiera proste testy sprzętowe HX711 i PN532 uruchamiane bezpośrednio z firmware. To są testy do szybkiej weryfikacji okablowania i działania modułów, a nie osobna architektura aplikacji.

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
- dla tagów NTAG213 odczytywana jest strona `0x24` bez zapisu do taga
- w `Serial` pojawiają się: `Page 24: ...`, `Usage: ... s`, `Life: ...%`
- licznik jest interpretowany jako little-endian, a przeliczenie czasu jest heurystyczne dla końcówek Philips Sonicare

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
