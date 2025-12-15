# Opel InfoDisplay

Ein ESP32-basiertes OBD-II Display für Opel-Fahrzeuge mit TFT-Anzeige. Zeigt Live-Motorparameter über Bluetooth-Verbindung zum ELM327 OBD-II Adapter.

## Benötigtes Material

*Hinweis: Die folgenden Links sind Affiliate Links.*

- **ESP32:** https://amzn.to/3YuoEfO
- **Display:** https://amzn.to/4s5zk2f
- **12V Netzteil:** https://amzn.to/45c1ApQ
- **OBD2 Adapter:** https://amzn.to/4pDKYj0

## Übersicht

Dieses Projekt verwandelt einen ESP32-Mikrocontroller mit TFT-Display in ein professionelles Fahrzeug-Infodisplay. Es liest OBD-II-Daten über Bluetooth aus und stellt sie übersichtlich auf einem farbigen TFT-Display dar.

### Features

- **Live-Motordaten:**
  - Motorlast (0-100%) mit Rundinstrument und Farbverlauf (Cyan → Grün → Orange → Rot)
  - Kühlmitteltemperatur (0-125°C) mit Farbverlauf (Blau → Grün → Rot)
  - Batteriespannung (V) mit Farbcodierung
  - Ansauglufttemperatur (°C) mit Farbcodierung
  - Durchschnittsverbrauch (L/100km) mit Farbcodierung
  
- **Intelligente Anzeige:**
  - Optimierte Ring-Meter mit Anti-Flacker-Technologie
  - Farbcodierte Anzeigen für alle relevanten Parameter
  - Delta-Updates (nur bei Wertänderung)
  - Skalierbare UI für verschiedene Display-Größen

- **Demo-Modus:**
  - Läuft auch ohne OBD-Verbindung
  - Simulierte, realistische Testdaten

## 🔧 Hardware

### Benötigte Komponenten

- **ESP32 Development Board** (z.B. UPESY WROOM)
- **TFT Display ST7789** (320x240 Pixel)
- **ELM327 Bluetooth OBD-II Adapter**
- Verbindungskabel

### Pin-Belegung (ESP32 → ST7789 Display)

| ESP32 Pin | TFT Signal | Funktion            |
|-----------|------------|---------------------|
| GPIO 23   | MOSI       | SPI Data Out        |
| GPIO 18   | SCLK       | SPI Clock           |
| GPIO 17   | CS         | Chip Select         |
| GPIO 4    | DC         | Data/Command Select |
| -         | RST        | Reset (nicht belegt)|
| GPIO 12   | BL         | Backlight Control   |
| GND       | GND        | Ground              |
| 3.3V      | VCC        | Power Supply        |

**Hinweis:** MISO wird nicht verwendet, da das Display keine Daten zurücksendet.

### Pin-Belegung (ESP32-S2 → ST7789 Display)

| ESP32-S2 Pin | TFT Signal | Funktion            |
|--------------|------------|---------------------|
| GPIO 35      | MOSI       | SPI Data Out        |
| GPIO 36      | SCLK       | SPI Clock           |
| GPIO 34      | CS         | Chip Select         |
| GPIO 37      | DC         | Data/Command Select |
| -            | RST        | Reset (nicht belegt)|
| GPIO 33      | BL         | Backlight Control   |
| GND          | GND        | Ground              |
| 3.3V         | VCC        | Power Supply        |

**Wichtig für ESP32-S2:** Der ESP32-S2 unterstützt nur Bluetooth Low Energy (BLE), nicht Bluetooth Classic. Für die Verwendung mit ELM327 wird ein BLE-fähiger Adapter oder eine serielle Verbindung (UART) benötigt.

### SPI-Konfiguration

```
SPI Frequenz: 27 MHz (actual: 26.67 MHz)
Display Auflösung: 320x240 Pixel
Farbtiefe: 16-bit (RGB565)
```

## 📦 Installation

### 1. PlatformIO installieren

```bash
# VS Code Extension installieren:
# PlatformIO IDE for VSCode
```

### 2. Projekt klonen/herunterladen

```bash
cd ~/dev
git clone <dein-repo> Opel-Infodisplay
cd Opel-Infodisplay
```

### 3. Abhängigkeiten

Die benötigten Libraries werden automatisch durch PlatformIO installiert:
- `bodmer/TFT_eSPI` (v2.5.43)
- `powerbroker2/ELMDuino` (v3.4.1)

