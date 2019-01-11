#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <FeatherBluetoothWrapper.h>
#include <LoggingWrapper.h>
#include <GammaCorrection.h>
#include <BluetoothData.h>
#include <LEDAnimator.h>

// LED Ring Attributes
#define PIN 6
#define NUM_LEDS 16
#define BRIGHTNESS 32

#define LOGGING false
#define NUM_ANIMATIONS 1

AnimData animation[NUM_ANIMATIONS] = {
  AnimData(ANIMATION_LOOP, 0)
};

// Initialize the LED and BLE
Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
FeatherBluetooth ble;
Logger logger(LOGGING);
LEDAnimator animator(&strip, &logger, animation, NUM_ANIMATIONS);

BluetoothData data;
bool run_animation = false;

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
    if (strlen(line) >= 7) {
      run_animation = false;
      animator.reset();
      logger.log("Line: %s", line);
      data.read(line);
      logger.log("Command: %d\nMessage: %s", data.command, data.message);
    } else if (!run_animation) {
      ble.proceed();
      return;
    }

    switch (data.command) {
        case NULL_COMMAND:
          logger.log("Null command: %d", data.command);
          animator.animate_led_color(0, 35);
          break;
        case COLOR_COMMAND:
          logger.log("Got color: %lx\nGamma corrected: %lx", data.hex_color(), data.gamma_corrected_hex());
          animator.animate_led_color(data.gamma_corrected_hex(), 35);
          break;
        case ANIMATE_COMMAND:
          logger.log("Animating");
          run_animation = true;
          animator.set_animation_color(data.gamma_corrected_hex());
          animator.step();
          break;
        default:
          logger.log("Unrecognized command: %d", data.command);
          break;
    }

    ble.proceed();
}
