#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "ELMduino.h"
#include "BluetoothSerial.h"
#include "opel-logo-brand.h"
//#include "battery.h"
#include <LittleFS.h>

//////////////////////////////////////////////////////////////////////////
// TFT_eSPI
//////////////////////////////////////////////////////////////////////////

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite boosttxt = TFT_eSprite(&tft);
TFT_eSprite coolanttxt = TFT_eSprite(&tft);
TFT_eSprite batteryicon = TFT_eSprite(&tft);
TFT_eSprite volt = TFT_eSprite(&tft);
TFT_eSprite intaketxt = TFT_eSprite(&tft);
TFT_eSprite tripavgtxt = TFT_eSprite(&tft);
TFT_eSprite buttonspray = TFT_eSprite(&tft);
TFT_eSprite opellogo = TFT_eSprite(&tft);
TFT_eSprite ui = TFT_eSprite(&tft);
TFT_eSprite errorspray = TFT_eSprite(&tft);

//////////////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////////////

#define DARKER_GREY 0x18E3
#define GAUGE_GREY 0x4A49   // Helleres Grau für Gauge-Hintergründe
#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_MEDIUM "Latin-Hiragana-24"
#define AA_FONT_LARGE "NotoSansBold36"
#define FlashFS LittleFS

//////////////////////////////////////////////////////////////////////////
// Display Configuration
//////////////////////////////////////////////////////////////////////////

// Base display configuration (original 280x240 from your current setup)
struct DisplayConfig {
  int width;
  int height;
  
  // Meter configurations
  struct {
    int x, y, radius;
  } boostMeter, coolantMeter;
  
  // Text positions
  struct {
    int x, y, width, height;
  } boostText, coolantText, voltageText, intakeText, tripAvgText;
  
  // Label positions
  struct {
    int lastX, lastY;
    int coolantX, coolantY;
    int voltageX, voltageY;
    int intakeX, intakeY;
  } labels;
  
  // Battery icon position
  struct {
    int x, y, width, height;
  } batteryIcon;
};

// Current display configuration
DisplayConfig displayConfig;

// Base configuration for 280x240 display
const DisplayConfig baseConfig = {
  280, 240,  // width, height
  {70, 70, 70},   // boostMeter - oben und links bündig
  {210, 70, 70},  // coolantMeter - oben und rechts bündig
  {35, 45, 70, 60},   // boostText - höhere Textbox für vollständigen Text
  {175, 45, 70, 60},  // coolantText - höhere Textbox für vollständigen Text
  {0, 200, 80, 32}, // voltageText - breiter für vollständiges "XX.X V"
  {96, 200, 80, 32}, // intakeText - rechts neben voltage
  {195, 200, 90, 32}, // tripAvgText - ganz rechts unten für Ø Verbrauch
  {55, 165, 185, 165, 10, 190, 155, 190}, // labels (Last, Coolant, Voltage, Intake) - weiter nach unten für große Gauges
};

//////////////////////////////////////////////////////////////////////////
// Scaling Functions
//////////////////////////////////////////////////////////////////////////

void initDisplayConfig(int displayWidth, int displayHeight) {
  float scaleX = (float)displayWidth / baseConfig.width;
  float scaleY = (float)displayHeight / baseConfig.height;
  
  displayConfig.width = displayWidth;
  displayConfig.height = displayHeight;
  
  // Scale meter positions and sizes
  displayConfig.boostMeter.x = (int)(baseConfig.boostMeter.x * scaleX);
  displayConfig.boostMeter.y = (int)(baseConfig.boostMeter.y * scaleY);
  displayConfig.boostMeter.radius = (int)(baseConfig.boostMeter.radius * min(scaleX, scaleY));
  
  displayConfig.coolantMeter.x = (int)(baseConfig.coolantMeter.x * scaleX);
  displayConfig.coolantMeter.y = (int)(baseConfig.coolantMeter.y * scaleY);
  displayConfig.coolantMeter.radius = (int)(baseConfig.coolantMeter.radius * min(scaleX, scaleY));
  
  // Scale text positions
  displayConfig.boostText.x = (int)(baseConfig.boostText.x * scaleX);
  displayConfig.boostText.y = (int)(baseConfig.boostText.y * scaleY);
  displayConfig.boostText.width = (int)(baseConfig.boostText.width * scaleX);
  displayConfig.boostText.height = (int)(baseConfig.boostText.height * scaleY);
  
  displayConfig.coolantText.x = (int)(baseConfig.coolantText.x * scaleX);
  displayConfig.coolantText.y = (int)(baseConfig.coolantText.y * scaleY);
  displayConfig.coolantText.width = (int)(baseConfig.coolantText.width * scaleX);
  displayConfig.coolantText.height = (int)(baseConfig.coolantText.height * scaleY);
  
  displayConfig.voltageText.x = (int)(baseConfig.voltageText.x * scaleX);
  displayConfig.voltageText.y = (int)(baseConfig.voltageText.y * scaleY);
  displayConfig.voltageText.width = (int)(baseConfig.voltageText.width * scaleX);
  displayConfig.voltageText.height = (int)(baseConfig.voltageText.height * scaleY);
  
  displayConfig.intakeText.x = (int)(baseConfig.intakeText.x * scaleX);
  displayConfig.intakeText.y = (int)(baseConfig.intakeText.y * scaleY);
  displayConfig.intakeText.width = (int)(baseConfig.intakeText.width * scaleX);
  displayConfig.intakeText.height = (int)(baseConfig.intakeText.height * scaleY);
  
  displayConfig.tripAvgText.x = (int)(baseConfig.tripAvgText.x * scaleX);
  displayConfig.tripAvgText.y = (int)(baseConfig.tripAvgText.y * scaleY);
  displayConfig.tripAvgText.width = (int)(baseConfig.tripAvgText.width * scaleX);
  displayConfig.tripAvgText.height = (int)(baseConfig.tripAvgText.height * scaleY);
  
  // Scale label positions
  displayConfig.labels.lastX = (int)(baseConfig.labels.lastX * scaleX);
  displayConfig.labels.lastY = (int)(baseConfig.labels.lastY * scaleY);
  displayConfig.labels.coolantX = (int)(baseConfig.labels.coolantX * scaleX);
  displayConfig.labels.coolantY = (int)(baseConfig.labels.coolantY * scaleY);
  displayConfig.labels.voltageX = (int)(baseConfig.labels.voltageX * scaleX);
  displayConfig.labels.voltageY = (int)(baseConfig.labels.voltageY * scaleY);
  displayConfig.labels.intakeX = (int)(baseConfig.labels.intakeX * scaleX);
  displayConfig.labels.intakeY = (int)(baseConfig.labels.intakeY * scaleY);
  
  // Scale battery icon
  displayConfig.batteryIcon.x = (int)(baseConfig.batteryIcon.x * scaleX);
  displayConfig.batteryIcon.y = (int)(baseConfig.batteryIcon.y * scaleY);
  displayConfig.batteryIcon.width = (int)(baseConfig.batteryIcon.width * scaleX);
  displayConfig.batteryIcon.height = (int)(baseConfig.batteryIcon.height * scaleY);
}

// Helper function to get scaled text size
int getScaledTextSize(int baseSize) {
  float scale = min((float)displayConfig.width / baseConfig.width, 
                   (float)displayConfig.height / baseConfig.height);
  return max(1, (int)(baseSize * scale));
}

// Helper function to select appropriate font based on display size
const char* getScaledFont() {
  float scale = min((float)displayConfig.width / baseConfig.width, 
                   (float)displayConfig.height / baseConfig.height);
  
  if (scale >= 1.5) {
    return AA_FONT_LARGE;    // Große Displays verwenden große Fonts
  } else if (scale >= 0.8) {
    return AA_FONT_MEDIUM;   // Mittlere Displays verwenden mittlere Fonts
  } else {
    return AA_FONT_SMALL;    // Kleine Displays verwenden kleine Fonts
  }
}

// Helper function for unit label font (smaller than main font)
const char* getScaledUnitFont() {
  float scale = min((float)displayConfig.width / baseConfig.width, 
                   (float)displayConfig.height / baseConfig.height);
  
  if (scale >= 1.5) {
    return AA_FONT_MEDIUM;   // Große Displays: mittlere Font für Units
  } else if (scale >= 0.8) {
    return AA_FONT_SMALL;    // Mittlere Displays: kleine Font für Units
  } else {
    return AA_FONT_SMALL;    // Kleine Displays: kleine Font für Units
  }
}

// Stufenlose Font-Skalierung für Gauge-Zahlen - einfache Lösung ohne struct
float getGaugeTextScale(int gaugeRadius) {
  float scale = min((float)displayConfig.width / baseConfig.width, 
                   (float)displayConfig.height / baseConfig.height);
  
  // Berechne Font-Skalierung basierend auf Gauge-Radius und Display-Skalierung
  float fontScale = scale * (gaugeRadius / 60.0);  // 60 ist der Base-Radius
  
  // Berechne Text-Skalierung: noch kleinere Werte für kompakte, aber fette Schrift
  float textScale = 0.3 + (fontScale * 0.5);  // Skalierung zwischen 0.3x und 0.8x
  
  // Begrenze die Skalierung auf kleinere Werte für kompakte, fette Darstellung
  if (textScale < 0.25) textScale = 0.25;
  if (textScale > 1.0) textScale = 1.0;
  
  return textScale;
}

