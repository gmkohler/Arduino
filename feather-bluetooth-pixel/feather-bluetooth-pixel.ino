#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <FeatherBluetoothWrapper.h>
#include <LoggingWrapper.h>
#include <GammaCorrection.h>
#include <BluetoothData.h>

// LED Ring Attributes
#define PIN 6
#define NUM_LEDS 16
#define BRIGHTNESS 32

#define LOGGING false

// Initialize the LED and BLE
Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
FeatherBluetooth ble;
Logger logger(LOGGING);

// Forward-define methods
void setLEDColor(uint32_t hex, uint8_t wait);

void setup() {
    logger.initialize();
    ble.initialize("Cav Pager B");

    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.show(); // Initialize all pixels to 'off'
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

void setLEDColor(uint32_t hex, uint8_t wait) {
    for (uint8_t k = 1; k <= strip.numPixels(); ++k) {
        strip.setPixelColor(NUM_LEDS - k, hex);
        strip.show();
        delay(wait);
    }
}
