# 06 — Ostatné súbory a slovník pojmov

Predchádzajúce lekcie rozobrali aktívny kód ESP32Logger aj RTULogScope. Táto lekcia vysvetľuje nastavenia zostavenia, pomocné súbory a obsah repozitára, ktorý nie je priamo súčasťou meracieho cyklu.

## 1. Repozitár a riešenie Visual Studia

### `RTULogSuite.sln`

Súbor `.sln` je **solution** (riešenie Visual Studia). Je to zoznam projektov a nastavení, ktoré IDE otvorí spolu. V tomto prípade uvádza:

- `RTULogScope/RTULogScope.csproj`
- `ESP32Logger/ESP32Logger.csproj`

**IDE** je vývojové prostredie — program, v ktorom sa kód píše, zostavuje a ladí. Samotný súbor `.sln` neobsahuje aplikačný kód.

### `README.md` v koreňovom priečinku

Hlavné README predstavuje projekt ako kombináciu ESP32 záznamníka a Windows grafickej aplikácie. Je dobrý úvod do účelu, ale niektoré tvrdenia nezodpovedajú aktuálnej implementácii. Najmä uvádza CSV, hoci firmvér zapisuje JSON riadky. Pri konkrétnom správaní preto uprednostni vysvetlenie založené na zdrojovom kóde v lekciách 01–05.

### `.gitattributes` a `.gitignore`

- `.gitattributes` nastavuje všeobecné zaobchádzanie s textovými súbormi a koncami riadkov.
- Koreňový `.gitignore` hovorí Gitu, ktoré lokálne súbory nemá zahrnúť do odovzdania, napríklad dočasné výstupy Visual Studia, priečinky `bin` a `obj` či balíčky NuGet.
- `RTULogScope/gitignore` je ďalší súbor s pravidlami ignorovania v priečinku desktopovej aplikácie. Jeho názov sa nekončí na `.gitignore`, takže Git ho automaticky nemusí považovať za bežný ignorovací súbor.

**Git** je nástroj na sledovanie histórie zmien. **Commit** je pomenovaný záznam zmien; **push** odošle lokálne commity na vzdialený GitHub.

## 2. Nastavenia časti ESP32Logger

### `ESP32Logger/ESP32Logger.csproj`

`.csproj` je projektový súbor pre C# a MSBuild. Obsahuje odkazy na `.NET Framework 4.8`, nastavenia Debug/Release a zoznam súborov. V zozname sú aj Arduino súbory `.ino` a C++ `.cpp/.h`, ale tento súbor ich nemení na zostaviteľný program Arduino.

Preto ho nepovažuj automaticky za návod na zostavenie ESP32 firmvéru. Skutočný vstup firmvéru je `ESP32Logger/src/main/main.ino`; README opisuje otvorenie v Arduino IDE.

### `ESP32Logger/App.config`

Konfiguračný súbor pre aplikácie .NET Framework. Uvádza podporovanú verziu .NET runtime. Arduino firmvér ho pri meraní nepoužíva.

### Dve ukážky konfigurácie

`ESP32Logger/config/config.json` a `ESP32Logger/SD Card/config/config.json` sú podľa porovnania obsahu totožné ukážky. Prvý je v zdrojovom priečinku; druhý ukazuje, kam sa konfigurácia kopíruje na SD karte.

Kód však vyžaduje, aby sa súbor na karte nachádzal pod cestou `/config/config.json`. Názov priečinka vo Windows a cesta na karte sú len dve reprezentácie tej istej adresárovej štruktúry.

### Dokumenty a obrázky ESP32Logger

- `ESP32Logger/README.md` obsahuje rýchly návod k hardvéru a konfigurácii. Odkazuje na `config_json_documentation.md`, ktorý repozitár neobsahuje.
- `ESP32Logger/hardware_wiring_schematic.md` opisuje zapojenie a je potrebné ho čítať spolu s upozornením o odlišných pinoch v lekcii 03.
- `ESP32Logger/Assets/zapojenie.jpg` a `ESP32Logger/Assets/zostava.jpg` sú obrázky zapojenia a fyzickej zostavy. Obrázok je binárny súbor: neobsahuje zdrojový kód, ale vizuálnu dokumentáciu.

## 3. Nastavenia časti RTULogScope

### `RTULogScope/RTULogScope.csproj`

Tento projektový súbor je pre aplikáciu WPF v jazyku C#. Uvádza:

- cieľovú platformu .NET Framework 4.8,
- názov aplikácie a jej ikonu,
- ktoré súbory C# sa majú skompilovať,
- ktoré XAML súbory sa majú preložiť na prvky WPF,
- knižnice Newtonsoft.Json a OxyPlot.

**Knižnica** je hotový balík kódu, ktorý môže projekt používať. **Zostavenie** (build) je premena zdrojových súborov na spustiteľný program.

### `RTULogScope/packages.config`

Zoznam NuGet balíkov s verziami:

