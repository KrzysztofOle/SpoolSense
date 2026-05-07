# 🤖 Projekt: SpoolSense

## 🎯 Cel projektu

Projekt **SpoolSense** ma na celu stworzenie inteligentnego systemu monitorowania szpul filamentu dla drukarek 3D.
System rozwijany jest modułowo i docelowo ma umożliwiać:

- monitorowanie ilości filamentu
- identyfikację szpul RFID/NFC
- analizę zużycia materiału
- integrację z drukarkami 3D i systemami IoT
- rozbudowę o dodatkowe funkcje diagnostyczne

Projekt koncentruje się na budowie stabilnej platformy sprzętowej i programowej umożliwiającej dalszy rozwój systemu.

---

## 🧩 Zakres projektu

### Etap 1 — Podstawowy pomiar masy

Pierwsza wersja systemu realizuje:

- pomiar masy szpuli
- podstawową diagnostykę urządzenia
- komunikację z użytkownikiem

### Etap 2 — Integracja RFID/NFC

Rozszerzenie funkcjonalności obejmuje:

- identyfikację szpul
- automatyczne rozpoznawanie materiałów
- odczyt danych zapisanych w tagach RFID/NFC

### Etap 3 — Monitoring procesu suszenia

Planowana funkcjonalność:

- monitorowanie procesu suszenia filamentu
- analiza parametrów pracy
- rejestracja historii suszenia

### Etap 4 — Integracja IoT

Docelowo system będzie umożliwiać:

- przesyłanie danych do sieci
- integrację z dashboardami
- analizę historii zużycia materiałów

---

## 🏗️ Architektura projektu

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

---

## ⚙️ Założenia projektowe

Projekt rozwijany jest zgodnie z następującymi założeniami:

- modularność
- czytelna architektura kodu
- łatwość testowania
- separacja logiki sprzętowej i aplikacyjnej
- możliwość dalszej automatyzacji i integracji

---

## 👥 Podział ról

### 🤖 ChatGPT

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

---

### 💻 CODEX

CODEX odpowiada za:

- implementację kodu
- rozwój firmware
- testowanie działania systemu
- wykonywanie refaktoryzacji
- przygotowywanie commitów Git
- tworzenie opisów commitów
- utrzymywanie spójności kodu projektu
CODEX realizuje:
- zadania programistyczne
- integrację modułów
- poprawki błędów
- rozwój funkcjonalności

---

## 📐 Standardy projektowe

Projekt rozwijany jest zgodnie z najlepszymi praktykami inżynierii oprogramowania.

### 📚 Kod

Wymagania:

- czytelny i modularny kod
- niski poziom sprzężenia modułów
- wysoka spójność komponentów
- jednoznaczne nazewnictwo
- łatwość utrzymania i rozbudowy

### 🧪 Testowanie

Obowiązuje:

- testowanie funkcjonalności
- walidacja działania sprzętu
- diagnostyka błędów
- analiza stabilności systemu

### 📝 Dokumentacja

Dokumentacja powinna być:

- aktualna
- technicznie precyzyjna
- zrozumiała
- utrzymywana równolegle z rozwojem kodu
  
---

## 🔄 Workflow Git

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

## 📖 Dokumentacja techniczna

Szczegółowe informacje techniczne znajdują się w pliku:

```text
README.md
```

README zawiera m.in.:

- konfigurację sprzętową
- schematy połączeń
- instrukcje uruchomienia
- konfigurację modułów
- procedury testowe
- informacje diagnostyczne

⸻

🚀 Kierunek rozwoju

Docelowo projekt ma umożliwiać:

- pełną automatyzację monitorowania filamentu
- integrację z farmami druku 3D
- analizę zużycia materiałów
- inteligentne zarządzanie filamentami
- rozbudowę o funkcje AI i analitykę danych

⸻

📝 Podsumowanie

Projekt SpoolSense jest rozwijany jako modularna platforma do monitorowania i zarządzania filamentem dla drukarek 3D.

Głównym celem projektu jest stworzenie stabilnego, rozszerzalnego i dobrze udokumentowanego systemu, rozwijanego zgodnie z najlepszymi standardami programowania oraz nowoczesnym workflow projektowym.