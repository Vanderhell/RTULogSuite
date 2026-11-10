# 02 — Konfigurácia, čas a Modbus meranie

V tejto lekcii sledujeme tri veci, ktoré dávajú meraniu význam: nastavenia v JSON, aktuálny čas a rozhovor medzi ESP32 a meračom.

## 1. JSON konfigurácia — nastaviteľný „recept“

JSON je textový formát na pomenované dáta. Používa zložené zátvorky pre objekt, hranaté zátvorky pre zoznam a dvojbodku medzi názvom a hodnotou.

Aktívny súbor, ktorý kód otvára, je:

```text
/config/config.json
```

To je cesta na SD karte. V repozitári je ukážková konfigurácia v `ESP32Logger/config/config.json`. Kód si ju pri štarte sám neskopíruje na kartu.

### `ConfigManager.h`: aké údaje si program pamätá

Tento súbor deklaruje dve dátové štruktúry:

- `ModbusSettings` — Slave ID, baudrate, parita, stop bity a počet dátových bitov.
- `ConfigManager` — interval merania, register mapu, pomery VTR/CTR, režim adresovania a príznak ladenia.

**Slave ID** je číslo zariadenia, s ktorým sa ESP32 rozpráva. **Baudrate** je rýchlosť sériovej linky. **Parita** je jednoduchý kontrolný bit, ktorý môže pomôcť odhaliť chybu prenosu.

Funkcie `get...` vracajú uložené nastavenia. `setStorage()` pripojí objekt úložiska, aby konfigurácia mohla zapísať chybu pri čítaní súboru.

### `ConfigManager.cpp`: načítanie a prevod nastavení

Metóda `load()`:

1. otvorí JSON súbor na SD karte,
2. prečíta ho do `DynamicJsonDocument`,
3. zavrie súbor,
4. vyberie pomenované hodnoty,
5. pre každý prvok v poli `registers` vytvorí `RegisterConfig`,
6. nastaví úložisko podľa časti `logging`.

Zápis `doc["logging"]["interval_ms"] | 1000` znamená: „skús načítať interval; ak chýba alebo sa nedá získať, použi 1000“. Toto `|` je tu operátor ArduinoJson na predvolenú hodnotu.

Ak súbor chýba alebo JSON nie je platný, metóda vypíše chybu a podľa možnosti ju uloží do `error.log`.

### Príklad konfigurácie

```json
{
  "communication": {
    "modbus_id": 1,
    "baudrate": 9600,
    "parity": "N",
    "addressing_mode": "1-based"
  },
  "logging": {
    "interval_ms": 10000
  },
  "registers": [
    {
      "key": "voltage_l1-n",
      "register": 4002,
      "type": "UINT16",
      "unit": "V",
      "scaling": "val * 0.1 * VTR",
      "length": 1
    }
  ]
}
```

Program teda čaká 10 000 ms, čiže 10 sekúnd, medzi meraniami. Register 4002 predstavuje prvú fázu napätia a jeho výsledok sa prepočíta vzorcom.

## 2. Adresy registrov a mierka

Niektoré príručky meračov číslujú registre od 1, iné od 0. Táto odlišnosť sa nazýva **offset** — posun v číslovaní.

Ak je `addressing_mode` nastavený na `1-based`, kód pri čítaní odpočíta 1:

```text
adresa v JSON: 4002
adresa odoslaná po Modbus: 4001
```

Treba si overiť, aké adresovanie používa konkrétny merač. Nesprávny posun môže čítať susedný register alebo dostať chybu.

Surová hodnota registra sa upravuje výrazom `scaling`. Pri `val = 2315` a `VTR = 1`:

```text
val * 0.1 * VTR
2315 * 0.1 * 1
= 231.5 V
```

### `ModbusManager.h` a `ModbusManager.cpp`

**Modbus RTU** prenáša binárne správy po sériovej linke. **RS485** je elektrické rozhranie, ktoré umožňuje komunikáciu po dlhšom kábli a medzi viacerými zariadeniami.

V `ModbusManager.cpp` sú piny UART1 pevne nastavené:

- RX (príjem ESP32): GPIO17
- TX (vysielanie ESP32): GPIO16
- DE/RE (prepínanie smeru RS485): GPIO4

`begin()` spustí sériovú linku, nastaví pin DE/RE ako výstup a pripojí Modbus knižnicu k Slave ID. Funkcie `preTransmission()` a `postTransmission()` prepnú RS485 modul medzi vysielaním a príjmom.

