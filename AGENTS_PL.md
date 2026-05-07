Document Type: Reference  
Status: Draft  

# AGENTS_PL.md

<small>Ostatnia aktualizacja: 2026-05-07T00:00:00+02:00</small>

## Zasady pracy asystenta (AGENTS) – SpoolSense

### 0. Informacje ogólne o projekcie

- Nazwa projektu: SpoolSense
- Główna technologia / język: C++ / Arduino Framework
- Środowisko docelowe: ESP32 / M5Stack Core ESP32 / PlatformIO
- Repozytorium / struktura kodu: Firmware embedded oparty o PlatformIO + dokumentacja Markdown
- Ostatnia aktualizacja zasad: 2026-05-07

Dokument opisuje współpracę przy projekcie „SpoolSense”.

Szczegóły techniczne dotyczące projektu znajdują się w pliku:

```text
README.md
```

---

## 0.1 Środowisko programistyczne

Projekt rozwijany jest obecnie w:

- języku `C++`
- frameworku `Arduino`
- środowisku `PlatformIO`

Potwierdzenie konfiguracji projektu:

- `platformio.ini`
  - `framework = arduino`
  - `board = m5stack-core-esp32`
- firmware znajduje się w:
  - `src/main.cpp`
  - `src/rfid_test.cpp`
- dokumentacja projektu:
  - `README.md`

Projekt jest obecnie rozwijany w oparciu o:

- `Arduino Framework`
- `PlatformIO`

Docelowym kierunkiem rozwoju projektu jest migracja do:

- `ESP-IDF`

Architektura kodu powinna być projektowana w sposób umożliwiający możliwie bezproblemowe przejście z Arduino Framework do natywnego środowiska ESP-IDF.

Należy:

- minimalizować zależność od specyficznych elementów Arduino
- separować logikę aplikacyjną od warstwy sprzętowej
- ograniczać użycie globalnego stanu
- projektować moduły w sposób kompatybilny z przyszłą architekturą ESP-IDF

Kod powinien być zgodny z:

- dobrymi praktykami embedded C++
- architekturą modularną
- ograniczeniami środowiska Arduino/ESP32
- wymaganiami PlatformIO

---

## 1. Definicje

- **Środowisko robocze** – katalog projektu na tej maszynie.
- **Wiadomość commita** – opis zmian według §4.
- **„— fragment”** – oznacz prezentowany wycinek funkcji/klasy (min. 3–5 linii kontekstu).
- **ISO 8601** – np. `2026-05-07T18:45:12+02:00`.

---

## 2. Ogólne zasady pracy

1. Komunikacja prowadzona jest w języku polskim i w formacie Markdown.
2. Na czacie prezentowane są wyłącznie potrzebne fragmenty kodu lub streszczenia zmian.
3. Duże funkcje i obszerne klasy oznaczaj jako „— fragment”.
4. Zachowuj zgodność z:
   - istniejącą architekturą projektu
   - aktualnym stylem kodu
   - modularnością projektu
   - nowoczesnymi praktykami embedded C++
5. Wprowadzaj zmiany iteracyjnie.
6. Nie wykonuj pushowania zmian bez wyraźnej zgody użytkownika.
7. Przed dodaniem nowych helperów sprawdź:
   - `helpers.md`
   - istniejące moduły narzędziowe
8. Przy modyfikacji dokumentacji (`*.md`) aktualizuj:

   ```html
   <small>Ostatnia aktualizacja: YYYY-MM-DDThh:mm:ss±hh:mm</small>
   ```

9.  Wszystkie daty podawaj w lokalnej strefie użytkownika.
11. README.md zawiera szczegóły techniczne projektu — nie duplikuj ich w innych dokumentach.
12. Kod i dokumentacja muszą być utrzymywane zgodnie z najlepszymi praktykami inżynierii oprogramowania.
13. Preferowana jest architektura modularna i łatwa do dalszej rozbudowy.
14. Kod firmware należy pisać zgodnie z praktykami dla:
    - embedded C++
    - Arduino Framework
    - ESP32
    - PlatformIO
15. Projektować kod z myślą o przyszłej migracji do ESP-IDF.

---

## 3. Podział ról

### 🤖 ChatGPT

ChatGPT odpowiada za:

- analizę architektury projektu
- konsultacje techniczne
- analizę jakości implementacji
- analizę wyników pracy CODEX
- proponowanie kierunków rozwoju
- wsparcie projektowe i dokumentacyjne

ChatGPT:

- nie wykonuje rzeczywistych commitów Git
- nie zastępuje procesu implementacji wykonywanego przez CODEX
- pełni rolę doradczą i analityczną

---

### 💻 CODEX

