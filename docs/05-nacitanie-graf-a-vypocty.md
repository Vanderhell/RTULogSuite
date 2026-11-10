# 05 — Načítanie dát, graf a výpočty

Táto lekcia sleduje súbor od otvorenia až po zobrazenie krivky. Hlavné súbory:

- `RTULogScope/Services/JsonLogLoader.cs`
- `RTULogScope/Services/CsvLoader.cs`
- `RTULogScope/ViewModels/GraphViewModel.cs`
- `RTULogScope/Models/SelectableItem.cs`
- `RTULogScope/Models/MeasurementData.cs`
- `RTULogScope/ViewModels/Converters/BoolToVisibilityConverter.cs`
- `RTULogScope/ViewModels/Converters/ContainsConverter.cs`
- `RTULogScope/ViewModels/Converters/EyeIconConverter.cs`

## 1. Načítavanie NDJSON: `JsonLogLoader.cs`

Trieda `JsonLogLoader` načíta všetky riadky súboru. Predpokladá, že každý riadok je samostatný JSON objekt s poľami `timestamp` a `values`.

Na prvom riadku zistí názvy veličín a vytvorí stĺpce tabuľky:

```text
timestamp | voltage_l1-n | frequency
```

Potom každý ďalší JSON riadok zmení na jeden riadok tabuľky. Hodnoty uloží do stĺpca podľa poľa `key`.

**Tabuľka** (`DataTable`) je dátová štruktúra s riadkami a stĺpcami, podobná malej tabuľke v Exceli. **Stĺpec** obsahuje jeden druh údaja; **riadok** obsahuje údaje z jedného meracieho času.

Ak je hodnota v JSON `null` alebo chýba, loader uloží `0.0`. To zabráni chybe pri čítaní čísla, ale nula sa potom nedá odlíšiť od skutočne nameranej nuly.

Ak súbor neobsahuje platný JSON alebo má inú štruktúru, zobrazí chybové okno a vráti tabuľku, ktorá môže byť prázdna.

## 2. Načítavanie CSV: `CsvLoader.cs`

Tento súbor číta klasické CSV:

```csv
timestamp,voltage,frequency
2026-11-10 14:05:30,231.5,50.0
```

Prvý riadok používa ako názvy stĺpcov. Každý ďalší riadok rozdelí podľa čiarky. Ak má riadok iný počet polí než hlavička, preskočí ho.

`CsvLoader.Load(path)` vracia `DataTable`. Chybu pri čítaní zobrazí cez WPF dialóg.

### Dôležité: tento loader nie je zapojený do aktuálneho načítania

V `GraphViewModel.cs` metódy `LoadCsv()` aj `LoadMultipleCsvs()` volajú `JsonLogLoader.Load(...)`. Tlačidlo v okne teda podľa aktuálneho kódu načítava NDJSON, nie tradičné CSV.

Jednoduchý `CsvLoader` navyše nie je plnohodnotný CSV parser: delenie cez `Split(',')` nespracuje čiarku vo vnútri úvodzoviek ani úvodzovkami obalené polia. Toto je ďalší dôvod, prečo názov „CSV“ v používateľskom rozhraní nezaručuje kompatibilitu s ľubovoľným CSV súborom.

## 3. `GraphViewModel.cs`: dáta pre graf

Tento súbor tvorí stred desktopovej aplikácie. Drží:

- `PlotModel` — model grafu pre OxyPlot,
- `StatsSummary` — text so štatistikami,
- `Measurements` — zoznam položiek, ktoré možno zaškrtnúť,
- `data` — tabuľku načítaných dát.

### `LoadCsv(path)`

Napriek názvu použije `JsonLogLoader`. Vymaže starý zoznam meraní, skontroluje stĺpec `timestamp` a pre každý iný stĺpec vytvorí zaškrtávaciu položku.

Pri zmene checkboxu sa zavolá `UpdatePlot()`. Je to spôsob, ako používateľovi okamžite ukázať nový výber.

### `LoadMultipleCsvs(paths)`

Táto metóda prejde viac ciest:

1. každú načíta cez `JsonLogLoader`,
2. skopíruje riadky do spoločnej tabuľky,
3. ponechá riadky s rozpoznateľným časom,
4. zoradí ich podľa timestampu,
5. vytvorí checkboxy a aktualizuje graf.

