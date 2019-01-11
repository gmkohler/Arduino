#include <GenuinoBluetoothWrapper.h>
#include <LoggingWrapper.h>
#include <GammaCorrection.h>
#include <Adafruit_NeoPixel.h>
#include <BluetoothData.h>
#include <LEDAnimator.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LOGGING false

#define PIN 6
#define NUM_LEDS 24
#define BRIGHTNESS 32

#define NUM_ANIMATIONS 1

AnimData animations[NUM_ANIMATIONS] = {
  AnimData(ANIMATION_LOOP, 0)
};

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
LEDAnimator animator(&strip, &logger, animations, NUM_ANIMATIONS);

BluetoothData data;
bool run_animation = false;

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