//////////////////////////////////////////////////////////////////////////
// Display Size Presets - einfach diese Funktion aufrufen für verschiedene Displays
//////////////////////////////////////////////////////////////////////////

void setDisplaySize_240x280() {
  initDisplayConfig(240, 280);
}

void setDisplaySize_320x240() {
  initDisplayConfig(320, 240);
}

void setDisplaySize_480x320() {
  initDisplayConfig(480, 320);
}

void setDisplaySize_128x160() {
  initDisplayConfig(128, 160);
}

// Für beliebige Displaygrößen
void setCustomDisplaySize(int width, int height) {
  initDisplayConfig(width, height);
}

BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

//String MACadd = "D1:03:D5:E3:A5:7D";                         //enter the ELM327 MAC address
uint8_t address[6]  = {0xD1, 0x03, 0xD5, 0xE3, 0xA5, 0x7D};  //enter the ELM327 MAC address after the 0x

//////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////

int reading = 0; // Value to be displayed

static uint16_t last_angle_boost = 30;
static uint16_t last_angle_coolant = 30;

typedef enum
{
  COOLANT,
  LOAD,
  VOLTAGE,
  INTAKE_TEMP,
  SPEED,
  FUEL_ECONOMY,
  RPM
} obd_pid_states;
obd_pid_states obd_state = VOLTAGE;

float coolant = 0;
float voltage = 0;
float load = 0;
float intakeTemp = 0;
float vehicleSpeed = 0;
float engineRPM = 0;

// Fuel Economy Variablen (gleitender Durchschnitt)
float currentFuelEconomy = 0;     // Momentanverbrauch L/100km (von OBD2)
float avgFuelEconomy = 0;         // Gleitender Durchschnitt L/100km
float fuelEconomySum = 0;         // Summe für Durchschnittsberechnung
unsigned long fuelEconomySamples = 0;  // Anzahl Messungen seit Neustart

// Vorherige Werte für Delta-Updates (verhindert unnötige Redraws)
float prev_coolant = -999;
float prev_voltage = -999;
float prev_load = -999;
float prev_intakeTemp = -999;
float prev_avgFuelEconomy = -999;

String message="";

//////////////////////////////////////////////////////////////////////////
// Motor-Aus-Erkennung und DTC (Fehlerspeicher) Variablen
//////////////////////////////////////////////////////////////////////////

// Ignition Detection Variablen (Drehzahlbasiert)
bool engineRunning = false;
bool previousEngineRunning = false;
unsigned long engineOffTime = 0;
const unsigned long ENGINE_OFF_DELAY = 3000;  // 3 Sekunden Verzögerung zur Sicherheit
const float RPM_THRESHOLD_RUNNING = 400.0;     // >= 400 RPM = Motor läuft

// DTC (Diagnostic Trouble Code) Variablen
String dtcCodes[10];  // Max 10 Fehler speichern
int dtcCount = 0;
bool dtcScreenActive = false;
unsigned long dtcScreenStartTime = 0;
const unsigned long DTC_SCREEN_DURATION = 15000;  // 15 Sekunden DTC-Anzeige

//////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////

void(* resetFunc) (void) = 0;

// Forward Declarations (Funktionen die später definiert sind)
void drawBoost(float load);
void drawCoolant(float reading);

//////////////////////////////////////////////////////////////////////////
// DTC-Analyse-Funktionen
//////////////////////////////////////////////////////////////////////////

// Bestimme Kritikalität und Farbe eines DTC-Codes
uint16_t getDTCColor(String dtcCode) {
  char type = dtcCode.charAt(0);
  
  // Extrahiere Subsystem (zweite Ziffer bei P-Codes)
  if (dtcCode.length() >= 4) {
    char subsystem = dtcCode.charAt(2);
    
    // 🔴 KRITISCH (Rot)
    // Zündaussetzer (P030x, P031x)
    if (type == 'P' && subsystem == '3') {
      return TFT_RED;
    }
    // Chassis-Codes (Bremsen, ABS, ESP)
    if (type == 'C') {
      return TFT_RED;
    }
    
    // 🟠 WARNUNG (Orange/Gelb)
    // Katalysator (P042x, P043x)
    if (type == 'P' && subsystem == '4') {
      return TFT_ORANGE;
    }
    // Gemisch (P01xx)
    if (type == 'P' && subsystem == '1') {
      return TFT_ORANGE;
    }
    
    // 🟡 INFO (Cyan/Hellblau)
    // Body-Codes (Komfort)
    if (type == 'B') {
      return TFT_CYAN;
    }
  }
  
  // Default: Weiß
  return TFT_WHITE;
}

// Kurzbeschreibung basierend auf DTC-Code
String getDTCDescription(String dtcCode) {
  if (dtcCode.length() < 5) return "Unbekannt";
  
  char type = dtcCode.charAt(0);
  String code = dtcCode.substring(1);  // z.B. "0420"
  
  // Häufige Codes mit deutschen Beschreibungen
  if (dtcCode == "P0420") return "Katalysator";
  if (dtcCode == "P0430") return "Katalysator Bank2";
  if (dtcCode == "P0171") return "Gemisch mager";
  if (dtcCode == "P0172") return "Gemisch fett";
  if (dtcCode == "P0174") return "Gemisch mager B2";
  if (dtcCode == "P0175") return "Gemisch fett B2";
  if (dtcCode == "P0128") return "Kuehlmitteltemp.";
  if (dtcCode == "P0133") return "O2 Sensor traege";
  if (dtcCode == "P0300") return "Zuendaussetzer";
  if (dtcCode == "P0301") return "Zuendauss. Zyl.1";
  if (dtcCode == "P0302") return "Zuendauss. Zyl.2";
  if (dtcCode == "P0303") return "Zuendauss. Zyl.3";
  if (dtcCode == "P0304") return "Zuendauss. Zyl.4";
  if (dtcCode == "P0305") return "Zuendauss. Zyl.5";
  if (dtcCode == "P0306") return "Zuendauss. Zyl.6";
  if (dtcCode == "P0440") return "Tankentlueftung";
  if (dtcCode == "P0441") return "EVAP Durchfluss";
  if (dtcCode == "P0442") return "EVAP Leck klein";
  if (dtcCode == "P0455") return "EVAP Leck gross";
  if (dtcCode == "P0456") return "EVAP Leck mini";
  
  // Fallback: Bestimme nach System-Typ
  if (type == 'P') {
    char subsystem = dtcCode.charAt(2);
    if (subsystem == '1') return "Kraftstoff/Luft";
    if (subsystem == '2') return "Injektor";
    if (subsystem == '3') return "Zuendung";
    if (subsystem == '4') return "Abgas";
    if (subsystem == '5') return "Drehzahl";
    if (subsystem == '6') return "Steuergeraet";
    if (subsystem == '7' || subsystem == '8') return "Getriebe";
    return "Motor";
  }
  if (type == 'C') return "Fahrwerk";
  if (type == 'B') return "Karosserie";
  if (type == 'U') return "Netzwerk";
  
  return "Unbekannt";
}

//////////////////////////////////////////////////////////////////////////
// Motor-Status-Erkennung (Drehzahlbasiert)
//////////////////////////////////////////////////////////////////////////

// Prüfe ob Motor läuft basierend auf Motordrehzahl
bool isEngineRunning() {
  // Drehzahl-basiert: >= 400 RPM = Motor läuft
  if (engineRPM >= RPM_THRESHOLD_RUNNING) {
    return true;
  }
  
  return false;  // Motor aus: Drehzahl < 400 RPM
}

//////////////////////////////////////////////////////////////////////////
// DTC-Fehlerspeicher auslesen
//////////////////////////////////////////////////////////////////////////