### Čítanie registrov

`readAll(regs)` prejde zoznam registrov jeden po druhom:

1. vypočíta adresu s prípadným posunom,
2. zavolá `readHoldingRegisters()`,
3. pri úspechu vezme surové číslo z odpovedného bufferu,
4. vypočíta `scaling`,
5. vloží výsledok do zoznamu,
6. pri chybe vloží `NAN` (označenie „nie je to platné číslo“).

`readRegister()` číta jeden 16-bitový register. Používa sa pri čítaní registrov transformátora.

`scanRange()` je diagnostická funkcia. Skúsi čítať sériu adries a vypíše, ktoré reagujú. Je pomalšia a pri zapnutom `debug` môže počas štartu vykonať mnoho požiadaviek.

### Ako sa vyhodnocuje textový vzorec

Funkcia `evaluateExpression()` nahradí slová `val`, `VTR` a `CTR` číslami, odstráni medzery a počíta operácie `+`, `-`, `*`, `/` a zátvorky.

**Zásobník** je ako stoh tanierov: posledná vložená hodnota sa vyberie ako prvá. Kód používa jeden zásobník pre čísla a druhý pre operátory. Pomocná funkcia `precedence()` určuje, že násobenie sa má vykonať pred sčítaním.

Podporovaný príklad:

```text
(val * 0.1) * VTR
```

Nie je to plnohodnotná kalkulačka. Záporné čísla so znamienkom na začiatku, exponenty ako `1e-3` ani ľubovoľné názvy funkcií sa bezpečne nepodporujú.

## 3. Čas — `RtcManager.h` a `.cpp`

**RTC** znamená hodiny reálneho času. DS3231 je malý samostatný modul s batériou, ktorý si čas drží aj po vypnutí ESP32.

- `begin()` otvorí RTC, skontroluje jeho stav a nastaví systémové hodiny ESP32 podľa času z modulu.
- `getFormattedTime()` vráti text vo formáte `YYYY-MM-DD HH:MM:SS`.
- `setTime(...)` nastaví DS3231 na zadaný dátum a čas.
- `serialSetupTime()` čaká až 10 sekúnd na čas zo Serial Monitora.

Príklad vstupu:

```text
setrtc 2026-11-10 14:05:30
```

Ak používateľ nič nenapíše alebo zadá nesprávny formát, funkcia skúsi nastaviť RTC podľa systémového času ESP32.

## 4. Dôležité rozdiely a limity v aktuálnom kóde

Tieto zistenia sú dôležité pre správne pochopenie projektu:

1. `ModbusSettings` načíta paritu, stop bity aj počet dátových bitov, ale `ModbusManager::begin()` volá `Serial1.begin(..., SERIAL_8N1, ...)`. Tým sa natvrdo nastaví 8 dátových bitov, bez parity a 1 stop bit. Ostatné načítané hodnoty sa teda momentálne nepoužijú.
2. `readAll()` požaduje `reg.length` registrov, ale následne číta iba `getResponseBuffer(0)`. Pri dĺžke 2 (napríklad `UINT32`) sa druhá polovica nezhromažďuje do 32-bitového čísla.
3. Pri neúspešnom načítaní konfigurácie program vypíše chybu a pokračuje ďalej s predvolenými alebo prázdnymi hodnotami. Treba preto sledovať výstup Serial Monitora.
4. `setTime()` nastaví RTC modul. Kód v tejto funkcii zároveň nenastaví systémové hodiny ESP32; tie sa synchronizujú v `begin()` pri štarte.
5. Komentár v `ModbusManager.h` hovorí o zapnutí korekcie „+1“, no implementácia od adresy odpočítava 1. Rozhodujúce je aktuálne správanie v `.cpp`.

Nič z toho nemeníme; zaznamenávame presne to, čo kód robí, aby sa začiatočník nepomýlil medzi zámerom a skutočnou implementáciou.

## 5. Skús si to prepočítať

V konfigurácii nastav:

```json
{ "scaling": "val * 0.01 * CTR" }
```

Ak merač vráti `val = 250` a `CTR = 20`, výsledok je:

```text
250 × 0.01 × 20 = 50 A
```

Takto sa dá ručne skontrolovať, či očakávaný výstup zodpovedá použitému vzorcu.

