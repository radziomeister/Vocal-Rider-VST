# Vocal Rider VST

Wtyczka VST3 do automatycznego "ridingu" (wyrównywania) dynamiki sygnału
wokalnego, wzorowana na koncepcji Waves Vocal Rider. Zbudowana od zera w
frameworku JUCE, bez żadnych zależności od oryginalnego produktu.

## Co robi

Wtyczka śledzi poziom sygnału wejściowego (RMS) i w czasie rzeczywistym
podbija lub ścina wzmocnienie tak, aby utrzymać go w okolicy zadanego
poziomu docelowego (Target) - dokładnie tak jak automatyczne "jeżdżenie po
faderze" wykonywane ręcznie przez inżyniera miksu.

## Kontrolki (suwaki / gałki)

- **Target** - docelowy poziom głośności (dB), do którego dąży sygnał.
- **Range Down / Range Up** - maksymalny zakres, o jaki wtyczka może
  odpowiednio wyciszyć / podbić sygnał.
- **Attack** - czas reakcji, gdy rider podbija wzmocnienie (sygnał zbyt cichy).
- **Release** - czas reakcji, gdy rider zmniejsza wzmocnienie (sygnał zbyt
  głośny). Niezależne od Attack - można ustawić np. szybki atak i wolne
  opadanie, żeby dobrać idealny balans agresywności/łagodności ridingu.
- **Output** - trym wyjściowy (dB), niezależny od ridingu.
- **Listen: Vocal / Music** - źródło analizy poziomu: `Vocal` analizuje
  sam sygnał wejściowy, `Music` pozwala podłączyć sidechain (np. z busa
  muzyki) i sterować ridingiem względem innego sygnału.
- **Rider ON/OFF** - włącznik/wyłącznik automatyzacji (bypass gain-ridingu).
- Wykres na górze pokazuje na żywo: poziom sygnału (niebieska linia), target
  (żółta przerywana linia) i aktualnie stosowane wzmocnienie (zielona linia).

## Instalacja (bez instalatora)

```bash
./install.sh
```

Kopiuje gotowy `Vocal Rider VST.vst3` do `~/Library/Audio/Plug-Ins/VST3/`
i podpisuje go ad-hoc (`codesign --sign -`), żeby Gatekeeper nie blokował
lokalnie zbudowanej, niepodpisanej wtyczki.

Po instalacji w Reaperze: **Options > Preferences > Plug-ins > VST >
Re-scan**. Wtyczka pojawi się jako **Vocal Rider VST (OpenRider)** -
zweryfikowane, że Reaper poprawnie ją skanuje i wpisuje do cache
pluginów bez błędów.

## Instalator (przenoszenie na inny komputer)

Żeby przenieść wtyczkę na inny Mac (Apple Silicon) bez ręcznego kopiowania
plików:

```bash
./build_installer.sh
```

Tworzy `Vocal Rider VST Installer.pkg` - podwójne kliknięcie uruchamia
standardowy instalator macOS, który:

- pozwala wybrać **"Zainstaluj dla wszystkich użytkowników"** (zalecane,
  `/Library/Audio/Plug-Ins/VST3`, wymaga Touch ID / hasła administratora)
  albo **"Zainstaluj tylko dla mnie"** (`~/Library/Audio/Plug-Ins/VST3`,
  bez uprawnień administratora),
- obie lokalizacje to domyślne ścieżki skanowane przez Reapera - po
  instalacji wystarczy zrobić Re-scan wtyczek, bez przenoszenia plików.

Pakiet **nie jest podpisany certyfikatem Apple Developer ID** (nie mamy
płatnego konta deweloperskiego) - na innym komputerze macOS prawdopodobnie
pokaże ostrzeżenie o nieznanym deweloperze. Wtedy: kliknij `.pkg` prawym
przyciskiem → **Otwórz**, albo zezwól w **Ustawienia systemowe → Prywatność
i bezpieczeństwo**.

## Budowanie ze źródeł

Wymagania: Xcode Command Line Tools, Python3 (`pip3 install --user cmake
ninja` jeśli nie masz `cmake`/`ninja` z Homebrew).

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
./install.sh
```

## Ograniczenia / uwagi

- Build wykonany na tym Macu jest **natywny arm64 (Apple Silicon)**.
  Jeśli potrzebujesz wersji dla Intel Mac lub dla **Windows/FL Studio na
  PC**, to wymaga osobnego builda na/dla tamtej platformy (cross-compile
  z Mac na Windows jest zawodny) - daj znać, jeśli to potrzebne.
- Format **AU** został celowo pominięty: FL Studio w ogóle go nie
  obsługuje, a nasza konfiguracja z opcjonalną szyną sidechain nie
  przechodzi rygorystycznej walidacji `auval` Apple. VST3 (który wspiera
  Reaper i FL Studio na Macu) jest w pełni zwalidowany i działa poprawnie.
- To autorska implementacja "na bazie" koncepcji Vocal Ridera, nie klon
  kodu ani grafiki oryginalnego produktu Waves.