void readDTCs() {
  DEBUG_PORT.println("Reading DTCs from vehicle...");
  
  dtcCount = 0;  // Zurücksetzen
  
  // Manuelle DTC-Abfrage über AT-Command
  // ELM327 Command "03" = Request stored DTCs
  ELM_PORT.print("03\r");
  
  delay(500);  // Warte auf Antwort
  
  String response = "";
  unsigned long timeout = millis() + 2000;  // 2 Sekunden Timeout
  
  while (millis() < timeout) {
    if (ELM_PORT.available()) {
      char c = ELM_PORT.read();
      response += c;
      
      // Wenn '>' empfangen: Antwort komplett
      if (c == '>') {
        break;
      }
    }
  }
  
  DEBUG_PORT.print("DTC Response: ");
  DEBUG_PORT.println(response);
  
  // Parse DTC Response
  // Format: "43 01 33 00 00 00 00" = 1 DTC: P0133
  // Erste Byte "43" = Mode 3 Response
  // Zweite Byte = Anzahl DTCs
  
  if (response.indexOf("43") >= 0) {
    // Extrahiere Anzahl DTCs (Byte nach "43")
    int pos = response.indexOf("43");
    if (pos >= 0 && response.length() > pos + 5) {
      String countStr = response.substring(pos + 3, pos + 5);
      dtcCount = strtol(countStr.c_str(), NULL, 16);
      
      DEBUG_PORT.print("Found ");
      DEBUG_PORT.print(dtcCount);
      DEBUG_PORT.println(" DTCs");
      
      // Parse einzelne DTCs (jeder DTC = 2 Bytes = 4 Hex-Zeichen)
      if (dtcCount > 0 && dtcCount <= 10) {
        int dtcPos = pos + 6;  // Position nach "43 XX "
        
        for (int i = 0; i < dtcCount && i < 10; i++) {
          if (response.length() >= dtcPos + 4) {
            String dtcHex = response.substring(dtcPos, dtcPos + 4);
            dtcHex.trim();
            
            // Konvertiere Hex zu DTC-Code (z.B. "0133" -> "P0133")
            // Erste 2 Bits bestimmen Typ: 00=P, 01=C, 10=B, 11=U
            int firstByte = strtol(dtcHex.substring(0, 2).c_str(), NULL, 16);
            int secondByte = strtol(dtcHex.substring(2, 4).c_str(), NULL, 16);
            
            char dtcType = 'P';  // Default: Powertrain
            int typeCode = (firstByte >> 6) & 0x03;
            if (typeCode == 1) dtcType = 'C';      // Chassis
            else if (typeCode == 2) dtcType = 'B';  // Body
            else if (typeCode == 3) dtcType = 'U';  // Network
            
            // Erstelle DTC-String: z.B. "P0133"
            char dtcStr[6];
            sprintf(dtcStr, "%c%01X%02X", dtcType, firstByte & 0x3F, secondByte);
            dtcCodes[i] = String(dtcStr);
            
            DEBUG_PORT.print("DTC ");
            DEBUG_PORT.print(i);
            DEBUG_PORT.print(": ");
            DEBUG_PORT.println(dtcCodes[i]);
            
            dtcPos += 5;  // Nächster DTC (4 Zeichen + Leerzeichen)
          }
        }
      }
    }
  } else if (response.indexOf("NO DATA") >= 0 || response.indexOf("43 00") >= 0) {
    DEBUG_PORT.println("No DTCs found - all clear!");
    dtcCount = 0;
  } else {
    DEBUG_PORT.println("Failed to read DTCs or invalid response");
    dtcCount = 0;
  }
}

//////////////////////////////////////////////////////////////////////////
// DTC-Bildschirm anzeigen nach Motor-Aus
//////////////////////////////////////////////////////////////////////////

void displayDTCScreen() {
  // Sicherstellen dass keine Fonts geladen sind
  tft.unloadFont();
  
  // Reset auf Standard-Text-Einstellungen
  tft.setTextSize(1);
  tft.setTextFont(1);
  
  // Bildschirm ZUERST komplett schwarz - BEVOR Sprites gelöscht werden
  tft.fillScreen(TFT_BLACK);
  
  // Explizit alle Sprites löschen (nach fillScreen, damit nichts mehr drüber gepusht wird)
  volt.deleteSprite();
  intaketxt.deleteSprite();
  tripavgtxt.deleteSprite();
  boosttxt.deleteSprite();
  coolanttxt.deleteSprite();
  ui.deleteSprite();
  
  if (dtcCount == 0) {
    // Keine Fehler - grüner Screen
    tft.loadFont(AA_FONT_MEDIUM, LittleFS);
    tft.setTextDatum(MC_DATUM);  // NACH loadFont setzen!
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("ALLES OK!", displayConfig.width / 2, displayConfig.height / 2 - 20);
    tft.unloadFont();
    
    tft.loadFont(AA_FONT_SMALL, LittleFS);
    tft.setTextDatum(MC_DATUM);  // NACH loadFont setzen!
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Keine Fehler", displayConfig.width / 2, displayConfig.height / 2 + 20);
    tft.unloadFont();
  } else {
    // Fehler vorhanden - Kompakte Darstellung
    
    // Überschrift: "DTC-Fehler (5)" - klein und oben am Rand
    tft.loadFont(AA_FONT_SMALL, LittleFS);
    tft.setTextDatum(TC_DATUM);  // NACH loadFont setzen!
    tft.setTextColor(TFT_RED, TFT_BLACK);
    String headerStr = "DTC-Fehler (" + String(dtcCount) + ")";
    tft.drawString(headerStr, displayConfig.width / 2, 5);
    tft.unloadFont();
    
    // Fehler-Liste direkt darunter beginnen - Built-in Font (viel kleiner)
    tft.setTextFont(1);  // Built-in 6x8 Font
    tft.setTextSize(1);  // Standard-Größe
    tft.setTextDatum(TL_DATUM);
    
    int startY = 30;  // Direkt unter Überschrift
    int lineHeight = 11;  // Kompakter für kleine Font
    int leftMargin = 5;
    
    // Zeichne alle Fehler untereinander (max 10)
    for (int i = 0; i < min(dtcCount, 10); i++) {
      String dtcCode = dtcCodes[i];
      String description = getDTCDescription(dtcCode);
      uint16_t codeColor = getDTCColor(dtcCode);
      
      int yPos = startY + (i * lineHeight);
      
      // DTC-Code in Farbe (je nach Kritikalität)
      tft.setTextColor(codeColor, TFT_BLACK);
      tft.drawString(dtcCode, leftMargin, yPos);
      
      // Beschreibung in Weiß und Klammern - direkt danach mit Leerzeichen
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      String descText = " (" + description + ")";
      // Position: Code-Breite (ca. 36 Pixel für 6 Zeichen) + kleiner Abstand
      tft.drawString(descText, leftMargin + 36, yPos);
    }
    
    // Font wird nicht entladen, da wir Built-in Font verwenden
  }
  
  dtcScreenActive = true;
  dtcScreenStartTime = millis();
}

//////////////////////////////////////////////////////////////////////////
// Motor-Status überwachen und bei Motor-Aus DTCs prüfen
//////////////////////////////////////////////////////////////////////////

void checkEngineStatus() {
  // Aktuellen Motor-Status ermitteln
  engineRunning = isEngineRunning();
  
  // Erkennung Motor-Aus Event
  if (previousEngineRunning && !engineRunning) {
    // Motor wurde gerade ausgeschaltet!
    engineOffTime = millis();
    DEBUG_PORT.println("\n========================================");
    DEBUG_PORT.println("ENGINE TURNED OFF - Starting DTC check...");
    DEBUG_PORT.println("========================================\n");
    
    // Kurz warten zur Sicherheit (Motor wirklich aus?)
    delay(ENGINE_OFF_DELAY);
    
    // Nochmal prüfen ob Motor wirklich aus ist
    if (!isEngineRunning()) {
      // DTCs auslesen
      readDTCs();
      
      // DTC-Screen anzeigen
      displayDTCScreen();
    } else {
      DEBUG_PORT.println("False alarm - engine still running");
    }
  }
  
  previousEngineRunning = engineRunning;
}

//////////////////////////////////////////////////////////////////////////
// UI nach DTC-Screen wiederherstellen
//////////////////////////////////////////////////////////////////////////

