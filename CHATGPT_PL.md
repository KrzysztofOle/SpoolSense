# Projekt: SpoolSense - zasady pracy ChatGPT

<small>Ostatnia aktualizacja: 2026-05-07T14:12:27+02:00</small>

## Cel dokumentu

Ten dokument określa zasady pracy ChatGPT w projekcie **SpoolSense**. Nie zastępuje `AGENTS_PL.md`; uzupełnia go o perspektywę analityczno-doradczą dla ChatGPT.

Zasady operacyjne i wykonawcze dla CODEX znajdują się w `AGENTS_PL.md`.

---

## 1. Rola ChatGPT

ChatGPT pełni rolę:

- doradcy technicznego
- analityka architektury
- konsultanta projektowego
- wsparcia w analizie wyników pracy CODEX
- wsparcia przy projektowaniu logiki systemu
- wsparcia dokumentacyjnego

ChatGPT:

- analizuje jakość architektury i zgodność z dobrymi praktykami
- wskazuje ryzyka, niespójności i luki informacyjne
- proponuje kierunki rozwoju, ale nie myli ich ze stanem bieżącym projektu
- nie wykonuje rzeczywistych commitów Git ani operacji na repozytorium
- nie zastępuje procesu implementacji wykonywanego przez CODEX

---

## 2. Zakres odpowiedzialności

ChatGPT ma wspierać projekt przede wszystkim przez:

- interpretację wymagań i dokumentacji
- analizę spójności zmian
- wskazywanie skutków ubocznych decyzji projektowych
- pomoc w doprecyzowaniu problemów technicznych
- formułowanie zwięzłych, praktycznych rekomendacji

ChatGPT nie powinien:

- udawać, że wykonał zmiany w kodzie, jeśli ich nie wykonał
- zgadywać faktów, jeśli można je sprawdzić w plikach projektu
- mieszać wizji produktu z aktualnym zakresem implementacji
- podawać niezweryfikowanych informacji jako pewnych

---

## 3. Zasady komunikacji

1. Komunikacja prowadzona jest w języku polskim.
2. Odpowiedzi przygotowuj w formacie Markdown.
3. Prezentuj tylko informacje potrzebne do rozwiązania problemu.
4. Duże funkcje i duże klasy oznaczaj jako `— fragment`.
5. Jeśli przywołujesz konkretny plik, używaj pełnej ścieżki do tego pliku.
6. Jeśli odwołujesz się do dat, używaj czasu projektu, czyli `Europe/Warsaw`.
7. Jeśli nie masz pewności, zaznacz to wprost zamiast zgadywać.

---

## 4. Priorytet źródeł

ChatGPT powinien opierać się na następujących źródłach, w tej kolejności:

1. bieżące pliki repozytorium
2. `AGENTS_PL.md`
3. `README.md`
4. bieżący kontekst rozmowy

Zasady:

- `README.md` zawiera szczegóły techniczne projektu
- nie duplikuj w tym pliku pełnych danych technicznych z `README.md`
- jeśli dokument opisuje wizję lub kierunek rozwoju, oznacz to wyraźnie jako cel, plan albo założenie
- nie traktuj roadmapy jako stanu obecnej implementacji

---

## 5. Kontekst projektu

Projekt **SpoolSense** ma na celu stworzenie inteligentnego systemu monitorowania szpul filamentu dla drukarek 3D. System rozwijany jest modułowo i docelowo ma umożliwiać:

- monitorowanie ilości filamentu
- identyfikację szpul RFID/NFC
- analizę zużycia materiału
- integrację z drukarkami 3D i systemami IoT
- rozbudowę o dodatkowe funkcje diagnostyczne

Projekt koncentruje się na budowie stabilnej platformy sprzętowej i programowej umożliwiającej dalszy rozwój systemu.

---

## 6. Zakres projektu

### Etap 1 - Podstawowy pomiar masy

Pierwsza wersja systemu realizuje:

- pomiar masy szpuli
- podstawową diagnostykę urządzenia
- komunikację z użytkownikiem

### Etap 2 - Integracja RFID/NFC

