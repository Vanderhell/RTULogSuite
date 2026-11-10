# 04 — Desktopová aplikácia a jej okno

`RTULogScope` je aplikácia pre Windows napísaná v C# a WPF. Jej úlohou je zobraziť uložené merania ako časový graf.

**WPF** je technológia Microsoftu na tvorbu okien. Rozloženie prvkov sa píše v XAML a ich správanie v C#. **XAML** je značkovací jazyk: zapisuje prvky a ich vlastnosti podobne ako recept alebo plán miestnosti.

## 1. Súbory, ktoré vytvoria aplikáciu

- `RTULogScope/App.xaml` — spúšťacie okno a spoločné prevodníky.
- `RTULogScope/App.xaml.cs` — C# trieda celej aplikácie; tu je takmer prázdna, pretože aplikácia nepotrebuje vlastnú globálnu logiku.
- `RTULogScope/Views/MainWindow.xaml` — vzhľad hlavného okna.
- `RTULogScope/Views/MainWindow.xaml.cs` — reakcia na kliknutie na tlačidlá.
- `RTULogScope/ViewModels/GraphViewModel.cs` — dáta, výber meraní, graf a štatistiky.

## 2. `App.xaml`: spoločné zdroje

V `StartupUri` je uvedené, že po spustení sa má otvoriť `Views/MainWindow.xaml`.

V `Application.Resources` sú pomenované **zdroje**, teda objekty pripravené na opakované použitie:

- `BoolToVisibilityConverter` zmení hodnotu áno/nie na viditeľné/skryté.
- `EyeIconConverter` zmení hodnotu áno/nie na ikonu oka alebo skrytého oka.

Ich kľúče `BoolToVisibilityConverter` a `EyeIconConverter` fungujú ako prezývky, cez ktoré ich XAML nájde.

## 3. `MainWindow.xaml`: okno poskladané z prvkov

Najvyšší prvok je `Window`. Má názov, rozmery, ikonu a štartuje vycentrovaný na obrazovke.

```xml
<Window.DataContext>
    <vm:GraphViewModel/>
</Window.DataContext>
```

**DataContext** je objekt, z ktorého daná časť XAML číta údaje. Tu je to `GraphViewModel`. Vďaka tomu môže text `{Binding StatsSummary}` zobraziť hodnotu vlastnosti `StatsSummary` bez ručného nastavovania textu v každom riadku C#.

**Binding** (väzba) prepája vlastnosť v kóde s prvkom obrazovky:

- `ItemsSource="{Binding Measurements}"` — zoznam checkboxov pochádza z kolekcie `Measurements`.
- `IsChecked="{Binding IsSelected, Mode=TwoWay}"` — zaškrtnutie sa zapisuje naspäť do objektu.
- `Model="{Binding PlotModel}"` — graf používa model z ViewModelu.

### Rozloženie

- `DockPanel` umiestni horný panel s tlačidlami nad hlavnú časť.
- `Grid` rozdelí hlavné okno na ľavý výber meraní a pravý graf.
- `Border` vytvára orámované panely.
- `ScrollViewer` dovolí rolovať zoznam meraní.
- `ItemsControl` vytvorí jeden checkbox pre každú položku.
- `oxy:PlotView` je grafická plocha z knižnice OxyPlot.

### Ovládacie prvky

Tlačidlo „Load CSV“ zavolá `LoadCsv_Click`; „Export PNG“ zavolá `ExportToPng_Click`. Toto prepojenie tvorí atribút `Click`.

Prepínač štatistík mení vlastnosť `IsChecked`. Konvertor z nej spraví `Visible` (zobraziť) alebo `Collapsed` (skryť a nezaberať miesto).

## 4. `MainWindow.xaml.cs`: udalosti používateľa

**Udalosť** je oznámenie, že sa niečo stalo, napríklad kliknutie.

Vlastnosť `ViewModel` zjednodušuje prístup k `DataContext` a prevedie ho na `GraphViewModel`.

### `LoadCsv_Click()`

1. Otvorí dialóg na výber jedného alebo viacerých súborov.
2. Ak používateľ súbor zvolí, zavolá `ViewModel.LoadMultipleCsvs(...)`.
3. Zmení titulok okna podľa prvého súboru.
4. Pri chybe zobrazí vysvetľujúce okno.

Názov tlačidla a dialóg hovoria CSV, no skutočná cesta načítania používa `JsonLogLoader`. Rozdiel detailnejšie rozoberá nasledujúca lekcia.

### `ExportToPng_Click()`

Najprv skontroluje, či graf obsahuje aspoň jednu dátovú krivku. Ak nič nie je vybraté, zobrazí upozornenie. Inak otvorí dialóg uloženia a použije `PngExporter` z OxyPlot na vytvorenie obrázka 1200 × 800 bodov.

**Výnimka** je informácia, ktorú program vyhodí, keď sa nepodarila operácia. Blok `try/catch` sa pokúsi vykonať úlohu a v `catch` zobrazí chybu namiesto pádu celej aplikácie.

## 5. Názov „ViewModel“ a rozdelenie rolí

V názve `GraphViewModel`:

- **Model** znamená údaje, s ktorými program pracuje.
- **View** znamená to, čo používateľ vidí.
- **ViewModel** pripravuje údaje v podobe vhodnej pre obrazovku.

Ide o vzor **MVVM** (Model–View–ViewModel). Cieľom je, aby súbor rozloženia nemusel obsahovať celé výpočty. Okno prijíma kliknutia, ViewModel spracúva dáta a OxyPlot kreslí graf.

V ďalšej lekcii rozoberieme, ako sa NDJSON mení na tabuľku, ako checkbox vyvolá prekreslenie a ako vznikajú štatistiky.