void restoreNormalUI() {
  DEBUG_PORT.println("Restoring normal UI...");
  
  // Explizit alle Fonts entladen und Screen löschen
  tft.unloadFont();
  tft.setTextSize(1);
  tft.setTextFont(1);
  
  // Reset alle previous Werte um komplettes Redraw zu erzwingen
  prev_coolant = -999;
  prev_voltage = -999;
  prev_load = -999;
  prev_intakeTemp = -999;
  prev_avgFuelEconomy = -999;
  last_angle_boost = 30;
  last_angle_coolant = 30;
  
  // Bildschirm komplett schwarz neu aufbauen
  tft.fillScreen(TFT_BLACK);
  
  // Sprites neu erstellen ZUERST (wurden in displayDTCScreen() gelöscht)
  volt.setColorDepth(8);
  volt.createSprite(displayConfig.voltageText.width, displayConfig.voltageText.height);
  volt.fillSprite(TFT_BLACK);
  volt.setTextColor(TFT_WHITE, TFT_BLACK);
  
  intaketxt.setColorDepth(8);
  intaketxt.createSprite(displayConfig.intakeText.width, displayConfig.intakeText.height);
  intaketxt.fillSprite(TFT_BLACK);
  intaketxt.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tripavgtxt.setColorDepth(8);
  tripavgtxt.createSprite(displayConfig.tripAvgText.width, displayConfig.tripAvgText.height);
  tripavgtxt.fillSprite(TFT_BLACK);
  tripavgtxt.setTextColor(TFT_CYAN, TFT_BLACK);
  
  boosttxt.setColorDepth(8);
  boosttxt.createSprite(displayConfig.boostText.width, displayConfig.boostText.height);
  boosttxt.fillSprite(GAUGE_GREY);
  boosttxt.setTextColor(TFT_WHITE, GAUGE_GREY);
  
  coolanttxt.setColorDepth(8);
  coolanttxt.createSprite(displayConfig.coolantText.width, displayConfig.coolantText.height);
  coolanttxt.fillSprite(GAUGE_GREY);
  coolanttxt.setTextColor(TFT_WHITE, GAUGE_GREY);
  
  ui.setColorDepth(8);
  ui.createSprite(displayConfig.width, displayConfig.height);
  ui.fillSprite(TFT_BLACK);
  ui.setSwapBytes(true);
  ui.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // UI Grundstruktur zeichnen
  ui.fillSprite(TFT_BLACK);
  ui.loadFont(getScaledFont(), LittleFS);
  ui.drawString("Motorlast", 5, displayConfig.labels.lastY-10);
  ui.drawString("Temp.", displayConfig.labels.coolantX-5, displayConfig.labels.coolantY-10);
  ui.unloadFont();
  ui.pushSprite(0, 0, TFT_BLACK);
  
  // Trennlinien
  int lineY = displayConfig.labels.voltageY - 8;
  tft.drawLine(0, lineY, displayConfig.width, lineY, TFT_DARKGREY);
  
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Batt", displayConfig.labels.voltageX, displayConfig.labels.voltageY - 3);
  tft.drawString("Ansaug.T", displayConfig.labels.intakeX - 35, displayConfig.labels.intakeY - 3);
  tft.drawString("L/100", displayConfig.labels.intakeX + 27, displayConfig.labels.intakeY - 3);
  
  int divider1X = displayConfig.voltageText.x + displayConfig.voltageText.width;
  int divider2X = displayConfig.intakeText.x + displayConfig.intakeText.width + 5;
  int dividerTopY = lineY;
  int dividerBottomY = displayConfig.height;
  tft.drawLine(divider1X, dividerTopY, divider1X, dividerBottomY, TFT_DARKGREY);
  tft.drawLine(divider2X, dividerTopY, divider2X, dividerBottomY, TFT_DARKGREY);
  
  // Boost Meter
  tft.fillCircle(displayConfig.boostMeter.x, displayConfig.boostMeter.y, displayConfig.boostMeter.radius, GAUGE_GREY);
  tft.drawSmoothCircle(displayConfig.boostMeter.x, displayConfig.boostMeter.y, displayConfig.boostMeter.radius, TFT_SILVER, GAUGE_GREY);
  int boostThickness = displayConfig.boostMeter.radius / 5;
  tft.drawArc(displayConfig.boostMeter.x, displayConfig.boostMeter.y, 
              displayConfig.boostMeter.radius - 3, displayConfig.boostMeter.radius - 3 - boostThickness, 
              30, 330, TFT_BLACK, GAUGE_GREY);
  
  // Coolant Meter
  tft.fillCircle(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, displayConfig.coolantMeter.radius, GAUGE_GREY);
  tft.drawSmoothCircle(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, displayConfig.coolantMeter.radius, TFT_SILVER, GAUGE_GREY);
  int coolantThickness = displayConfig.coolantMeter.radius / 5;
  tft.drawArc(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, 
              displayConfig.coolantMeter.radius - 3, displayConfig.coolantMeter.radius - 3 - coolantThickness, 
              30, 330, TFT_BLACK, GAUGE_GREY);
  
  // Unit Labels
  float scale = min((float)displayConfig.width / baseConfig.width, (float)displayConfig.height / baseConfig.height);
  tft.loadFont(getScaledUnitFont(), LittleFS);
  int percentX = displayConfig.boostMeter.x - (int)(9 * scale);
  int percentY = displayConfig.boostMeter.y + displayConfig.boostMeter.radius - (int)(25 * scale);
  tft.setTextColor(TFT_WHITE, GAUGE_GREY);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("%", percentX - 3, percentY - 5);
  int dotX = displayConfig.coolantMeter.x - (int)(10 * scale);
  tft.drawString("°C", dotX + (int)(6 * scale) - 6, percentY - 5);
  tft.unloadFont();
  
  // Initial Gauge-Werte zeichnen (sonst bleiben sie leer)
  drawBoost(0);
  drawCoolant(0);
  
  dtcScreenActive = false;
}

// Temperatur-Farbverlauf Funktion
uint16_t getTemperatureColor(float temp) {
  // Definiere die Temperaturbereiche und Farben
  if (temp < 10) {
    return TFT_BLUE;  // unter 10°C: blau
  } 
  else if (temp >= 10 && temp <= 80) {
    // Durchgehender Übergang von Blau zu Grün (10-80°C)
    float factor = (temp - 10) / 70.0;  // 0.0 bis 1.0
    // Interpolation zwischen Blau (0x001F) und Grün (0x07E0)
    uint8_t r = 0;
    uint8_t g = (uint8_t)(factor * 63);  // Grün von 0 auf 63
    uint8_t b = (uint8_t)((1.0 - factor) * 31);  // Blau von 31 auf 0
    return (r << 11) | (g << 5) | b;
  }
  else if (temp > 80 && temp <= 110) {
    // Durchgehender Übergang von Grün zu Rot (80-110°C)
    float factor = (temp - 80) / 30.0;  // 0.0 bis 1.0
    // Interpolation zwischen Grün (0x07E0) und Rot (0xF800)
    uint8_t r = (uint8_t)(factor * 31);  // Rot von 0 auf 31
    uint8_t g = (uint8_t)((1.0 - factor) * 63);  // Grün von 63 auf 0
    uint8_t b = 0;
    return (r << 11) | (g << 5) | b;
  }
  else {
    return TFT_RED;  // über 110°C: rot
  }
}

void drawVolt(float reading)
{
  // Nur bei Änderung updaten (verhindert Flackern)
  if (abs(reading - prev_voltage) < 0.05) return;  // 50mV Toleranz
  prev_voltage = reading;
  
  // Bestimme Farbe basierend auf Spannung
  uint16_t textColor;
  if (reading < 12.4) {
    textColor = TFT_RED;     // unter 12,4V: rot
  } else if (reading >= 12.4 && reading < 13.3) {
    textColor = TFT_CYAN;    // 12,4 - 13,3V: hellblau
  } else {
    textColor = TFT_GREEN;   // ab 13,3V: grün
  }
  
  // Double-Buffer: Zeichne auf Sprite statt direkt auf Display (kein Flackern!)
  volt.fillSprite(TFT_BLACK);
  volt.loadFont(AA_FONT_SMALL, LittleFS);  // Große Schrift (NotoSansBold36)
  volt.setTextColor(textColor, TFT_BLACK);
  volt.setTextDatum(TL_DATUM);
  
  // Erstelle String mit angehängtem V
  String voltString = String(reading, 1) + "V";
  volt.drawString(voltString, 0, 5);
  volt.unloadFont();
  
  // Ein atomarer Push auf Display (kein Flackern!)
  volt.pushSprite(displayConfig.voltageText.x, displayConfig.voltageText.y);
}

void drawIntakeTemp(float reading)
{
  // Nur bei Änderung updaten (verhindert Flackern)
  if (abs(reading - prev_intakeTemp) < 0.5) return;  // 0.5°C Toleranz
  prev_intakeTemp = reading;
  
  // Bestimme Farbe basierend auf Temperatur
  uint16_t textColor;
  if (reading < 20) {
    textColor = TFT_CYAN;    // unter 20°C: hellblau
  } else if (reading >= 20 && reading <= 40) {
    textColor = TFT_GREEN;   // 20-40°C: grün
  } else {
    textColor = TFT_RED;     // über 40°C: rot
  }
  
  // Double-Buffer: Zeichne auf Sprite statt direkt auf Display (kein Flackern!)
  intaketxt.fillSprite(TFT_BLACK);
  intaketxt.loadFont(AA_FONT_SMALL, LittleFS);
  intaketxt.setTextColor(textColor, TFT_BLACK);
  intaketxt.setTextDatum(TL_DATUM);
  
  // Erstelle String mit angehängtem °C
  String tempString = String(reading, 0) + "°C";
  intaketxt.drawString(tempString, 5, 5);
  intaketxt.unloadFont();
  
  // Ein atomarer Push auf Display (kein Flackern!)
  intaketxt.pushSprite(displayConfig.intakeText.x, displayConfig.intakeText.y);
}

