# Audyt gałęzi — 2026-11-21

Zakres: zdalne gałęzie GitHub na tle `main` **przed dodaniem tego raportu**. `Ahead/behind`: commity unikalne na gałęzi / brakujące na niej względem `main`. Opis bazuje na porównaniu plików i typów zmian; ocena funkcjonalna wymaga kwalifikacji kodu i testów. Lokalne gałęzie i worktree nie są widoczne w GitHub API.

Wszystkie cztery gałęzie mają unikalną historię i duże drzewa (ponad 120 plików różnic). Część śledzi pliki `build/`; przed integracją należy oddzielić źródła od artefaktów kompilacji. `main` jest małym szkieletem PlatformIO; nie przedstawiać prototypów z innych gałęzi jako obecnych na `main`.

| Gałąź | Ostatni commit UTC | Ahead/behind | Zawartość techniczna / uzasadnienie | Rekomendacja |
|---|---|---:|---|---|
| `feature/home-screen-mvp` | 2026-05-10 | 45/0 | Firmware ekranu głównego i renderingu, CMake/PlatformIO, `src`, dokumentacja; także pliki build. | **[DO PRZEGLĄDU]** |
| `feature/hx711-rfid-tests` | 2026-05-10 | 42/0 | Testy HX711/RFID, diagnostyka startowa i LCD, zmiany w firmware oraz plikach build. | **[DO PRZEGLĄDU]** |
| `feature/multi-screen-runtime` | 2026-05-10 | 47/0 | Wieloekranowy runtime i komunikaty tare/zapamiętania odniesienia, firmware, build. | **[DO PRZEGLĄDU]** |
| `feature/rfid-screen-mvp` | 2026-05-10 | 52/0 | Ekran RFID i informacja o pozostałym filamencie, firmware, build. | **[DO PRZEGLĄDU]** |

## Warunek integracji
Najpierw wybrać najbardziej kompletną wspólną bazę zmian, porównać gałęzie między sobą, usunąć z planowanego diffu artefakty `build/` i przetestować na właściwym urządzeniu. Nie łączyć czterech gałęzi mechanicznie.

Nie dokonano merge, kasowania gałęzi, force-push ani uruchomienia systemów zewnętrznych. Sam fakt obecności unikalnych commitów nie dowodzi gotowości do integracji.
