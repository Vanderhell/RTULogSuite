# 01 — Základ a štart firmvéru

Táto lekcia sleduje program od zapnutia ESP32 po pravidelné spustenie merania. Hlavné súbory sú:

- `ESP32Logger/src/main/main.ino` — vstupný bod programu Arduino.
- `ESP32Logger/src/main/SystemManager.h` a `.cpp` — koordinátor zariadenia.
- `ESP32Logger/src/main/DataLogger.h` a `.cpp` — vykoná jeden cyklus merania a uloženia.
- `ESP32Logger/src/main/RegisterConfig.h` — opis jedného meraného registra.

## 1. Čo znamená „vstupný bod“

**Vstupný bod** je miesto, odkiaľ platforma začne spúšťať náš program. V programe Arduino sú ním dve funkcie:

- `setup()` sa vykoná raz po štarte.
- `loop()` sa opakuje stále dookola.

Predstav si ESP32 ako kuchára: `setup()` pripraví kuchyňu a načíta recept; `loop()` potom v pravidelných intervaloch pripravuje ďalšiu porciu údajov.

## 2. `main.ino`: spustenie a opakovanie

### Hlavičky a objekty

Súbor začína riadkami `#include`. **Hlavička** (súbor `.h`) oznamuje, aké triedy a funkcie môže daný súbor používať.

```cpp
#include "SystemManager.h"
#include "ConfigManager.h"
#include "DataLogger.h"

SystemManager systemManager;
unsigned long lastPollTime = 0;
unsigned long pollInterval = 1000;
```

- `SystemManager` je názov triedy, teda vlastného typu programu.
- `systemManager` je konkrétny objekt tejto triedy.
- `lastPollTime` si pamätá čas posledného cyklu.
- `pollInterval` hovorí, koľko milisekúnd medzi cyklami čakáme.
- `unsigned long` je celočíselný typ pre nezáporné, väčšie počty. Funkcia Arduino `millis()` vracia milisekundy od zapnutia.

### Funkcia `setup()`

`Serial.begin(115200)` zapne sériovú komunikáciu s počítačom, napríklad cez Serial Monitor. Číslo `115200` je jej prenosová rýchlosť. `delay(1000)` počká sekundu, aby sa monitor stihol pripojiť.

Následne sa zavolá `systemManager.setupAll()`. Je to jedno miesto, ktoré pripraví hodiny, SD kartu, konfiguráciu a Modbus. Na konci sa z konfigurácie načíta `pollInterval`.

Ak je povolený režim ladenia (`debug`), program vypíše na sériovú linku nastavenia, zoznam registrov a čas. **Ladenie** znamená pridávanie pomocných informácií, ktoré uľahčujú hľadanie problému.

### Funkcia `loop()`

```cpp
unsigned long now = millis();
checkSerialCommands();

if (now - lastPollTime >= pollInterval) {
    systemManager.runCycle();
    lastPollTime = now;
}
delay(10);
```

Program zistí aktuálny čas. Ak od posledného merania uplynul nastavený interval, spustí `runCycle()` a uloží si nový čas.

Porovnanie `now - lastPollTime` je bežný spôsob merania intervalu na mikrokontroléri. Nezávisí od presného okamihu zapnutia. `delay(10)` na chvíľu uvoľní procesor.

### Funkcia `checkSerialCommands()`

Táto pomocná funkcia zbiera znaky zo sériovej linky do textu `input`. Keď príde koniec riadka, odstráni okolité medzery a skontroluje, či text nie je `setrtc`. Ak je, požiada hodinový modul o nastavenie času cez `setupRTCFromSerial()`.

**Príkaz** je text, ktorý používateľ napíše programu. Premenná `static String input` si zachová obsah medzi opakovaniami funkcie, aby sa príkaz dal skladať po znakoch.

## 3. `SystemManager`: dirigent zariadenia

**Manažér** v názve triedy znamená objekt, ktorý združuje prácu viacerých častí. `SystemManager` sám nečíta dátový register ani nepíše JSON. Rozhoduje, kedy a v akom poradí to majú urobiť špecializované objekty.

### `SystemManager.h`: verejné a súkromné časti

V hlavičke sa deklaruje, čo trieda ponúka:

- `setupAll()` — jednorazová príprava pri štarte.
- `getPollingInterval()` — získa interval merania.
- `runCycle()` — vykoná jeden cyklus.
- `getConfig()`, `getRtc()` — sprístupnia konfiguráciu a hodiny, najmä na ladenie.
- `setupRTCFromSerial()` — spustí nastavenie hodín.

Slovo `public` znamená, že tieto funkcie môže zavolať aj iný súbor. Pod `private` sú vnútorné objekty: `rtc`, `config`, `storage`, `modbus`, `logger`. Iný súbor ich nemôže priamo meniť.

### `SystemManager.cpp`: čo robí `setupAll()`

`setupAll()` pripravuje zariadenie v poradí:

1. Spustí RTC a skontroluje, či čas nevyzerá ako predvolený rok 1970 alebo 2000.
2. Inicializuje SD kartu.
3. Pripojí úložisko ku konfigurácii a zavolá `config.load()`.
4. Z nastavení pripraví Modbus komunikáciu.
5. Načíta pomery transformátorov z konfigurácie alebo ich skúsi prečítať z merača.
6. Vytvorí `DataLogger` a odovzdá mu odkazy na potrebné objekty.

**Inicializovať** znamená pripraviť vec na používanie, napríklad otvoriť komunikáciu s hardvérom. **Odkaz** alebo pointer je adresa iného objektu v pamäti; v C++ sa píše napríklad `DataLogger*`. Vďaka tomu `DataLogger` používa už vytvorené hodiny a úložisko, namiesto toho, aby vytváral ich druhú kópiu.

### Transformátorové pomery

`VTR` a `CTR` predstavujú napäťový a prúdový pomer transformátora. Surová hodnota z merača môže byť len časťou výsledného fyzikálneho merania. `SystemManager` najprv pripraví predvolenú hodnotu z JSON a potom sa pokúsi prečítať hodnotu z registrov `VTR_register` a `CTR_register`. Pri chybe použije predvolenú hodnotu.

### `runCycle()`

Každý cyklus skontroluje SD kartu. Ak je dostupná, zavolá `logger.logAll()`; inak zapíše chybu. Toto je jednoduché rozhodnutie typu „ak — inak“: meranie má zmysel uložiť iba vtedy, ak úložisko odpovedá.

## 4. `DataLogger`: jeden kompletný záznam

Trieda `DataLogger` spája štyri úlohy:

1. Získa čas z RTC.
2. Z konfigurácie získa zoznam registrov.
3. Požiada Modbus o hodnoty.
4. Odovzdá výsledky úložisku.

Jej metóda `logAll()` je jedným kompletným krokom. Ak počet hodnôt nesedí s počtom registrov alebo sa nevráti žiadna hodnota, zaznamená chybu.

**Validácia** je kontrola, či dáta spĺňajú očakávania. Podmienka `values.size() == registers.size()` overuje, že ku každému opisu merania existuje zodpovedajúca hodnota.

V kóde sa používa aj `std::vector`, čo je zoznam, ktorého veľkosť sa môže meniť. Napríklad konfigurácia môže mať 10 registrov a vector potom obsahuje 10 výsledkov.

## 5. `RegisterConfig.h`: kartička jedného merania

`struct RegisterConfig` je skupina pomenovaných polí, ktoré spolu opisujú jeden register. Príklad kartičky:

```text
key: voltage_l1-n
name: Voltage L1-N
register_address: 4002
type: UINT16
unit: V
scaling: val * 0.1 * VTR
length: 1
```

- `key` je stabilný strojový názov, používa sa ako kľúč v logu.
- `name` je čitateľný názov.
- `description` je doplnkový opis.
- `register_address` je adresa v merači.
- `type` hovorí, aký druh čísla merač ukladá.
- `unit` je jednotka, napríklad volt.
- `scaling` je vzorec na prepočet surovej hodnoty.
- `access` opisuje, či sa dá register čítať alebo aj zapisovať.
- `length` je počet 16-bitových registrov, ktoré tvoria hodnotu.

`UINT16` znamená nezáporné 16-bitové celé číslo; `UINT32` nezáporné 32-bitové číslo. Pri 32-bitovom čísle sa zvyčajne spoja dva 16-bitové registre.

## 6. Celý štart v skratke

```text
ESP32 sa zapne
  → Arduino zavolá setup()
  → SystemManager.setupAll()
  → konfigurácia načíta register mapu z SD
  → Modbus a RTC sú pripravené
  → Arduino opakuje loop()
  → po intervale SystemManager.runCycle()
  → DataLogger.logAll()
```

V ďalšej lekcii prejdeme, ako sa JSON konfigurácia zmení na Modbus nastavenia a ako sa surové číslo premení na meranie.

## Dodatočné upresnenie: zlyhané Modbus čítanie

`ModbusManager::readAll()` pri chybe čítania vloží do výsledného zoznamu `NAN`, no stále vloží jednu položku pre každý nakonfigurovaný register. Preto sa počet výsledkov často rovná počtu registrov aj vtedy, keď niektoré čítanie zlyhalo.

`DataLogger::logAll()` kontroluje iba to, či zoznam nie je prázdny a či majú zoznamy rovnakú veľkosť. Samotnú hodnotu `NAN` nekontroluje. Taký cyklus teda môže označiť ako úspešný a poslať ho na zápis. Pri serializácii sa neplatné číslo môže zobraziť ako JSON `null`.

To je rozdiel medzi „máme jednu výslednú položku pre každý register“ a „každé meranie bolo úspešne prečítané“. Aktuálny kód spoľahlivo overuje len prvú podmienku.