void drawAvgFuelEconomy(float reading)
{
  // Nur bei Änderung updaten (verhindert Flackern)
  if (abs(reading - prev_avgFuelEconomy) < 0.1) return;  // 0.1 L/100km Toleranz
  prev_avgFuelEconomy = reading;
  
  // Bestimme Farbe basierend auf Verbrauch
  uint16_t textColor;
  if (reading < 8.0) {
    textColor = TFT_GREEN;   // unter 8L: grün
  } else if (reading >= 8.0 && reading <= 10.0) {
    textColor = TFT_ORANGE;  // 8-10L: orange
  } else {
    textColor = TFT_RED;     // über 10L: rot
  }
  
  // Double-Buffer: Zeichne auf Sprite statt direkt auf Display (kein Flackern!)
  tripavgtxt.fillSprite(TFT_BLACK);
  tripavgtxt.loadFont(AA_FONT_SMALL, LittleFS);
  tripavgtxt.setTextColor(textColor, TFT_BLACK);
  tripavgtxt.setTextDatum(TL_DATUM);
  
  String avgString = String(reading, 1) + "L";
  tripavgtxt.drawString(avgString, 2, 5);
  tripavgtxt.unloadFont();
  
  // Ein atomarer Push auf Display (kein Flackern!)
  tripavgtxt.pushSprite(displayConfig.tripAvgText.x, displayConfig.tripAvgText.y);
}

// Motorlast-Farbverlauf Funktion
uint16_t getLoadColor(float load) {
  if (load < 10) {
    return TFT_CYAN;  // unter 10%: cyan/hellblau
  }
  else if (load >= 10 && load <= 60) {
    // Übergang von Grün zu Rot (10-60%)
    float factor = (load - 10) / 50.0;  // 0.0 bis 1.0
    // Interpolation zwischen Grün (0x07E0) und Rot (0xF800)
    uint8_t r = (uint8_t)(factor * 31);  // Rot von 0 auf 31
    uint8_t g = (uint8_t)((1.0 - factor) * 63);  // Grün von 63 auf 0
    uint8_t b = 0;
    return (r << 11) | (g << 5) | b;
  }
  else {
    return TFT_RED;  // über 60%: vollständig rot
  }
}

void ringMeterBoost(float val)
{
  // Nur bei signifikanter Änderung updaten (verhindert Zittern)
  if (abs(val - prev_load) < 0.5) return;  // 0.5% Toleranz
  prev_load = val;
  
  int x = displayConfig.boostMeter.x;
  int y = displayConfig.boostMeter.y;
  int r = displayConfig.boostMeter.radius - 3;

  // Range here is 0-100 so value is scaled to an angle 30-330
  float val_angle = map(val, 0, 100, 30, 330);

  if (abs(last_angle_boost - val_angle) > 1)  // Mindest-Winkeldifferenz
  {
    // Verwende Sprite für stufenlose Font-Skalierung mit fetter Schrift
    boosttxt.fillSprite(GAUGE_GREY);
    boosttxt.loadFont(AA_FONT_LARGE, LittleFS);  // NotoSansBold36 für fetteren Text
    boosttxt.setTextSize(getGaugeTextScale(displayConfig.boostMeter.radius));  // Stufenlose Skalierung!
    boosttxt.setTextDatum(CC_DATUM);
    boosttxt.setTextColor(TFT_WHITE, GAUGE_GREY);
    
    // Zentriere Text perfekt im Sprite
    boosttxt.drawFloat(val, 0, displayConfig.boostText.width/2, displayConfig.boostText.height/2);
    boosttxt.pushSprite(displayConfig.boostText.x, displayConfig.boostText.y);
    boosttxt.unloadFont();

    // Allocate a value to the arc thickness dependant of radius
    uint8_t thickness = r / 5;
    if (r < 25)
      thickness = r / 3;

    // Update the arc mit Farbverlauf basierend auf Last
    if (val_angle > last_angle_boost)
    {
      // Zeichne nur den neuen Bereich mit Farbverlauf
      int segment_size = 3;
      for (int angle = last_angle_boost; angle < val_angle; angle += segment_size) {
        int end_angle = min(angle + segment_size, (int)val_angle);
        float load_at_angle = map(angle, 30, 330, 0, 100);
        uint16_t segment_color = getLoadColor(load_at_angle);
        tft.drawArc(x, y, r, r - thickness, angle, end_angle, segment_color, TFT_BLACK, true);
      }
    }
    else
    {
      // Lösche nur den reduzierten Bereich
      tft.drawArc(x, y, r, r - thickness, val_angle, last_angle_boost, TFT_BLACK, GAUGE_GREY, true);
    }
    last_angle_boost = val_angle; // Store meter arc position for next redraw
  }
}

void ringMeterCoolant(float val)
{
  // Nur bei signifikanter Änderung updaten (verhindert Zittern)
  if (abs(val - prev_coolant) < 0.5) return;  // 0.5°C Toleranz
  prev_coolant = val;
  
  int x = displayConfig.coolantMeter.x;
  int y = displayConfig.coolantMeter.y;
  int r = displayConfig.coolantMeter.radius - 3;

  // Range here is 0-125 so value is scaled to an angle 30-330
  int val_angle = map(val, 0, 125, 30, 330);

  if (abs(last_angle_coolant - val_angle) > 1)  // Mindest-Winkeldifferenz
  {
    // Text-Update nur bei signifikanter Änderung (Sprite wird nur einmal erstellt)
    coolanttxt.fillSprite(GAUGE_GREY);
    coolanttxt.loadFont(AA_FONT_LARGE, LittleFS);
    coolanttxt.setTextSize(getGaugeTextScale(displayConfig.coolantMeter.radius));
    coolanttxt.setTextDatum(CC_DATUM);
    coolanttxt.setTextColor(TFT_WHITE, GAUGE_GREY);
    coolanttxt.drawFloat(val, 0, displayConfig.coolantText.width/2, displayConfig.coolantText.height/2);
    coolanttxt.pushSprite(displayConfig.coolantText.x, displayConfig.coolantText.y);
    coolanttxt.unloadFont();

    // Allocate a value to the arc thickness dependant of radius
    uint8_t thickness = r / 5;
    if (r < 25)
      thickness = r / 3;

    // Optimierte Arc-Updates: Nur geänderte Bereiche updaten (kein komplettes Löschen!)
    if (val_angle > last_angle_coolant)
    {
      // Zeichne nur den neuen Bereich
      int segment_size = 3;
      for (int angle = last_angle_coolant; angle < val_angle; angle += segment_size) {
        int end_angle = min(angle + segment_size, val_angle);
        float temp_at_angle = map(angle, 30, 330, 0, 125);
        uint16_t segment_color = getTemperatureColor(temp_at_angle);
        tft.drawArc(x, y, r, r - thickness, angle, end_angle, segment_color, TFT_BLACK, true);
      }
    }
    else if (val_angle < last_angle_coolant)
    {
      // Lösche nur den reduzierten Bereich
      tft.drawArc(x, y, r, r - thickness, val_angle, last_angle_coolant, TFT_BLACK, GAUGE_GREY, true);
    }
    
    last_angle_coolant = val_angle;
  }
}

void drawBoost(float load)
{
  ringMeterBoost(load); // Draw analogue meter with scaled values
}

void drawCoolant(float reading)
{
  ringMeterCoolant(reading); // Draw analogue meter with scaled values
}

void errorHandling(String message)
{
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  tft.begin(); // initialize a ST7789 chip
  tft.setRotation(1);
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness
  tft.fillScreen(TFT_BLACK);

  digitalWrite(12, HIGH);

  // Verwende skalierte Display-Größe
  errorspray.setColorDepth(8);
  errorspray.createSprite(displayConfig.width, displayConfig.height);
  errorspray.fillSprite(TFT_BLACK);
  errorspray.setSwapBytes(true);
  errorspray.setTextWrap(true, false);
  errorspray.setTextColor(TFT_WHITE, TFT_BLACK);
  errorspray.setTextSize(getScaledTextSize(2));
  
  // Zentriere Fehlermeldung
  int textX = displayConfig.width / 40;  // 5% Rand von links
  int textY = displayConfig.height / 2;  // Vertikal zentriert
  errorspray.drawString(message, textX, textY);  
  errorspray.pushSprite(0, 0, TFT_BLACK);

  delay(10000);
  ESP.restart();
}