### 4. Fonts hochladen

Die Fonts müssen ins LittleFS-Dateisystem des ESP32 hochgeladen werden:

```bash
# In PlatformIO Terminal:
pio run --target uploadfs
```

**Wichtig:** Die Fonts im `data/` Verzeichnis müssen zuerst hochgeladen werden, bevor der Code funktioniert:
- `NotoSansBold15.vlw`
- `NotoSansBold36.vlw`
- `Latin-Hiragana-24.vlw`

### 5. ELM327 MAC-Adresse anpassen

In [src/main.cpp](src/main.cpp) die MAC-Adresse deines ELM327 Adapters eintragen:

```cpp
// Zeile 240 in main.cpp:
uint8_t address[6] = {0xD1, 0x03, 0xD5, 0xE3, 0xA5, 0x7D};
```

**MAC-Adresse herausfinden:**
1. ELM327 mit Smartphone pairen
2. In Bluetooth-Einstellungen MAC-Adresse notieren
3. Format: `AA:BB:CC:DD:EE:FF` → `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}`

### 6. Kompilieren und Hochladen

```bash
# Build und Upload:
pio run --target upload

# Serielle Ausgabe anzeigen:
pio device monitor
```

## 🚗 Verwendung

### Erste Inbetriebnahme

1. **Display-Größe einstellen** (falls nicht 320x240):
   
   In [src/main.cpp](src/main.cpp#L584) im `setup()`:
   ```cpp
   // Für 240x280 Display:
   setDisplaySize_240x280();
   
   // Für 320x240 Display (Standard):
   setDisplaySize_320x240();
   
   // Für 480x320 Display:
   setDisplaySize_480x320();
   
   // Für beliebige Größe:
   setCustomDisplaySize(width, height);
   ```

2. **ELM327 einschalten** (Zündung an)

3. **ESP32 mit Strom versorgen**

4. **Warten auf Verbindung:**
   - Display zeigt Opel-Logo
   - Bluetooth verbindet sich automatisch
   - Nach erfolgreicher Verbindung erscheint die Anzeige

### Demo-Modus

Falls keine OBD-Verbindung besteht, läuft das Display automatisch im Demo-Modus mit simulierten Werten. Ideal zum Testen ohne Auto.

## 🎨 Display-Layout

```
┌──────────────────────────────────────┐
│   ╭───╮           ╭───╮              │
│   │ % │   LAST    │°C │   TEMP.      │
│   ╰───╯           ╰───╯              │
│────────────────────────────────────────│
│ Batt  │ Ansaug.T │       L/100       │
│ 12.6V │   25°C   │       8.5L        │
└──────────────────────────────────────┘
```

### Anzeigen und Farbcodierung

- **Links oben - Motorlast (Ring-Meter):**
  - 🔵 0-30%: Cyan (niedriger Bereich)
  - 🟢 30-60%: Grün (normaler Bereich)
  - 🟠 60-80%: Orange (erhöhter Bereich)
  - 🔴 80-100%: Rot (hoher Bereich)
  - Fließende Farbübergänge zwischen den Bereichen

- **Rechts oben - Kühlmitteltemperatur (Ring-Meter):**
  - 🔵 0-50°C: Blau (Motor kalt)
  - 🟢 60-80°C: Grün (Betriebstemperatur)
  - 🔴 90-125°C: Rot (Überhitzung)
  - Fließende Farbübergänge zwischen den Bereichen

- **Unten links - Batteriespannung:**
  - 🔴 < 12,4V: Rot (niedrige Spannung)
  - 🔵 12,4-13,8V: Cyan (Normalbetrieb)
  - 🟢 > 13,8V: Grün (Ladevorgang)

- **Unten mitte - Ansauglufttemperatur:**
  - 🔵 < 20°C: Cyan (kalte Ansaugluft)
  - 🟢 20-40°C: Grün (optimaler Bereich)
  - 🔴 > 40°C: Rot (warme Ansaugluft)

- **Unten rechts - Ø Verbrauch (Trip-Computer):**
  - 🟢 < 8L/100km: Grün (sparsam)
  - 🟠 8-10L/100km: Orange (normal)
  - 🔴 > 10L/100km: Rot (hoher Verbrauch)

## 🛠️ Konfiguration

### Demo-Modus Ein/Aus

In [platformio.ini](platformio.ini) kann der Demo-Modus gesteuert werden:

```ini
build_flags = 
  ; ... andere Flags ...
  ; Demo Mode: 1 = ON (keine Fehlermeldung bei Verbindungsfehler), 0 = OFF (Reboot bei Verbindungsfehler)
  -DDEMO_MODE=1
```

**Demo-Modus EIN** (`-DDEMO_MODE=1`, Standard):
- Bei Verbindungsfehlern läuft das Display im Demo-Modus weiter
- Zeigt simulierte, realistische Testdaten
- Ideal zum Testen ohne Auto/OBD-Verbindung

**Demo-Modus AUS** (`-DDEMO_MODE=0`):
- Bei Verbindungsfehlern wird eine Fehlermeldung angezeigt
- Countdown von 30 Sekunden bis zum automatischen Neustart
- Für den Produktivbetrieb im Fahrzeug empfohlen

### Display-Anpassungen

Die Display-Konfiguration erfolgt über [src/main.cpp](src/main.cpp):

```cpp
// Base-Konfiguration für verschiedene Displays
const DisplayConfig baseConfig = {
  280, 240,          // Breite x Höhe
  {70, 70, 70},     // Boost-Meter Position
  {210, 70, 70},    // Coolant-Meter Position
  // ... weitere Positionen
};
```

### ELM327 Verbindungseinstellungen

```cpp
// Timeout für OBD-Befehle (in ms):
myELM327.begin(ELM_PORT, true, 2000);
```

## 📊 OBD-II PIDs

Das Projekt liest folgende PIDs aus:

| PID  | Beschreibung              | Einheit  |
|------|---------------------------|----------|
| 0x04 | Engine Load               | %        |
| 0x05 | Coolant Temperature       | °C       |
| 0x0D | Vehicle Speed             | km/h     |
| 0x0F | Intake Air Temperature    | °C       |
| 0x42 | Control Module Voltage    | V        |
| 0x5E | Fuel Rate                 | L/h      |

## 🐛 Fehlerbehandlung

### Display zeigt "Font missing in Flash FS"
→ Fonts nicht hochgeladen. Führe `pio run --target uploadfs` aus.

### "Couldn't connect to OBD scanner"
→ Prüfe ELM327 MAC-Adresse und ob Adapter eingeschaltet ist.
→ Demo-Modus wird automatisch aktiviert.

### Display bleibt schwarz
→ Prüfe Pin-Belegung und Backlight (GPIO 12).
→ Stelle sicher, dass 3.3V Stromversorgung ausreicht.

### Flackernde Anzeige
→ SPI-Frequenz reduzieren (in [platformio.ini](platformio.ini)):
```ini
-DSPI_FREQUENCY=20000000
```

## 🔬 Entwicklung

### Debug-Ausgabe

Über Serial Monitor (115200 baud):
```
Starting Bluetooth connection...
Connected to ELM327
LOAD: 45.5 %
Coolant: 87.0 °C
Battery: 12.8 V
Speed: 65 km/h
```

### Code-Struktur

```
src/
├── main.cpp              # Hauptprogramm
├── opel-logo-brand.h     # Opel Logo (Bitmap)

TFT_eSPI_Setups/
├── my_Setup.h            # Display-Konfiguration

data/
├── *.vlw                 # Anti-Aliased Fonts
```

### Anpassungen

**Andere Fahrzeugmarke:**
- Logo in [src/opel-logo-brand.h](src/opel-logo-brand.h) ersetzen
- Labels in `setup()` anpassen

**Zusätzliche OBD-PIDs:**
1. Neuen State in `obd_pid_states` hinzufügen
2. Case in `loop()` implementieren
3. Draw-Funktion erstellen

## 📝 Lizenz

Dieses Projekt steht unter der MIT-Lizenz. Siehe LICENSE-Datei für Details.

## 🙏 Credits

- **TFT_eSPI** von Bodmer - https://github.com/Bodmer/TFT_eSPI
- **ELMduino** von PowerBroker2 - https://github.com/PowerBroker2/ELMduino
- **Opel Logo** - Eigentum von Opel Automobile GmbH

## 📞 Support

Bei Fragen oder Problemen bitte ein Issue auf GitHub erstellen.

---

**Happy Driving! 🚗💨**