CODEX odpowiada za:

- implementację kodu
- testowanie działania systemu
- rozwój firmware
- refaktoryzację
- utrzymywanie spójności kodu
- wykonywanie commitów
- przygotowywanie opisów commitów
- aktualizację dokumentacji technicznej

CODEX:

- implementuje zmiany zgodnie z wymaganiami projektu
- odpowiada za poprawność techniczną implementacji
- wykonuje testy funkcjonalne
- przestrzega workflow Git projektu

---

## 4. Zarządzanie zmianami i commitami

### Standard commitów

Projekt wykorzystuje standard:

- Conventional Commits

Preferowany format:

```text
type(scope): short description
```

Przykłady:

```text
feat(rfid): add PN532 tag parsing
fix(hx711): prevent unstable weight readings
refactor(core): split hardware abstraction layer
docs(readme): update wiring section
test(rfid): add hardware initialization tests
chore(ci): update PlatformIO configuration
```

---

### Dozwolone typy commitów

| Typ      | Znaczenie                                 |
|----------|-------------------------------------------|
| feat     | nowa funkcjonalność                       |
| fix      | poprawka błędu                            |
| refactor | refaktoryzacja bez zmiany funkcjonalności |
| docs     | zmiany dokumentacji                       |
| test     | testy                                     |
| chore    | zmiany techniczne / utrzymaniowe          |
| build    | zmiany build systemu                      |
| ci       | konfiguracja CI/CD                        |
| perf     | optymalizacje wydajności                  |

---

### Zasady commitów

1. Commit musi być:
   - logiczny
   - spójny
   - mały i czytelny
   - łatwy do review

2. Jeden commit powinien realizować:
   - jedną funkcjonalność
   - jedną poprawkę
   - jedną logiczną zmianę

3. Nie mieszaj w jednym commitcie:
   - refaktoryzacji
   - nowych funkcji
   - zmian dokumentacyjnych
   - zmian formatowania

4. WIP commit:
   - dozwolony lokalnie
   - nie powinien trafiać do `main`

5. Przed finalizacją:
   - wykonaj squash WIP
   - przygotuj finalny logiczny commit

6. Commit message:
   - pisz w języku angielskim
   - używaj czasu teraźniejszego
   - bez kropki na końcu
   - maksymalnie konkretny

7. Nie generuj automatycznych changelogów.

8. Szczegółowy opis commita przygotowuj wyłącznie na żądanie użytkownika.

9. Pomijaj:
   - `.reports/junit.xml`
   - `.reports/tests.html`

10. Commity dokumentacyjne (`*.md`) mogą posiadać uproszczony opis.

---

## 5. Workflow Git

Projekt wykorzystuje workflow oparty o branche zadaniowe.

### Zasady

1. Jeden branch = jedno zadanie.
2. Nie pracuj bezpośrednio na:

   ```text
   main
   ```

3. Dopuszczalne są lokalne commity WIP.
4. Przed finalizacją:
   - wykonaj squash commitów WIP
   - przygotuj logiczny commit końcowy
5. Commit message musi być:
   - czytelny
   - techniczny
   - jednoznaczny

---

## 6. Aktualizacja README.md

README.md aktualizuj wyłącznie przy:

- istotnych zmianach funkcjonalnych
- zmianach sprzętowych
- zmianach konfiguracji
- nowych procedurach uruchomieniowych

Nie aktualizuj README dla:

- drobnych zmian UI
- kosmetycznych refaktoryzacji
- lokalnych testów

---

## 7. Testowanie kodu

1. Nowe funkcje muszą posiadać testy.
2. Po każdej zmianie uruchamiaj odpowiedni zestaw testów.
3. Nie proponuj kolejnych zmian przy niezaliczonych testach.
4. Testy długotrwałe wymagają zgody użytkownika.
5. Nie twórz sztucznych atrap sprzętu.
6. Testy sprzętowe mogą być pomijane przy braku dostępu do hardware.
7. Artefakty testowe zapisuj w:

   ```text
   .run/TESTS
   ```

8. Testy firmware powinny uwzględniać:

   - ograniczenia środowiska embedded
   - realny hardware ESP32/M5Stack
   - komunikację I2C/SPI/UART
   - stabilność inicjalizacji urządzeń
  
9.  Nie zastępuj testów sprzętowych pełnymi mockami hardware bez wyraźnej potrzeby.

---

## 8. Organizacja katalogów

### py_tools/

Współdzielone narzędzia między projektami.

Modyfikacje:

- wyłącznie po uzyskaniu zgody użytkownika

---

### tools/

Lokalne narzędzia projektu.

Mogą zostać:

- przeniesione do `py_tools/`
- ustandaryzowane po stabilizacji

---

### workspace/

Katalog eksperymentalny / sandbox.

Kod po stabilizacji powinien zostać:

- przeniesiony do docelowej struktury projektu

---

## 9. Styl i linting kodu

Kod musi przechodzić:

- `clang-format`
- `cppcheck`
- lokalne reguły projektu

### Wymagania

1. ASCII-only w kodzie źródłowym.
2. Include wyłącznie na początku pliku.
3. Nazwy:

   ```text
   lower_snake_case
   ```

4. Zachowuj zgodność z:
   - nowoczesnym C++
   - modularnością projektu
5. Jedna pusta linia na końcu pliku.
6. Brak trailing whitespace.
7. Nieużywane zmienne oznaczaj prefiksem:

   ```cpp
   _
   ```

---

### Embedded / Arduino / ESP-IDF Ready

1. Preferuj:
   - klasy i moduły zamiast dużych plików proceduralnych
   - separację HAL / logiki biznesowej
   - minimalizację dynamicznej alokacji pamięci
   - unikanie blokujących `delay()` poza kodem testowym

2. Unikaj:
   - globalnego mutable state bez uzasadnienia
   - nadmiernego użycia `String`
   - silnego sprzężenia modułów

3. Preferowane są:
   - `constexpr`
   - `enum class`
   - `std::array`
   - RAII tam gdzie ma sens

4. Logika sprzętowa powinna być izolowana od logiki aplikacyjnej.

5. Kod powinien być możliwie łatwy do migracji do:
   - ESP-IDF
   - FreeRTOS
   - architektury komponentowej ESP32

6. Unikaj nadmiernego uzależnienia od:
   - klas Arduino specyficznych dla frameworka
   - globalnych singletonów
   - logiki umieszczonej bezpośrednio w `loop()`

---

## 10. Header modułów C++

Każdy moduł `.cpp` oraz `.hpp` powinien rozpoczynać się nagłówkiem komentarza.

### Wymagana struktura

```cpp
/**
 * Short summary sentence.
 *
 * Features (EN):
 * - ...
 *
 * Funkcje (PL):
 * - ...
 *
 * File: relative/path.cpp
 */
```

---

## 11. Workflow pracy CODEX

1. Dla prostych zmian:
   - brak rozbudowanych preambuł
2. Dla większych zmian:
   - stosuj plan działania
3. Konflikty zasad:
   - zgłaszaj natychmiast
4. Operacje destrukcyjne:
   - wymagają zgody użytkownika
5. Testy:
   - obowiązkowe po zmianach funkcjonalnych

---

## 12. Precedencja zasad

1. Obowiązuje dokument najbliżej modyfikowanego pliku.
2. Lokalne reguły mają wyższy priorytet.
3. Konflikty zasad należy zgłaszać użytkownikowi.

---

## 13. Dokumentacja projektu

Dokumentacja projektu dzieli się na:

| Dokument  | Przeznaczenie                |
|-----------|------------------------------|
| README.md | szczegóły techniczne         |
| AGENTS.md | zasady współpracy i workflow |

---

## 13.1 Język dokumentacji

Podstawowym językiem dokumentacji projektu jest:

- język angielski

Dokumentacja techniczna powinna być tworzona przede wszystkim w wersji angielskiej.

Wyjątki:

- wybrane dokumenty mogą posiadać polskie tłumaczenie

Konwencja nazewnictwa:

| Typ                    | Przykład       |
|------------------------|----------------|
| wersja podstawowa (EN) | `README.md`    |
| wersja polska.         | `README_PL.md` |

Zasady:

1. Wersja angielska jest źródłem referencyjnym.
2. Wersje `_PL.md` są tłumaczeniem dokumentacji podstawowej.
3. Dokumentacja techniczna API, architektury i firmware powinna być tworzona w języku angielskim.
4. Dokumentacja użytkowa może posiadać wersje wielojęzyczne.

---

## 14. Podsumowanie

Projekt SpoolSense rozwijany jest zgodnie z:

- modularną architekturą
- najlepszymi praktykami programowania
- uporządkowanym workflow Git
- zasadami czytelnej dokumentacji

Rozdzielenie odpowiedzialności pomiędzy ChatGPT i CODEX ma zapewnić:

- wysoką jakość implementacji
- lepszą analizę architektury
- łatwiejsze utrzymanie projektu
- większą przewidywalność procesu rozwoju

Projekt należy traktować jako firmware embedded rozwijany obecnie w środowisku Arduino Framework na platformie ESP32 z wykorzystaniem PlatformIO, z docelową migracją do natywnego środowiska ESP-IDF.