// Funktion für Verbindungsfehler-Behandlung mit Countdown
void connectionErrorHandling(String message)
{
#if DEMO_MODE == 1
  // Demo-Modus aktiv: Keine Fehlermeldung, läuft im Demo-Modus weiter
  DEBUG_PORT.println("Demo Mode: Connection error ignored, running in demo mode");
  return;
#else
  // Demo-Modus deaktiviert: Fehlermeldung und Reboot nach 30 Sekunden
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  tft.begin();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  digitalWrite(12, HIGH);

  errorspray.setColorDepth(8);
  errorspray.createSprite(displayConfig.width, displayConfig.height);
  errorspray.setSwapBytes(true);
  errorspray.setTextWrap(true, true);
  errorspray.setTextDatum(MC_DATUM);  // Text mittig zentriert
  
  // Countdown von 30 Sekunden (runterzählend!)
  for (int countdown = 30; countdown > 0; countdown--) {
    // Komplett neu zeichnen für sauberen Wechsel
    errorspray.fillSprite(TFT_BLACK);
    
    // FEHLER! - ganz groß und fett oben mit Skalierung
    errorspray.loadFont(AA_FONT_LARGE, LittleFS);
    errorspray.setTextSize(2);  // Doppelte Größe!
    errorspray.setTextColor(TFT_RED, TFT_BLACK);
    int errorY = displayConfig.height / 6;
    errorspray.drawString("FEHLER!", displayConfig.width / 2, errorY);
    errorspray.setTextSize(1);  // Zurücksetzen
    errorspray.unloadFont();
    
    // "Neustart in" Text - größer mit Skalierung
    errorspray.loadFont(AA_FONT_LARGE, LittleFS);
    errorspray.setTextSize(2);
    errorspray.setTextColor(TFT_CYAN, TFT_BLACK);
    int labelY = displayConfig.height / 2;
    errorspray.drawString("Neustart in", displayConfig.width / 2, labelY);
    errorspray.unloadFont();
    
    // Countdown extra groß mit 3-facher Skalierung - zweistellig formatieren
    errorspray.loadFont(AA_FONT_LARGE, LittleFS);
    errorspray.setTextSize(3);  // 3-fache Größe für Countdown!
    errorspray.setTextColor(TFT_YELLOW, TFT_BLACK);
    String countdownText;
    if (countdown < 10) {
      countdownText = "0" + String(countdown);  // Führende Null für einstellige Zahlen
    } else {
      countdownText = String(countdown);
    }
    int countdownY = displayConfig.height / 2 + 40;
    errorspray.drawString(countdownText, displayConfig.width / 2, countdownY);
    errorspray.setTextSize(1);  // Zurücksetzen
    errorspray.unloadFont();
    
    // Sprite auf Display zeichnen
    errorspray.pushSprite(0, 0);
    
    delay(1000);
  }
  
  DEBUG_PORT.println("Rebooting due to connection error...");
  ESP.restart();
#endif
}

//////////////////////////////////////////////////////////////////////////
// PID Debug Funktion - listet alle verfügbaren PIDs auf
//////////////////////////////////////////////////////////////////////////

void debugAvailablePIDs() {
#if DEBUG_PIDS == 1
  DEBUG_PORT.println("\n========================================");
  DEBUG_PORT.println("VERFÜGBARE OBD-PIDs WERDEN GEPRÜFT...");
  DEBUG_PORT.println("========================================\n");
  
  // Liste der gängigen PIDs mit Beschreibungen
  struct PIDInfo {
    uint8_t pid;
    const char* name;
  };
  
  const PIDInfo commonPIDs[] = {
    {0x00, "PIDs 01-20 verfügbar"},
    {0x01, "Status seit DTC gelöscht"},
    {0x03, "Fuel System Status"},
    {0x04, "Motorlast (LOAD)"},
    {0x05, "Kühlmitteltemperatur"},
    {0x06, "Kurzzeit Fuel Trim Bank 1"},
    {0x07, "Langzeit Fuel Trim Bank 1"},
    {0x0A, "Kraftstoffdruck"},
    {0x0B, "Ansaugkrümmer Absolutdruck"},
    {0x0C, "Motordrehzahl (RPM)"},
    {0x0D, "Geschwindigkeit (KPH)"},
    {0x0E, "Zündwinkel"},
    {0x0F, "Ansauglufttemperatur (IAT)"},
    {0x10, "Luftmassenstrom (MAF)"},
    {0x11, "Drosselklappenposition"},
    {0x1F, "Motor Laufzeit"},
    {0x20, "PIDs 21-40 verfügbar"},
    {0x21, "Distanz mit MIL an"},
    {0x2F, "Tankfüllstand"},
    {0x33, "Absolutdruck Ansaugkrümmer"},
    {0x40, "PIDs 41-60 verfügbar"},
    {0x42, "Batteriespannung"},
    {0x43, "Absolute Motorlast"},
    {0x44, "Fuel/Air Verhältnis"},
    {0x45, "Relative Drosselklappenposition"},
    {0x46, "Umgebungstemperatur"},
    {0x49, "Fahrpedalposition"},
    {0x4C, "Drosselklappenposition Kommandiert"},
    {0x4D, "Motor Laufzeit mit MIL an"},
    {0x51, "Kraftstofftyp"},
    {0x5C, "Motoröltemperatur"},
    {0x5E, "Kraftstoffverbrauch L/h"},
    {0x60, "PIDs 61-80 verfügbar"},
  };
  
  const int numPIDs = sizeof(commonPIDs) / sizeof(commonPIDs[0]);
  
  DEBUG_PORT.println("Prüfe verfügbare PIDs (dies kann einige Sekunden dauern)...\n");
  
  int foundCount = 0;
  
  for (int i = 0; i < numPIDs; i++) {
    uint8_t pid = commonPIDs[i].pid;
    
    // Sende OBD-Anfrage für PID
    String query = "01" + String(pid, HEX);
    query.toUpperCase();
    if (pid < 0x10) {
      query = "010" + String(pid, HEX);
      query.toUpperCase();
    }
    
    ELM_PORT.print(query + "\r");
    delay(100);  // Kurze Verzögerung für Antwort
    
    String response = "";
    unsigned long timeout = millis() + 500;
    
    while (millis() < timeout) {
      if (ELM_PORT.available()) {
        char c = ELM_PORT.read();
        response += c;
        if (c == '>') break;
      }
    }
    
    // Prüfe ob valide Antwort (nicht "NO DATA" oder Error)
    if (response.indexOf("41") >= 0 && 
        response.indexOf("NO DATA") < 0 && 
        response.indexOf("ERROR") < 0 &&
        response.indexOf("?") < 0) {
      
      DEBUG_PORT.print("✓ PID 0x");
      if (pid < 0x10) DEBUG_PORT.print("0");
      DEBUG_PORT.print(pid, HEX);
      DEBUG_PORT.print(": ");
      DEBUG_PORT.println(commonPIDs[i].name);
      foundCount++;
    }
  }
  
  DEBUG_PORT.println("\n========================================");
  DEBUG_PORT.print("GEFUNDEN: ");
  DEBUG_PORT.print(foundCount);
  DEBUG_PORT.print(" von ");
  DEBUG_PORT.print(numPIDs);
  DEBUG_PORT.println(" geprüften PIDs");
  DEBUG_PORT.println("========================================\n");
#endif
}

//////////////////////////////////////////////////////////////////////////
// Build Constructor
//////////////////////////////////////////////////////////////////////////

ELM327 myELM327;
bool elm327_ready = false;

//////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////