Pri spájaní sa názvy a štruktúra stĺpcov preberajú z prvého súboru. Súbory s inými stĺpcami alebo rozličnými názvami veličín nemusia vytvoriť očakávanú tabuľku.

### `UpdatePlot())

Metóda zistí, ktoré veličiny používateľ vybral. Potom vymaže staré krivky a osi, vytvorí:

- časovú vodorovnú os (`DateTimeAxis`),
- číselnú zvislú os (`LinearAxis`).

Pre každú vybranú veličinu prejde riadky tabuľky. Ak sa čas dá prečítať ako dátum a hodnota ako číslo, pridá bod do krivky.

**Krivka** (`LineSeries`) je rad bodov, ktoré graf spojí čiarou. **Os** určuje, čo znamená smer v grafe; tu je vodorovne čas a zvisle hodnota.

Číslo sa číta s kultúrou `InvariantCulture`, teda s jednotnými pravidlami bez ohľadu na jazyk Windows. Pred tým sa desatinná čiarka nahradí bodkou.

Nakoniec sa pre vybrané stĺpce vypočíta:

- `Min` — najmenšia hodnota,
- `Max` — najväčšia hodnota,
- `Avg` — aritmetický priemer, čiže súčet delený počtom hodnôt.

`PlotModel.InvalidatePlot(true)` oznámi OxyPlotu, že model sa zmenil a graf sa má znova vykresliť.

## 4. Ako checkbox oznámi zmenu: `SelectableItem.cs`

`SelectableItem<T>` je **generická trieda**: písmeno `T` predstavuje typ údaja, ktorý položka drží. Tu je `T` text, napríklad `"voltage_l1-n"`.

Každá položka má:

- `Value` — názov veličiny,
- `IsSelected` — či je checkbox zaškrtnutý,
- `PropertyChanged` — upozornenie pre WPF, že vlastnosť zmenila hodnotu,
- `SelectionChanged` — upozornenie pre ViewModel, že treba obnoviť graf.

Keď používateľ zaškrtne políčko, setter `IsSelected` vyvolá udalosti. WPF aktualizuje obrazovku a pripojená lambda vo ViewModele zavolá `UpdatePlot()`.

## 5. `MeasurementData.cs`: dátová trieda

Trieda opisuje jedno meranie pomocou troch vlastností: `Timestamp`, `Name`, `Value`. Je to jednoduchý **model** — skupina údajov pomenovaná jedným typom.

V aktuálnom grafe sa však táto trieda nepoužíva: graf pracuje priamo s tabuľkou `DataTable`. Súbor teda momentálne predstavuje alternatívny alebo pripravený dátový model.

## 6. Prevodníky pre WPF

**Prevodník** (converter) zmení hodnotu jedného typu na hodnotu, ktorú očakáva prvok obrazovky.

- `BoolToVisibilityConverter.cs`: `true` zmení na `Visible`, `false` na `Collapsed`. Používa sa na panel štatistík.
- `EyeIconConverter.cs`: `true` zmení na ikonu oka, `false` na ikonu skrytého oka. Používa sa na prepínači panelu.
- `ContainsConverter.cs`: skontroluje, či zoznam obsahuje konkrétnu položku. V aktuálnom `MainWindow.xaml` nie je použitý; vyzerá ako pomocník pripravený pre inú väzbu.

`ConvertBack()` je opačný smer prevodu, z obrazovky naspäť do dát. Pri ikonke sa nepoužíva, preto vráti pokyn `Binding.DoNothing`.

## 7. Jednoduchý priechod od súboru po obrazovku

```text
klik Load CSV
  → MainWindow.LoadCsv_Click()
  → GraphViewModel.LoadMultipleCsvs()
  → JsonLogLoader.Load() pre každý súbor
  → DataTable a zoznam názvov veličín
  → checkbox vyberie stĺpec
  → UpdatePlot() vytvorí body a štatistiky
  → OxyPlot prekreslí graf
```

## 8. Praktický príklad

Ak JSON riadok obsahuje:

```json
{"timestamp":"2026-11-10 14:05:30","values":[{"key":"voltage","value":231.5,"unit":"V"}]}
```

loader vytvorí tabuľku s hodnotou `231.5` v stĺpci `voltage`. Zaškrtnutie `voltage` spôsobí, že sa v grafe objaví bod v čase 14:05:30 s výškou 231.5.

Aktuálne sa jednotka `unit` pri načítaní do tabuľky nepoužije, takže os grafu ukazuje iba všeobecné „Value“.