- `Newtonsoft.Json` — čítanie JSON,
- `OxyPlot` a WPF doplnky — kreslenie grafov,
- .NET reference assemblies — referenčné súbory pre zostavenie proti .NET 4.8.

**NuGet** je správca knižníc pre .NET. Verzia v tomto zozname určuje, ktorý balík sa má obnoviť pri príprave vývojového prostredia.

### `RTULogScope/App.config`

Rovnako ako jeho náprotivok pre ESP32Logger deklaruje verziu .NET Framework runtime, ktorú desktopová aplikácia očakáva.

### Ikona a snímka obrazovky

- `RTULogScope/RTULogScope.ico` — ikona programu, ktorú používa okno aj projektový súbor.
- `RTULogScope/Assets/program.png` — snímka používateľského rozhrania na ilustráciu v README.

## 4. Generované a projektové vlastnosti

V priečinku `RTULogScope/Properties` sú súbory, ktoré spravujú vlastnosti .NET aplikácie:

- `AssemblyInfo.cs` — názov, verzia, výrobca a ďalšie metadáta programu.
- `Resources.resx` — tabuľka vložených zdrojov, napríklad textov alebo obrázkov. V aktuálnej verzii je prakticky prázdna.
- `Resources.Designer.cs` — pomocná trieda vygenerovaná z `Resources.resx`, aby sa zdroje dali pohodlne čítať z C#.
- `Settings.settings` — deklarácia používateľských nastavení. Momentálne neobsahuje vlastné nastavenia.
- `Settings.Designer.cs` — automaticky vygenerovaná C# trieda pre `Settings.settings`.

**Generovaný súbor** vytvára vývojový nástroj z iného súboru. Ak ho niekto zmení ručne a neskôr sa spustí generátor, ručné zmeny sa môžu stratiť. Preto sa upravuje najmä `.resx` alebo `.settings`, nie ich `.Designer.cs` výstup.

V `AssemblyInfo.cs` sú uvedené konkrétne metadáta projektu vrátane spoločnosti a copyrightu. Tieto údaje sú popisné; samy osebe neurčujú, ako aplikácia meria alebo kreslí.

## 5. Licencie a README jednotlivých častí

V repozitári sú:

- koreňový `LICENSE`,
- `RTULogScope/LICENSE`,
- `RTULogScope/README.md`.

Licencia MIT popisuje podmienky, za ktorých možno projekt používať a šíriť. README v podpriečinkoch vysvetľujú, ako autor zamýšľal používať danú časť. README sú úvod, nie automatická záruka, že každý opísaný formát zodpovedá aktuálnemu zdrojovému kódu.

## 6. Slovník pojmov

| Pojem | Jednoduchý význam |
|---|---|
| API | Dohodnutý spôsob, akým jedna časť programu používa druhú. |
| Arduino `.ino` | Súbor programu, ktorý Arduino prostredie spracuje ako vstup firmvéru. |
| C# / C++ | Programovacie jazyky používané v desktopovej a hardvérovej časti. |
| CSV | Textová tabuľka, kde sú hodnoty zvyčajne oddelené čiarkou. |
| JSON | Textový formát objektov a zoznamov, vhodný na konfiguráciu aj dáta. |
| NDJSON | Viac samostatných JSON objektov, po jednom na každý riadok. |
| RTC | Hodiny reálneho času, ktoré si zachovajú čas aj po vypnutí. |
| RS485 | Elektrické rozhranie používané na sériovú komunikáciu na väčšiu vzdialenosť. |
| UART | Hardvérová sériová linka s vodičom na príjem a vodičom na vysielanie. |
| WPF | Technológia Windows na tvorbu aplikácií s oknami a dátovými väzbami. |
| XAML | Textový zápis vzhľadu a rozloženia prvkov WPF. |
| VTR / CTR | Pomery napäťového a prúdového transformátora použité pri prepočte merania. |
| ViewModel | Trieda, ktorá pripraví dáta a príkazy pre používateľské rozhranie. |
| Register | Číslovaná hodnota dostupná v zariadení cez Modbus. |
| GPIO | Programovateľný vstupný alebo výstupný pin mikrokontroléra. |

## 7. Rýchla kontrola: čo sa stane pri jednom meraní

1. `loop()` zistí, že uplynul interval.
2. `SystemManager` overí SD kartu.
3. `DataLogger` vezme čas, konfiguráciu a hodnoty z Modbus.
4. `ModbusManager` zmení surové čísla na mierkované hodnoty.
5. `StorageManager` pridá JSON objekt ako nový riadok.
6. Používateľ otvorí súbor v RTULogScope.
7. `JsonLogLoader` ho zmení na tabuľku a `GraphViewModel` zobrazí vybrané stĺpce.

Pre obsah a poradie lekcií sa vráť na [00 — Obsah a tok projektu](00-obsah-a-ako-projekt-tecie.md).