void setup(void)
{
pinMode(12, OUTPUT);
digitalWrite(12, LOW);

#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  DEBUG_PORT.begin(115200);
  
  // Display sofort initialisieren und Logo anzeigen
  tft.begin();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  initDisplayConfig(tft.width(), tft.height());
  tft.fillScreen(TFT_BLACK);
  
  // Zeige dein echtes Opel-Logo (160x128 Pixel) intelligent skaliert
  float logo_scale = min((float)displayConfig.width / baseConfig.width, (float)displayConfig.height / baseConfig.height);
  
  int logo_width, logo_height;
  
  // Logo mit ursprünglichen Dimensionen (160 breit, 128 hoch)
  logo_width = 160;
  logo_height = 128;
  
  tft.pushImage(0, 0, logo_width, logo_height, opel_logo_brand_car_symbol_black_and_white_design_vector_46105953);
  digitalWrite(12, HIGH);
  
  // Kein delay - Logo bleibt während Verbindungsaufbau
  
  DEBUG_PORT.println("Starting Bluetooth connection...");
  
  // Initialisiere Bluetooth entsprechend ELMduino-Dokumentation
  ELM_PORT.begin("ArduHUD", true);
  
  DEBUG_PORT.println("Attempting to connect to ELM327...");
  
  if (!ELM_PORT.connect(address))
  {
    DEBUG_PORT.println(F("Couldn't connect to OBD scanner - Phase 1"));
    elm327_ready = false;
    connectionErrorHandling("Bluetooth Verbindung fehlgeschlagen!");
  }
  else 
  {
    DEBUG_PORT.println("Bluetooth connected successfully!");
    
    // ELM327 mit offiziellen Parametern initialisieren (wie in den Beispielen)
    DEBUG_PORT.println("Initializing ELM327...");
    
    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
      DEBUG_PORT.println(F("Couldn't connect to OBD scanner - Phase 2"));
      elm327_ready = false;
      connectionErrorHandling("ELM327 Initialisierung fehlgeschlagen!");
    }
    else 
    {
      DEBUG_PORT.println(F("Connected to ELM327"));
      elm327_ready = true;
      
      // Debug: Liste alle verfügbaren PIDs auf (nur wenn DEBUG_PIDS aktiviert)
      debugAvailablePIDs();
    }
  }

  if (!LittleFS.begin()) {
    Serial.println(F("Flash FS initialisation failed!"));
    message="Flash FS initialisation failed!";
    errorHandling(message);
  }
  Serial.println("\nFlash FS available!");

  bool font_missing = false;
  if (LittleFS.exists("/NotoSansBold15.vlw")    == false) font_missing = true;
  if (LittleFS.exists("/NotoSansBold36.vlw")    == false) font_missing = true;
  if (LittleFS.exists("/Latin-Hiragana-24.vlw")    == false) font_missing = true;

  if (font_missing)
  {
    Serial.println("\nFont missing in Flash FS, did you upload it?");
    message="Font missing in Flash FS!";
    errorHandling(message);
  }
  else Serial.println("\nFonts found OK.");

  //pinMode(4, INPUT);

  // Lösche Logo und baue UI auf - Verbindung ist jetzt bereit
  tft.fillScreen(TFT_BLACK);

  // UI - verwende skalierte Werte
  ui.setColorDepth(8);
  ui.createSprite(displayConfig.width, displayConfig.height);
  ui.fillSprite(TFT_BLACK);
  ui.setSwapBytes(true);
  ui.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Battery-Icon intelligent skalieren - nur in sauberen Stufen
  float scale = min((float)displayConfig.width / baseConfig.width, (float)displayConfig.height / baseConfig.height);
  int iconWidth, iconHeight;
  
  // Verwende saubere Skalierungsstufen um Pixelbrei zu vermeiden
  if (scale >= 1.0) {
    iconWidth = 72; iconHeight = 54;  // Original für große Displays
  } else if (scale >= 0.75) {
    iconWidth = 54; iconHeight = 40;  // 75% für mittlere Displays
  } else {
    iconWidth = 36; iconHeight = 27;  // 50% für kleine Displays
  }
  
  // ui.pushImage(displayConfig.batteryIcon.x, displayConfig.batteryIcon.y, 
  //              iconWidth, iconHeight, battery);
  
  // Voltage Reading - skaliert (wird nur einmal erstellt!)
  volt.setColorDepth(8);
  volt.createSprite(displayConfig.voltageText.width, displayConfig.voltageText.height);
  volt.fillSprite(TFT_BLACK);
  volt.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Intake Temp Reading - skaliert (wird nur einmal erstellt!)
  intaketxt.setColorDepth(8);
  intaketxt.createSprite(displayConfig.intakeText.width, displayConfig.intakeText.height);
  intaketxt.fillSprite(TFT_BLACK);
  intaketxt.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Trip Avg Reading - skaliert (wird nur einmal erstellt!)
  tripavgtxt.setColorDepth(8);
  tripavgtxt.createSprite(displayConfig.tripAvgText.width, displayConfig.tripAvgText.height);
  tripavgtxt.fillSprite(TFT_BLACK);
  tripavgtxt.setTextColor(TFT_CYAN, TFT_BLACK);
  
  // Labels - verwende skalierte Font
  ui.loadFont(getScaledFont(), LittleFS);
  ui.drawString("Motorlast", 5, displayConfig.labels.lastY-10);
  ui.drawString("Temp.", displayConfig.labels.coolantX-5, displayConfig.labels.coolantY-10);
  ui.unloadFont();
  
  ui.pushSprite(0, 0, TFT_BLACK);
  
  // Horizontale Trennlinie zwischen oberen und unteren Labels
  int lineY = displayConfig.labels.voltageY - 8;
  tft.drawLine(0, lineY, displayConfig.width, lineY, TFT_DARKGREY);
  
  // Labels für untere Zeile - direkt auf TFT mit Built-in Font für bessere Skalierung
  tft.setTextFont(1);  // Built-in Font 1 (6x8 pixel)
  tft.setTextSize(1);  // Kleine Größe
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Batt", displayConfig.labels.voltageX, displayConfig.labels.voltageY - 3);
  tft.drawString("Ansaug.T", displayConfig.labels.intakeX - 35, displayConfig.labels.intakeY - 3);
  tft.drawString("L/100", displayConfig.labels.intakeX + 27, displayConfig.labels.intakeY - 3);
  
  // Vertikale Trennlinien zwischen den 3 unteren Abschnitten
  int divider1X = displayConfig.voltageText.x + displayConfig.voltageText.width;
  int divider2X = displayConfig.intakeText.x + displayConfig.intakeText.width + 5;
  int dividerTopY = lineY;
  int dividerBottomY = displayConfig.height;
  tft.drawLine(divider1X, dividerTopY, divider1X, dividerBottomY, TFT_DARKGREY);
  tft.drawLine(divider2X, dividerTopY, divider2X, dividerBottomY, TFT_DARKGREY);
  
  // Boost Meter (linker Kreis) - hellerer Hintergrund
  tft.fillCircle(displayConfig.boostMeter.x, displayConfig.boostMeter.y, displayConfig.boostMeter.radius, GAUGE_GREY);
  tft.drawSmoothCircle(displayConfig.boostMeter.x, displayConfig.boostMeter.y, displayConfig.boostMeter.radius, TFT_SILVER, GAUGE_GREY);
  
  int boostThickness = displayConfig.boostMeter.radius / 5;
  tft.drawArc(displayConfig.boostMeter.x, displayConfig.boostMeter.y, 
              displayConfig.boostMeter.radius - 3, displayConfig.boostMeter.radius - 3 - boostThickness, 
              30, 330, TFT_BLACK, GAUGE_GREY);
  
  // Coolant Meter (rechter Kreis) - hellerer Hintergrund
  tft.fillCircle(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, displayConfig.coolantMeter.radius, GAUGE_GREY);
  tft.drawSmoothCircle(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, displayConfig.coolantMeter.radius, TFT_SILVER, GAUGE_GREY);
  
  int coolantThickness = displayConfig.coolantMeter.radius / 5;
  tft.drawArc(displayConfig.coolantMeter.x, displayConfig.coolantMeter.y, 
              displayConfig.coolantMeter.radius - 3, displayConfig.coolantMeter.radius - 3 - coolantThickness, 
              30, 330, TFT_BLACK, GAUGE_GREY);
  
  // "%" und "°C" Labels - NACH den Gauges direkt auf TFT zeichnen (verwende bereits deklarierte scale Variable)
  tft.loadFont(getScaledUnitFont(), LittleFS);
  
  // "%" Label für Boost - skalierte Position
  int percentX = displayConfig.boostMeter.x - (int)(9 * scale);
  int percentY = displayConfig.boostMeter.y + displayConfig.boostMeter.radius - (int)(25 * scale);
  tft.setTextColor(TFT_WHITE, GAUGE_GREY);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("%", percentX - 3, percentY - 5);
  
  // "°C" Label für Coolant - skalierte Position
  int dotX = displayConfig.coolantMeter.x - (int)(10 * scale);
  int dotY = displayConfig.coolantMeter.y + displayConfig.coolantMeter.radius - (int)(38 * scale);
  tft.drawString("°C", dotX + (int)(6 * scale) - 6, percentY - 5);
  
  tft.unloadFont();

    // Button Spray - skaliert
  int scaledButtonSize = getScaledTextSize(30);
  buttonspray.createSprite(scaledButtonSize, scaledButtonSize);

  // Boost Reading - skaliert mit hellerem Hintergrund (wird nur einmal erstellt!)
  boosttxt.setColorDepth(8);
  boosttxt.createSprite(displayConfig.boostText.width, displayConfig.boostText.height);
  boosttxt.fillSprite(GAUGE_GREY);
  boosttxt.setTextColor(TFT_WHITE, GAUGE_GREY);

  // Coolant Reading - skaliert (wird nur einmal erstellt - nicht mehr dynamisch!)
  coolanttxt.setColorDepth(8);
  coolanttxt.createSprite(displayConfig.coolantText.width, displayConfig.coolantText.height);
  coolanttxt.fillSprite(GAUGE_GREY);
  coolanttxt.setTextColor(TFT_WHITE, GAUGE_GREY);

  drawBoost(0.00);
  drawCoolant(00);
  
  // Initial Durchschnittsverbrauch anzeigen (0.0 L/100km)
  drawAvgFuelEconomy(0.0);

}

//////////////////////////////////////////////////////////////////////////
// Loop
//////////////////////////////////////////////////////////////////////////

