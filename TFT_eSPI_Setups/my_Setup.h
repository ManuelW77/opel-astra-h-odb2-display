// Only define one driver, the other ones must be commented out
//#define ILI9341_DRIVER
// manu #define ST7735_DRIVER      // Define additional parameters below for this display
//#define ILI9163_DRIVER     // Define additional parameters below for this display
//#define S6D02A1_DRIVER
//#define RPI_ILI9486_DRIVER // 20MHz maximum SPI
//#define HX8357D_DRIVER
//#define ILI9481_DRIVER
//#define ILI9486_DRIVER
//#define ILI9488_DRIVER     // WARNING: Do not connect ILI9488 display SDO to MISO if other devices share the SPI bus (TFT SDO does NOT tristate when CS is high)
#define ST7789_DRIVER      // Define additional parameters below for this display
// For ST7789 ONLY, define the colour order IF the blue and red are swapped on your display
// Try ONE option at a time to find the correct colour order for your display

//  #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
#define TFT_RGB_ORDER TFT_RGB  

// For ST7735 ONLY, define the type of display, originally this was based on the
// colour of the tab on the screen protector film but this is not always true, so try
// out the different options below if the screen does not display graphics correctly,
// e.g. colours wrong, mirror images, or tray pixels at the edges.
// Comment out ALL BUT ONE of these options for a ST7735 display driver, save this
// this User_Setup file, then rebuild and upload the sketch to the board again:

//#define ST7735_INITB
//#define ST7735_GREENTAB
//#define ST7735_GREENTAB2
////#define ST7735_GREENTAB3
// #define ST7735_GREENTAB128    // For 128 x 128 display
// #define ST7735_GREENTAB160x80 // For 160 x 80 display (BGR, inverted, 26 offset)
// manu #define ST7735_REDTAB
//#define ST7735_BLACKTAB
// #define ST7735_REDTAB160x80   // For 160 x 80 display with 24 pixel offset

// For ST7789, ST7735 and ILI9163 ONLY, define the pixel width and height in portrait orientation
// #define TFT_WIDTH  80
// manu #define TFT_WIDTH  128
#define TFT_WIDTH  320 // ST7789 240 x 240 and 240 x 320
// #define TFT_HEIGHT 160
// manu #define TFT_HEIGHT 160
#define TFT_HEIGHT 240 // ST7789 240 x 240
//#define TFT_HEIGHT 280 // ST7789 240 x 240
// #define TFT_HEIGHT 320 // ST7789 240 x 320

// If colours are inverted (white shows as black) then uncomment one of the next
// 2 lines try both options, one of the options should correct the inversion.

//#define TFT_INVERSION_ON
#define TFT_INVERSION_OFF

// If a backlight control signal is available then define the TFT_BL pin in Section 2
// below. The backlight will be turned ON when tft.begin() is called, but the library
// needs to know if the LEDs are ON with the pin HIGH or LOW. If the LEDs are to be
// driven with a PWM signal or turned OFF/ON then this must be handled by the user
// sketch. e.g. with digitalWrite(TFT_BL, LOW);

//#define TFT_BACKLIGHT_ON HIGH  // HIGH or LOW are options

// For ESP32 Dev board (only tested with ILI9341 display)
// The hardware SPI can be mapped to any pins

 #define TFT_MISO 12 // Frei
 #define TFT_MOSI 13
 #define TFT_SCLK 14
 //////#define TFT_CS   17  // Chip select control pin
 #define TFT_CS   15
 #define TFT_DC   2  // Data Command control pin
 #define TFT_RST   -1  // Reset pin (could connect to RST pin)
 //#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

//#define TFT_BL   12  // LED back-light (only for ST7789 with backlight control pin)

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000 // Actually sets it to 26.67MHz = 80/3

// Optional reduced SPI frequency for reading TFT
#define SPI_READ_FREQUENCY  20000000

// The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here:
#define SPI_TOUCH_FREQUENCY  2500000

// The ESP32 has 2 free SPI ports i.e. VSPI and HSPI, the VSPI is the default.
// If the VSPI port is in use and pins are not accessible (e.g. TTGO T-Beam)
// then uncomment the following line:
//#define USE_HSPI_PORT

// Transaction support is needed to work with SD library but not needed with TFT_SdFat
// Transaction support is required if other SPI devices are connected.

// Transactions are automatically enabled by the library for an ESP32 (to use HAL mutex)
// so changing it here has no effect

#define SUPPORT_TRANSACTIONS
