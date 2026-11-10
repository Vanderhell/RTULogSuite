# 03 — Ukladanie, formát dát a zapojenie

Po načítaní registrov treba výsledky bezpečne uložiť, aby sa dali neskôr otvoriť v počítači. Táto lekcia vysvetľuje SD kartu, dátový formát a hardvérové prepojenia.

## 1. `StorageManager`: práca so SD kartou

Súbory:

- `ESP32Logger/src/main/StorageManager.h` — opis funkcií na zápis a kontrolu karty.
- `ESP32Logger/src/main/StorageManager.cpp` — skutočná implementácia.

`begin()` inicializuje SD kartu. **Inicializovať** znamená pripraviť spojenie s komponentom.

`isCardPresent()` zavolá `SD.begin()` znova a vráti `true` (áno), ak karta odpovedá. `SystemManager::runCycle()` túto funkciu kontroluje pred zápisom.

`configure(folder, format, enable, withHeader)` uloží nastavenia z JSON:

- kam ukladať súbory,
- ako vytvárať ich názov podľa dátumu,
- či je logovanie zapnuté,
- či sa má pripraviť hlavička CSV.

`getTodayLogFilename()` vezme systémový čas, použije formát kompatibilný so `strftime` a spojí priečinok s výsledným názvom. Napríklad formát `data_%Y%m%d.json` pri dátume 10. 11. 2026 vytvorí `data_20261110.json`.

## 2. Čo znamená NDJSON

Metóda `writeJSON()` vytvorí jeden JSON objekt:

```json
{
  "timestamp": "2026-11-10 14:05:30",
  "values": [
    { "key": "voltage_l1-n", "value": 231.5, "unit": "V" }
  ]
}
```

Na konci zapíše znak nového riadka. Ďalšie meranie pridá ako ďalší samostatný JSON objekt pod neho:

```text
{"timestamp":"2026-11-10 14:05:30","values":[...]}
{"timestamp":"2026-11-10 14:05:40","values":[...]}
```

Tento formát sa volá **NDJSON** (newline-delimited JSON), teda „JSON oddelený novými riadkami“. Je praktický pri postupnom pridávaní záznamov. Každý riadok sa dá spracovať samostatne.

`writeJSON()` tiež overí, že počet hodnôt zodpovedá počtu registrov. Ak sa nezhoduje, zápis preskočí a uloží chybu.

`logError(message)` pridá text chyby s časom do `error.log`. Pomocné logy pomáhajú po zlyhaní zistiť, čo sa dialo.

## 3. Konfiguračné súbory na karte

V repozitári sú dve podobné ukážky:

- `ESP32Logger/config/config.json`
- `ESP32Logger/SD Card/config/config.json`

Kód otvára `/config/config.json`, čiže priečinok `config` na karte. Počas nasadenia musí byť správny súbor nakopírovaný na SD kartu do tejto cesty.

V ukážkovej konfigurácii sú nastavené:

- komunikačné parametre merača EM-07,
- adresy napätia, prúdu, výkonu, frekvencie a energie,
- vzorce mierky,
- interval `10000` ms,
- priečinok `/logs/`,
- názvový formát `%Y-%m-%d.csv`.

## 4. Dôležitý rozpor: CSV názov, JSON obsah

Konfigurácia nastavuje príponu `.csv`, ale `StorageManager::writeJSON()` zapisuje JSON objekty. Súbor sa teda môže volať napríklad `2026-11-10.csv`, ale jeho obsah je NDJSON, nie CSV tabuľka.

Ďalšie súvisiace pozorovania:

- `include_header` sa uloží, ale v metóde `writeJSON()` sa nepoužíva.
- `StorageManager` v kóde nevytvára priečinok `/logs/`. Ak na karte neexistuje, otvorenie cieľového súboru môže zlyhať.
- Vytváranie mena súboru používa systémový čas ESP32; priame manuálne nastavenie RTC nemusí v rovnakej chvíli zmeniť systémový čas.

Zatiaľ ide iba o vysvetlenie zisteného správania.

## 5. Hardvérové zapojenie

Príručka je v `ESP32Logger/hardware_wiring_schematic.md`. Obrázky zapojenia a zostavy sú v `ESP32Logger/Assets/`.

Zmysel hlavných spojení:

- **SD karta** ukladá súbory. SPI je komunikačná zbernica s vodičmi hodinového signálu a dát.
- **DS3231 RTC** poskytuje dátum a čas. I²C je dvojvodičová zbernica s vodičmi SDA a SCL.
- **RS485 modul** prenáša Modbus RTU medzi meračom a ESP32.
- **GND** je spoločná zem. Moduly ju zvyčajne musia mať spojenú, aby sa správne chápali ich elektrické úrovne.

V kóde `ModbusManager.cpp` sú RX GPIO17, TX GPIO16 a DE/RE GPIO4. V `hardware_wiring_schematic.md` sú RX/TX prehodené a navyše sa na inom mieste odkazuje na iné hodnoty. `StorageManager.cpp` volá `SD.begin()` bez explicitného CS pinu, hoci príručka uvádza GPIO5.

**GPIO** je programovateľný pin na doske. **CS** vyberá SD zariadenie na SPI zbernici. Pred zapojením treba tieto rozdiely skontrolovať proti skutočnej doske a zapojeniu.

## 6. Ako sa všetko spojí

```text
DataLogger.logAll()
  ├─ rtc.getFormattedTime()          → textový čas
  ├─ config.getRegisters()           → zoznam meraní
  ├─ modbus.readAll(registers)        → zoznam čísel
  └─ storage.writeJSON(time, values)  → riadok v súbore
```

Príklad: ak sú v konfigurácii tri registre, očakávame tri číselné výsledky. Kľúč, hodnota a jednotka sa potom uložia ako trojica do poľa `values`.

