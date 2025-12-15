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
  FUEL_ECONOMY
} obd_pid_states;
obd_pid_states obd_state = VOLTAGE;

float coolant = 0;
float voltage = 0;
float load = 0;
float intakeTemp = 0;

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
// Functions
//////////////////////////////////////////////////////////////////////////

void(* resetFunc) (void) = 0;

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

}

//////////////////////////////////////////////////////////////////////////
// Loop
//////////////////////////////////////////////////////////////////////////

void loop()
{
  // Demo-Modus falls ELM327 nicht verfügbar (optimiert für flüssige Updates)
  if (!elm327_ready) {
    static unsigned long lastUpdate = 0;
    static float demoLoad = 50, demoCoolant = 80, demoVolt = 12.6, demoIntake = 25;  // Startiere mit realistischen Werten
    static float demoFuelEco = 7.5;  // Demo Fuel Economy L/100km
    static float demoAvgFuelEco = 7.5;
    
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

  case FUEL_ECONOMY:
  {
    // Fuel Economy (inverse) = L/100km Momentanverbrauch
    // PID 0x5E - unterstützt von deinem Fahrzeug
    currentFuelEconomy = myELM327.fuelRate();  // Temporär - wird durch custom command ersetzt
    
    // Custom Command für PID 0x5E (Fuel Economy inverse)
    // Format: "01 5E" returns 2 bytes: A*B/20 = L/100km
    // Dies muss ggf. manuell implementiert werden, falls ELMduino das nicht unterstützt
    
    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
      Serial.print("Fuel Economy (Momentan): ");
      Serial.print(currentFuelEconomy);
      Serial.println(" L/100km");
      
      // Gleitender Durchschnitt berechnen
      // Nur Werte > 0 einbeziehen (Fahrzeug fährt)
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