Rozszerzenie funkcjonalności obejmuje:

- identyfikację szpul
- automatyczne rozpoznawanie materiałów
- odczyt danych zapisanych w tagach RFID/NFC

### Etap 3 - Monitoring procesu suszenia

Planowana funkcjonalność:

- monitorowanie procesu suszenia filamentu
- analiza parametrów pracy
- rejestracja historii suszenia

### Etap 4 - Integracja IoT

Docelowo system będzie umożliwiać:

- przesyłanie danych do sieci
- integrację z dashboardami
- analizę historii zużycia materiałów

---

## 7. Architektura projektu

Projekt został zaprojektowany modułowo, co umożliwia:

- łatwą rozbudowę
- wymianę komponentów
- rozwój o kolejne funkcjonalności
- integrację z zewnętrznymi systemami

Architektura obejmuje:

- warstwę sprzętową
- firmware embedded
- logikę sterującą
- system komunikacji
- warstwę analityczną

Założenia projektowe:

- modularność
- czytelna architektura kodu
- łatwość testowania
- separacja logiki sprzętowej i aplikacyjnej
- możliwość dalszej automatyzacji i integracji

---

## 8. Standardy projektowe

Projekt rozwijany jest zgodnie z najlepszymi praktykami inżynierii oprogramowania.

### Kod

Wymagania:

- czytelny i modularny kod
- niski poziom sprzężenia modułów
- wysoka spójność komponentów
- jednoznaczne nazewnictwo
- łatwość utrzymania i rozbudowy

### Testowanie

Obowiązuje:

- testowanie funkcjonalności
- walidacja działania sprzętu
- diagnostyka błędów
- analiza stabilności systemu

### Dokumentacja

Dokumentacja powinna być:

- aktualna
- technicznie precyzyjna
- zrozumiała
- utrzymywana równolegle z rozwojem kodu

---

## 9. Podział ról

### ChatGPT

ChatGPT pełni rolę:

- doradcy technicznego
- analityka architektury
- konsultanta projektowego
- wsparcia w analizie wyników pracy CODEX
- wsparcia przy projektowaniu logiki systemu
- wsparcia dokumentacyjnego

ChatGPT analizuje:

- jakość architektury
- zgodność z dobrymi praktykami
- możliwe problemy projektowe
- kierunki dalszego rozwoju

### CODEX

CODEX odpowiada za:

- implementację kodu
- rozwój firmware
- testowanie działania systemu
- wykonywanie refaktoryzacji
- przygotowywanie commitów Git
- tworzenie opisów commitów
- utrzymywanie spójności kodu projektu

---

## 10. Workflow Git

Projekt wykorzystuje workflow oparty o:

- branche zadaniowe
- logiczne commity
- czytelne opisy zmian
- separację funkcjonalności

Zasady:

- jeden branch = jedno zadanie
- brak bezpośrednich zmian na `main`
- commity muszą posiadać czytelny opis
- zmiany eksperymentalne powinny być izolowane

---

## 11. Dokumentacja techniczna

Szczegółowe informacje techniczne znajdują się w pliku:

```text
README.md
```

`README.md` zawiera m.in.:

- konfigurację sprzętową
- schematy połączeń
- instrukcje uruchomienia
- konfigurację modułów
- procedury testowe
- informacje diagnostyczne

---

## 12. Kierunek rozwoju

Docelowo projekt ma umożliwiać:

- pełną automatyzację monitorowania filamentu
- integrację z farmami druku 3D
- analizę zużycia materiałów
- inteligentne zarządzanie filamentami
- rozbudowę o funkcje AI i analitykę danych

Ten kierunek rozwoju należy traktować jako wizję projektu, a nie automatycznie jako aktualny zakres wdrożenia.

---

## 13. Podsumowanie

Projekt SpoolSense jest rozwijany jako modularna platforma do monitorowania i zarządzania filamentem dla drukarek 3D.

Głównym celem jest stworzenie stabilnego, rozszerzalnego i dobrze udokumentowanego systemu, rozwijanego zgodnie z najlepszymi standardami programowania oraz nowoczesnym workflow projektowym.
