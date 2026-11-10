# RTULogSuite po lopatkách

> **Sprievodca pre začínajúceho programátora**  
> Vysvetľuje, ako je projekt poskladaný, kadiaľ tečú dáta a čo robia jeho súbory. Výklad vychádza z aktuálneho obsahu repozitára.

## Čo je RTULogSuite

Projekt má dve samostatné časti, ktoré spolu spracúvajú namerané hodnoty:

1. **ESP32Logger** je firmvér pre dosku ESP32. Číta registre merača cez Modbus RTU a RS485, pridá čas a ukladá záznamy na SD kartu.
2. **RTULogScope** je aplikácia pre Windows. Otvorí uložený súbor, zobrazí vybrané veličiny v grafe a vypočíta minimum, maximum a priemer.

**Firmvér** je program určený priamo pre zariadenie. **Register** je číslovaná položka v merači, z ktorej možno prečítať hodnotu, napríklad napätie. **Protokol** je dohodnutý spôsob, akým si zariadenia posielajú správy.

## Ako putuje jedno meranie

```text
merač
  │ Modbus RTU cez RS485
  ▼
ESP32 ── načíta register a prepočíta surovú hodnotu
  │
  ├── čas z RTC (hodinového modulu DS3231)
  └── záznam na SD kartu
             │
             ▼
     súbor s riadkami JSON
             │ otvorenie súboru
             ▼
       RTULogScope ── tabuľka → výber veličín → graf a štatistika
```

**Surová hodnota** je číslo presne tak, ako ho poslal merač. Často treba použiť mierku: ak register obsahuje `2315` a mierka je `val * 0.1`, výsledok je `231.5 V`.

## Kde začať čítať kód

Ak ešte len spoznávaš projekt, prejdi si dokumenty v tomto poradí:

1. [01 — Základ a štart firmvéru](01-zaklady-a-start-firmveru.md)
2. [02 — Konfigurácia, čas a meranie](02-konfiguracia-cas-a-meranie.md)
3. [03 — Ukladanie, formát dát a zapojenie](03-ukladanie-format-a-hardver.md)
4. [04 — Desktopová aplikácia a jej okno](04-desktopova-aplikacia.md)
5. [05 — Načítanie dát, graf a výpočty](05-nacitanie-graf-a-vypocty.md)
6. [06 — Ostatné súbory a slovník pojmov](06-ostatne-subory-a-slovnik.md)

Každá lekcia uvádza cesty k súborom tak, ako sa nachádzajú v repozitári. Najskôr si všimni názov a úlohu súboru, až potom čítaj jeho funkcie.

## Dôležité rozdiely medzi opisom a skutočným kódom

Dokumentácia má začínajúcemu programátorovi ukázať aj miesta, kde sa opis a implementácia nezhodujú:

- Názov a README hovoria o CSV, ale ESP32 ukladá **NDJSON** — jeden JSON objekt na riadok. Ukážkový názov súboru v konfigurácii má príponu `.csv`, hoci jeho obsah je JSON.
- Desktopová aplikácia má jednoduchý `CsvLoader`, no grafický model momentálne načítava pomocou `JsonLogLoader`. Pri použití skutočného CSV súboru preto môže zobraziť chybu.
- README odkazuje na `config_json_documentation.md`, no taký súbor sa v repozitári nenachádza.
- Zapojovacia príručka a kód neuvádzajú rovnaké piny RS485. Kým sa nezjednotia, zapojenie treba overiť podľa skutočného zariadenia.
- V `ModbusManager.cpp` sa pri čítaní typu `UINT32` vyžiadajú dva 16-bitové registre, ale kód používa iba prvú 16-bitovú časť. To môže skrátiť alebo skresliť väčšie hodnoty.
- `ESP32Logger.csproj` vyzerá ako projekt .NET Framework, zatiaľ čo hlavný firmvér je súbor Arduino `.ino`. Tento projektový súbor sám osebe nevysvetľuje ani nezostavuje firmvér ESP32.

Tieto body sú zaznamenané ako vysvetlenie aktuálneho stavu. Zatiaľ nimi nemeníme správanie programu.

## Ako používať príklady

V príkladoch používame vymyslené hodnoty, aby bolo vidieť princíp:

```text
register merača: 4002
surová hodnota: 2315
výraz mierky: val * 0.1 * VTR
VTR: 1
výsledok: 231.5 V
```

`VTR` je tu pomer napäťového transformátora. Ak by bol `VTR = 10`, výsledok výrazu by bol `2315 V`. Správny pomer a mierka závisia od merača a zapojenia.

## Rozsah tejto série

Súbory s programovým kódom vysvetľujeme po skupinách podľa ich úlohy. Pri každom dôležitom názve vysvetľujeme, čo znamená, a pri funkciách ukazujeme, kto ich zavolá, čo dostanú a čo vrátia alebo zmenia. Generované súbory, obrázky, licencie a nastavenia majú samostatný stručný opis v poslednej lekcii.

