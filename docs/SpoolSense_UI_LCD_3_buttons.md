# SpoolSense - architektura UI LCD + 3 przyciski

<small>Ostatnia aktualizacja: 2026-05-10T12:49:48+02:00</small>

## Zakres

Dokument powstał po analizie archiwum `SpoolSense-feature-hx711-rfid-tests-2.zip`.

Uwzględniono aktualny stan firmware:

- ESP-IDF / C++
- M5Stack Core ESP32
- LCD ILI9342C 320x240
- BtnA / BtnB / BtnC
- taski FreeRTOS
- komponenty `app`, `board`, `display`, `hx711`, `pn532`, `diagnostics`

## Kluczowe sprostowanie

**Waga referencyjna** oznacza masę pełnej, nowej szpuli filamentu po procesie suszenia.

Nie jest to:

- masa pustej szpuli
- tara HX711
- odważnik kalibracyjny

## Główna rekomendacja UI

Ekrany:

1. HOME - dane szpuli z RFID + masa z HX711
2. DIAGNOSTYKA - stan PN532, HX711, LCD, przycisków, AXP192, I2C
3. WAGA - masa, tara, kalibracja, zapis referencji pełnej wysuszonej szpuli
4. RFID - odczyt, edycja i zapis danych filamentu
5. USTAWIENIA / INFO - wersja, konfiguracja, opcje serwisowe

## Mapowanie przycisków

| Przycisk | Klik krótki | Długie przytrzymanie |
|---|---|---|
| BtnA | poprzedni / minus / wstecz | HOME |
| BtnB | OK / akcja | menu akcji |
| BtnC | następny / plus | akcja kontekstowa |

## Ważna zmiana względem obecnego kodu

Obecnie BtnA zeruje HX711 globalnie w `components/app/src/app.cpp`.

Docelowo:

- BtnA nie powinien globalnie zerować wagi
- zerowanie powinno być akcją tylko na ekranie WAGA
- zapis referencji powinien być osobną akcją: `Zapis ref.`

## Model obliczeń

```text
used_weight_g = reference_full_weight_g - current_weight_g
remaining_percent = current_weight_g / reference_full_weight_g * 100
```

Jeżeli w przyszłości system ma liczyć sam filament bez masy szpuli, należy dodać osobne pole:

```text
empty_spool_weight_g
```

Nie należy mylić go z `reference_full_weight_g`.

## Proponowany rekord szpuli

```json
{
  "schema": 1,
  "material": "PLA",
  "color": "Black",
  "diameter_mm": 1.75,
  "manufacturer": "Prusa",
  "nozzle_temp_c": 215,
  "bed_temp_c": 60,
  "reference_full_weight_g": 1000,
  "drying_profile": "PLA_45C_4H",
  "last_dried_at": "2026-05-10T12:49:48+02:00"
}
```

## Proponowany komponent UI

```text
components/ui/
  CMakeLists.txt
  idf_component.yml
  include/ui/ui_controller.hpp
  include/ui/ui_screen.hpp
  include/ui/ui_actions.hpp
  src/ui_controller.cpp
  src/screens/home_screen.cpp
  src/screens/diagnostics_screen.cpp
  src/screens/scale_screen.cpp
  src/screens/rfid_screen.cpp
  src/screens/settings_screen.cpp
```

## Docelowy przepływ danych

```text
PN532 Task  ---- RFID Event ----+
                                |
HX711 Task  ---- Weight Event --+--> App Task --> AppState --> UiState
                                |                     |
Buttons ---- Button Event ------+                     v
                                                  UI Controller
                                                       |
                                                       v
                                                  Display Renderer
```

## Kolejność wdrożenia

1. Dodać `UiScreen` do `AppState` / `UiState`.
2. Zmienić obsługę BtnA/B/C na nawigację ekranów.
3. Przenieść globalne zerowanie HX711 do ekranu WAGA.
4. Dodać `SpoolData` z `reference_full_weight_g`.
5. Dodać obliczanie `used_weight_g` i `remaining_percent`.
6. Powiązać UID RFID z rekordem szpuli.
7. Dodać zapis referencji pełnej wysuszonej szpuli.
8. Rozdzielić dane minimalne RFID od pełnych danych w NVS.

## Ryzyka

| Ryzyko | Skutek | Rekomendacja |
|---|---|---|
| BtnA jako globalne zero | przypadkowe zerowanie wagi | przenieść do ekranu WAGA |
| mieszanie tary z referencją | błędne obliczenia filamentu | rozdzielić pojęcia w UI |
| zbyt duży rekord RFID | brak miejsca na tagu | UID + NVS lub format binarny |
| rozrost `app.cpp` | trudne utrzymanie | wydzielić `components/ui` |
| brak long press | słaba ergonomia | dodać akcję `kLongPress` |

## Rekomendacja końcowa

Najlepszy kierunek:

```text
App FSM = stan urządzenia
UI FSM  = aktywny ekran i akcje użytkownika
Display = czyste renderowanie
Storage = dane szpul i referencje
```