void loop()
{
  // DTC-Screen Handling (falls aktiv)
  if (dtcScreenActive) {
    // Wenn Fehler vorhanden sind: Nur bei Motor-Start zurückwechseln!
    if (dtcCount > 0) {
      // Prüfe ob Motor wieder gestartet wurde
      if (elm327_ready && isEngineRunning()) {
        DEBUG_PORT.println("Engine started - returning to normal UI");
        restoreNormalUI();
      }
      // Im Demo-Modus: Nach 15 Sekunden zurück
      else if (!elm327_ready && millis() - dtcScreenStartTime >= DTC_SCREEN_DURATION) {
        restoreNormalUI();
      }
    }
    // Wenn keine Fehler: Nach 15 Sekunden zurück (wie bisher)
    else {
      if (millis() - dtcScreenStartTime >= DTC_SCREEN_DURATION) {
        restoreNormalUI();
      }
    }
    return;  // Normale Loop überspringen während DTC-Screen aktiv
  }
  
  // Motor-Status permanent überwachen (nur wenn ELM327 bereit)
  if (elm327_ready) {
    checkEngineStatus();
  }
  
  // Demo-Modus falls ELM327 nicht verfügbar (optimiert für flüssige Updates)
  if (!elm327_ready) {
    static unsigned long lastUpdate = 0;
    static unsigned long demoStartTime = millis();  // Start des Demo-Zyklus
    static bool demoMotorRunning = true;
    static float demoLoad = 50, demoCoolant = 80, demoVolt = 12.6, demoIntake = 25;  // Startiere mit realistischen Werten
    static float demoFuelEco = 7.5;  // Demo Fuel Economy L/100km
    static float demoAvgFuelEco = 7.5;
    
    // Demo-Modus: Simuliere Motor-Aus nach 30 Sekunden
    unsigned long demoRuntime = millis() - demoStartTime;
    
    if (demoMotorRunning && demoRuntime >= 30000) {
      // Nach 30 Sekunden: Motor-Aus-Simulation
      DEBUG_PORT.println("\n========================================");
      DEBUG_PORT.println("DEMO MODE: Simulating engine shutdown...");
      DEBUG_PORT.println("========================================\n");
      
      demoMotorRunning = false;
      demoVolt = 12.3;  // Spannung fällt (Lichtmaschine aus)
      demoLoad = 0;     // Motorlast 0%
      
      // Simuliere DTC-Fehler (Beispiele) - 5 Fehler für Demo
      dtcCount = 5;
      dtcCodes[0] = "P0420";  // Katalysator-Effizienz unter Schwellwert
      dtcCodes[1] = "P0171";  // System zu mager (Bank 1)
      dtcCodes[2] = "P0301";  // Zündaussetzer Zylinder 1
      dtcCodes[3] = "P0133";  // O2 Sensor träge Reaktion (Bank 1 Sensor 1)
      dtcCodes[4] = "P0128";  // Kühlmitteltemperatur unter Thermostat-Öffnungstemperatur
      
      DEBUG_PORT.println("Demo DTCs generated:");
      for (int i = 0; i < dtcCount; i++) {
        DEBUG_PORT.print("  ");
        DEBUG_PORT.println(dtcCodes[i]);
      }
      
      // Zeige DTC-Screen
      displayDTCScreen();
    }
    
    // Wenn Motor "aus" ist und DTC-Screen nicht mehr aktiv: Neustart des Demo-Zyklus
    if (!demoMotorRunning && !dtcScreenActive) {
      DEBUG_PORT.println("\nDEMO MODE: Restarting demo cycle...\n");
      demoStartTime = millis();  // Reset Timer
      demoMotorRunning = true;
      demoVolt = 13.5;  // Lichtmaschine wieder aktiv
      demoLoad = 50;    // Motorlast zurück
      dtcCount = 0;     // Fehler löschen für neuen Zyklus
    }
    
    if (millis() - lastUpdate > 1000) {  // Häufigere Updates für flüssigere Demo
      // Simuliere sanftere, realistischere Wertänderungen (kleinere Sprünge)
      demoLoad += random(-2, 3);  // Kleinere Sprünge
      if (demoLoad < 0) demoLoad = 0;
      if (demoLoad > 100) demoLoad = 100;
      
      demoCoolant += random(-1, 2) * 0.5;  // Noch kleinere Temperaturänderungen
      if (demoCoolant < 0) demoCoolant = 0;
      if (demoCoolant > 120) demoCoolant = 120;
      
      demoVolt += random(-5, 6) / 100.0;  // Kleinere Spannungsänderungen
      if (demoVolt < 11.5) demoVolt = 11.5;
      if (demoVolt > 14.5) demoVolt = 14.5;
      
      demoIntake += random(-1, 2) * 0.3;  // Ansaugtemperatur ändert sich langsam
      if (demoIntake < -10) demoIntake = -10;
      if (demoIntake > 60) demoIntake = 60;
      
      // Simuliere Fuel Economy
      demoFuelEco += random(-5, 6) / 10.0;
      if (demoFuelEco < 3.0) demoFuelEco = 3.0;
      if (demoFuelEco > 15.0) demoFuelEco = 15.0;
      
      // Gleitender Durchschnitt (Demo)
      demoAvgFuelEco = (demoAvgFuelEco * 0.95) + (demoFuelEco * 0.05);
      
      Serial.println("DEMO MODE - Simulated values:");
      Serial.printf("LOAD: %.1f %%\n", demoLoad);
      Serial.printf("Coolant: %.1f °C\n", demoCoolant);
      Serial.printf("Battery: %.2f V\n", demoVolt);
      Serial.printf("Intake Air: %.1f °C\n", demoIntake);
      Serial.printf("Fuel Economy: %.1f L/100km (Current) / %.1f L/100km (Avg)\n", demoFuelEco, demoAvgFuelEco);
      
      drawAvgFuelEconomy(demoAvgFuelEco);
      
      // Updates erfolgen nur bei signifikanten Änderungen (dank Delta-Check)
      drawBoost(demoLoad);
      drawCoolant(demoCoolant);
      drawVolt(demoVolt);
      drawIntakeTemp(demoIntake);
      
      lastUpdate = millis();
    }
    return;
  }

  // Normaler OBD-Modus
  switch (obd_state)
  {

  case LOAD:
  {
    load = myELM327.engineLoad();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      Serial.print("LOAD: ");
      Serial.print(load);
      Serial.println(" %");
      obd_state = COOLANT;
      drawBoost(load);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = COOLANT;
    }

    break;
  }

  case COOLANT:
  {
    float coolant = myELM327.engineCoolantTemp();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      Serial.print("Coolant: ");
      Serial.print(coolant);
      Serial.println(" °C");
      obd_state = VOLTAGE;
      drawCoolant(coolant);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = VOLTAGE;
    }

    break;
  }

  case VOLTAGE:
  {
    float battery = myELM327.batteryVoltage();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      voltage = battery;  // Globale Variable für Motor-Erkennung aktualisieren
      Serial.print("Battery: ");
      Serial.print(battery);
      Serial.println(" V");
      obd_state = INTAKE_TEMP;
      drawVolt(battery);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = INTAKE_TEMP;
    }

    break;
  }

  case INTAKE_TEMP:
  {
    float intakeTemp = myELM327.intakeAirTemp();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      Serial.print("Intake Air Temp: ");
      Serial.print(intakeTemp);
      Serial.println(" °C");
      obd_state = SPEED;
      drawIntakeTemp(intakeTemp);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = SPEED;
    }

    break;
  }

  case SPEED:
  {
    // Speed wird nicht angezeigt, aber wir lesen ihn trotzdem aus
    // um den OBD-Zyklus vollständig zu machen und für Verbrauchsberechnung
    float speed = myELM327.kph();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      vehicleSpeed = speed;  // Globale Variable für Verbrauchsberechnung
      Serial.print("Speed: ");
      Serial.print(speed);
      Serial.println(" km/h");
      obd_state = FUEL_ECONOMY;
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = FUEL_ECONOMY;
    }

    break;
  }

  case FUEL_ECONOMY:
  {
    // Fuel Rate in L/h auslesen (PID 0x5F)
    float fuelRateLperH = myELM327.fuelRate();
    
    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      Serial.print("Fuel Rate: ");
      Serial.print(fuelRateLperH);
      Serial.println(" L/h");
      
      // Berechne L/100km aus Fuel Rate (L/h) und Speed (km/h)
      // Formel: L/100km = (Fuel Rate / Speed) * 100
      if (vehicleSpeed > 5.0) {  // Nur bei Fahrt berechnen (> 5 km/h)
        currentFuelEconomy = (fuelRateLperH / vehicleSpeed) * 100.0;
        
        Serial.print("Fuel Economy (berechnet): ");
        Serial.print(currentFuelEconomy);
        Serial.println(" L/100km");
        
        // Gleitender Durchschnitt berechnen
        // Nur plausible Werte einbeziehen
        if (currentFuelEconomy > 0 && currentFuelEconomy < 50) {  // Plausibilitätsprüfung
          fuelEconomySamples++;
          fuelEconomySum += currentFuelEconomy;
          
          // Gewichteter gleitender Durchschnitt (95% alt, 5% neu)
          if (avgFuelEconomy == 0) {
            avgFuelEconomy = currentFuelEconomy;  // Initialisierung
          } else {
            avgFuelEconomy = (avgFuelEconomy * 0.95) + (currentFuelEconomy * 0.05);
          }
          
          Serial.print("Fuel Economy (Durchschnitt): ");
          Serial.print(avgFuelEconomy);
          Serial.print(" L/100km (Samples: ");
          Serial.print(fuelEconomySamples);
          Serial.println(")");
          
          drawAvgFuelEconomy(avgFuelEconomy);
        }
      } else {
        // Bei Stillstand/langsamer Fahrt: Behalte letzten Wert
        Serial.println("Speed too low for fuel economy calculation");
      }
      
      obd_state = RPM;
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = RPM;
    }
    break;
  }

  case RPM:
  {
    float rpm = myELM327.rpm();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      engineRPM = rpm;  // Globale Variable für Motor-Erkennung aktualisieren
      Serial.print("RPM: ");
      Serial.print(rpm);
      Serial.println(" U/min");
      obd_state = LOAD;
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
      myELM327.printError();
      obd_state = LOAD;
    }

    break;
  }
  }
}
