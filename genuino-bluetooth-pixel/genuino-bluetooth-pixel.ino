#include <GenuinoBluetoothWrapper.h>
#include <LoggingWrapper.h>
#include <GammaCorrection.h>
#include <Adafruit_NeoPixel.h>
#include <BluetoothData.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LOGGING false

#define PIN 6
#define NUM_LEDS 24
#define BRIGHTNESS 32

// Forward-define methods
void setLEDColor(uint32_t hex, uint8_t wait);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
GenuinoBluetooth ble;
Logger logger(LOGGING);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  // Init. and start BLE library.
  ble.initialize("Cav Pager");
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'

  logger.initialize();
}

void loop() {
    // Check for incoming characters from Bluefruit
    char *line = ble.read_line();
    // Return if insufficient data
    if (strlen(line) < 7) {
        ble.proceed();
        return;
    }

    logger.log("Line: %s", line);
    BluetoothData data(line);
    logger.log("Command: %d\nMessage: %s", data.command, data.message);

    switch (data.command) {
        case NULL_COMMAND:
          logger.log("Null command: %d", data.command);
          setLEDColor(0, 35);
          break;
        case COLOR_COMMAND:
          uint32_t hex_color = strtol(data.message, (char **) NULL, 16);
          uint32_t gamma_corrected = gamma_correct_hex(hex_color);
          logger.log("Got color: %lx\nGamma corrected: %lx", hex_color, gamma_corrected);
          setLEDColor(gamma_corrected, 35);
          break;
        default:
          logger.log("Unrecognized command: %d", data.command);
          break;
    }

    ble.proceed();
}

// Fill the dots one after the other with a color
void setLEDColor(uint32_t hex, uint8_t wait) {
    for (uint8_t k = 1; k <= strip.numPixels(); ++k) {
        strip.setPixelColor(NUM_LEDS - k, hex);
        strip.show();
        delay(wait);
    }
}
